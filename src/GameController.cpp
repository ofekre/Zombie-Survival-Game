#include "Gamecontroller.h"
#include "Constants.h"
#include <algorithm>

using namespace GameConstants;

void GameController::run()
{
    // Create initial window for menu
    m_window.create(sf::VideoMode({MENU_WINDOW_WIDTH, MENU_WINDOW_HEIGHT}), "Lode Runner");
    m_window.setFramerateLimit(FRAME_RATE);
    
    // Initialize sound effects
    m_gameOverSound.emplace(ResourceManager::instance().getSound("GameOver"));
    m_winSound.emplace(ResourceManager::instance().getSound(Sounds::WIN));
    m_countdownSound.emplace(ResourceManager::instance().getSound("countdown"));

    // Initialize and start background music
    if (m_backgroundMusic.openFromFile("resources/sounds/music.ogg"))
    {
        m_backgroundMusic.setLooping(true);
        m_backgroundMusic.setVolume(80.f);  // Lower volume 
        m_backgroundMusic.play();
    }
    
	m_state = GameState::Menu;// Start in menu state
    
    while (m_window.isOpen())
    {
        handleEvents();
        update();
        render();
    }
}

void GameController::handleEvents()
{
    while (const auto event = m_window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            m_window.close();
        }
        
        // Handle menu input when in menu state
        if (m_state == GameState::Menu)
        {
            m_menu.handleEvent(*event, m_window);
            
            // Check if user made a selection
            if (m_menu.isOptionChosen())
            {
                MenuOption selected = m_menu.getSelectedOption();
                if (selected == MenuOption::NewGame)
                {
                    startNewGame();  // Delegate to game logic
                }
                else if (selected == MenuOption::Help)
                {
                    m_state = GameState::Help;
                    m_menu.reset();
                    continue; // Skip checking Enter again in this same event loop iteration
                }
                else if (selected == MenuOption::Exit)
                {
                    m_window.close();
                }
                m_menu.reset();
            }
        }
        
        // ESC returns to menu from gameplay/help, closes only from menu
        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                if (m_state == GameState::Help)
                {
                    m_state = GameState::Menu;
                    m_menu.reset();
                }
                else if (m_state == GameState::Playing || m_state == GameState::Victory ||
                         m_state == GameState::GameOver || m_state == GameState::TimeUp)
                {
                    // Return to menu from gameplay
                    m_board.stopPlayerFallingSound();  // Stop any playing sounds
                    if (m_countdownSound)
                        m_countdownSound->stop();
                    resetToMenu();
                }
                else if (m_state == GameState::Menu)
                {
                    m_window.close();  // Exit only from menu
                }
            }
            // Enter also returns from Help screen
            if (keyPressed->code == sf::Keyboard::Key::Enter && m_state == GameState::Help)
            {
                m_state = GameState::Menu;
                m_menu.reset();
            }
        }
        
        // Handle mouse click on mute icon (in all states)
        if (const auto* mouseClick = event->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mouseClick->button == sf::Mouse::Button::Left)
            {
                sf::Vector2f clickPos(static_cast<float>(mouseClick->position.x), 
                                      static_cast<float>(mouseClick->position.y));
                if (m_menu.getMuteIconBounds(m_window).contains(clickPos))
                {
                    m_menu.toggleMute();  // Use Menu's mute function
                }
            }
        }
    }
}

void GameController::handleMuteInput()
{
    // Handle mute toggle with M key
    bool mPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::M);
    if (mPressed && !m_mKeyWasPressed)
    {
        m_menu.toggleMute();
    }
    m_mKeyWasPressed = mPressed;

    // Update sound volumes if mute state changed
    bool currentMuteState = m_menu.isMuted();
    if (currentMuteState != m_lastMuteState)
    {
        updateSoundVolumes();
        m_lastMuteState = currentMuteState;
    }
}

void GameController::updateSoundVolumes()
{
    bool muted = m_menu.isMuted();
    if (muted)
    {
        m_backgroundMusic.setVolume(0.f);
        if (m_gameOverSound) m_gameOverSound->setVolume(0.f);
        if (m_winSound) m_winSound->setVolume(0.f);
        if (m_countdownSound) m_countdownSound->setVolume(0.f);
    }
    else
    {
        m_backgroundMusic.setVolume(80.f);
        if (m_gameOverSound) m_gameOverSound->setVolume(100.f);
        if (m_winSound) m_winSound->setVolume(100.f);
        if (m_countdownSound) m_countdownSound->setVolume(100.f);
    }
}

void GameController::update()
{
    // Handle mute toggle with M key (in all states)
    handleMuteInput();
    
    // Update Board (and Player) mute state
    m_board.setMuted(m_menu.isMuted());
    
    // Handle end screen timeout (Victory and GameOver return to menu after delay)
    if (m_state == GameState::GameOver || m_state == GameState::Victory)
    {
        if (m_endScreenTimer.getElapsedTime().asSeconds() > 4.0f)
        {
            resetToMenu();
        }
        return;
    }
    
    // Handle TimeUp separately - reload level instead of menu
    if (m_state == GameState::TimeUp)
    {
        if (m_endScreenTimer.getElapsedTime().asSeconds() > 3.0f)
        {
            m_board.reloadCurrentLevel();
            m_gameTimer.restart();
            m_clock.restart();
            m_skipNextFrame = true;  // Prevent falling on reload
            m_state = GameState::Playing;
        }
        return;
    }
    
    if (m_state != GameState::Playing)
    {
        return;
    }
    
    // Check if time is up
    float elapsedTime = m_gameTimer.getElapsedTime().asSeconds();
    float timeRemaining = m_board.getLevelTimeLimit() - elapsedTime;
    
    // Play countdown sound in last 4 seconds (audio is 3 sec, start 1 sec early for sync)
    if (timeRemaining <= 4.f && timeRemaining > 0.f && !m_countdownPlaying)
    {
        if (m_countdownSound)
        {
            m_countdownSound->play();
        }
        m_countdownPlaying = true;
    }
    
    // Trigger TIME'S UP exactly at the end of the 3rd second of countdown (1.0 sec remaining)
    if (elapsedTime >= m_board.getLevelTimeLimit() - 1.0f)
    {
        m_board.losePlayerLife();  // Lose a life when time runs out
        m_board.stopPlayerFallingSound();  // Stop falling sound on time up
        m_state = GameState::TimeUp;
        m_endMessage = "TIME'S UP!";
        m_endMessageColor = GameConstants::Colors::TIME_UP;
        m_endScreenTimer.restart();
        m_countdownPlaying = false;  // Reset for next level
        return;
    }
    
    float deltaTime = m_clock.restart().asSeconds();

    // Skip the first frame after level load to prevent player from falling
    if (m_skipNextFrame)
    {
        m_skipNextFrame = false;
        deltaTime = 0.f;  // No movement on first frame
    }

    if (deltaTime > 0.1f)
    {
        deltaTime = 0.1f;
    }

	m_board.update(deltaTime);// Update board and all objects
    
    // Check for game over (no lives left)
    if (m_board.getPlayerLives() <= 0)
    {
        m_board.stopPlayerFallingSound();  // Stop falling sound on game over
        m_state = GameState::GameOver;
        m_endMessage = "GAME OVER";
        m_endMessageColor = GameConstants::Colors::GAME_OVER;
        m_backgroundMusic.stop();  // Stop background music
        if (m_gameOverSound)
            m_gameOverSound->play();
        m_endScreenTimer.restart();
        return;
    }
    
    // Check if level is complete (all coins collected)
    if (m_board.isLevelComplete())
    {
        // Add level completion bonus (50 × level number)
        m_board.addLevelCompleteScore();
        
        if (m_board.hasMoreLevels())
        {
            // Go to next level
            m_board.nextLevel();
            m_gameTimer.restart();
            m_clock.restart();
            m_skipNextFrame = true;  // Prevent falling on level transition
            m_countdownPlaying = false;  // Reset countdown for new level
            if (m_countdownSound)
                m_countdownSound->stop();  // Stop countdown sound if it was playing

            // Resize window to fit new level
            unsigned int newWidth = static_cast<unsigned int>(m_board.getWidth());
            unsigned int newHeight = static_cast<unsigned int>(m_board.getHeight() + GameConstants::HUD_HEIGHT);
            m_window.setSize({newWidth, newHeight});
            m_window.setView(sf::View(sf::FloatRect({0, 0}, {static_cast<float>(newWidth), static_cast<float>(newHeight)})));
        }
        else
        {
            // All levels complete - Victory!
            m_board.stopPlayerFallingSound();  // Stop falling sound on victory
            m_state = GameState::Victory;
            m_endMessage = "VICTORY!";
            m_endMessageColor = GameConstants::Colors::VICTORY;
            m_backgroundMusic.stop();  // Stop background music
            if (m_countdownSound) 
                m_countdownSound->stop(); // Ensure countdown stops if it was playing
            if (m_winSound) 
                m_winSound->play();
            m_endScreenTimer.restart();
        }
    }
    
    // Update HUD with remaining time (countdown)
    elapsedTime = m_gameTimer.getElapsedTime().asSeconds();
    float remainingTime = m_board.getLevelTimeLimit() - elapsedTime;
    if (remainingTime < 0)
        remainingTime = 0;
    
    m_hud.update(
        m_board.getLevelNumber(),
        m_board.getPlayerScore(),
        m_board.getPlayerLives(),
        remainingTime
    );
}

void GameController::render()
{
    m_window.clear(sf::Color::Black);
    
    if (m_state == GameState::Playing || m_state == GameState::Victory || m_state == GameState::GameOver || m_state == GameState::TimeUp)
    {
        m_board.draw(m_window);
        m_hud.draw(m_window);
        
        // Draw end screen overlay
        if (m_state == GameState::Victory || m_state == GameState::GameOver || m_state == GameState::TimeUp)
        {
            showEndScreen(m_endMessage, m_endMessageColor);
        }
    }
    else if (m_state == GameState::Help)
    {
        m_menu.drawHelpScreen(m_window);  // Menu handles help screen rendering
    }
    else
    {
        m_menu.draw(m_window);
    }
    
    // Draw mute icon in ALL states (using Menu's function)
    m_menu.drawMuteIcon(m_window);
    
    m_window.display();
}

void GameController::startNewGame()
{
    // Transition from menu to playing
    m_state = GameState::Playing;
    loadNextLevel();
    m_clock.restart();
    m_gameTimer.restart();
    
    // Restart background music if it was stopped
    if (m_backgroundMusic.getStatus() != sf::SoundSource::Status::Playing)
    {
        m_backgroundMusic.play();
    }
}

void GameController::loadNextLevel()
{
    // Reset clocks BEFORE loading to prevent accumulated time from affecting physics
    m_clock.restart();
    m_gameTimer.restart();

    m_board.loadFromFile("resources/Board.txt");

    // Resize window to fit board + HUD space
    unsigned int width = static_cast<unsigned int>(m_board.getWidth());
    unsigned int height = static_cast<unsigned int>(m_board.getHeight() + HUD_HEIGHT);

    m_window.setSize({width, height});

    // Also update the view to match
    sf::View view(sf::FloatRect({0.f, 0.f}, {static_cast<float>(width), static_cast<float>(height)}));
    m_window.setView(view);
    
    // Center window on screen
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    m_window.setPosition({
        static_cast<int>((desktop.size.x - width) / 2),
        static_cast<int>((desktop.size.y - height) / 2)
    });

    // Reset clock again after loading to ensure first frame has minimal deltaTime
    m_clock.restart();
    m_gameTimer.restart();

    // Skip the first frame's deltaTime to prevent player from falling
    m_skipNextFrame = true;
}

void GameController::showEndScreen(const std::string& message, sf::Color color)
{
    // TIME'S UP uses overlay, VICTORY and GAME OVER use full backgrounds
    if (message == "TIME'S UP!")
    {
        // Draw semi-transparent overlay
        sf::RectangleShape overlay({m_board.getWidth(), m_board.getHeight() + HUD_HEIGHT});
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        m_window.draw(overlay);
    }
    else
    {
        // Draw appropriate background based on outcome
        const char* bgTexture = (message == "VICTORY!") ? Textures::VICTORY_BACKGROUND : Textures::GAME_OVER_BACKGROUND;
        sf::Texture& endBgTexture = ResourceManager::instance().getTexture(bgTexture);
        sf::Sprite background(endBgTexture);
        
        // Scale to fit within window while maintaining aspect ratio
        sf::Vector2u windowSize = m_window.getSize();
        sf::Vector2u texSize = endBgTexture.getSize();
        
        // Calculate scale to fit within window (use smaller scale to ensure it fits)
        float scaleX = static_cast<float>(windowSize.x) / texSize.x;
        float scaleY = static_cast<float>(windowSize.y) / texSize.y;
        float scale = std::min(scaleX, scaleY);  // Use smaller scale to fit entirely
        
        background.setScale({scale, scale});
        
        // Center the scaled background (black margins will appear on sides if aspect ratios differ)
        float scaledWidth = texSize.x * scale;
        float scaledHeight = texSize.y * scale;
        float offsetX = (static_cast<float>(windowSize.x) - scaledWidth) / 2.0f;
        float offsetY = (static_cast<float>(windowSize.y) - scaledHeight) / 2.0f;
        background.setPosition({offsetX, offsetY});
        
        m_window.draw(background);
    }
    
    // Draw message
    sf::Text text(ResourceManager::instance().getFont(Fonts::MENU), message, 72);
    text.setFillColor(color);
    
    // Center the text
    sf::FloatRect textBounds = text.getGlobalBounds();
    float x = (m_board.getWidth() - textBounds.size.x) / 2.0f;
    float y = (m_board.getHeight() + HUD_HEIGHT - textBounds.size.y) / 2.0f;
    text.setPosition({x, y});
    
    m_window.draw(text);
}

void GameController::resetToMenu()
{
    m_state = GameState::Menu;

    // Stop any playing end-game sounds
    if (m_winSound)
        m_winSound->stop();
    if (m_gameOverSound)
        m_gameOverSound->stop();

    // Reload all levels from file for fresh start
    m_board.loadFromFile("resources/Board.txt");

    // Reset window size for menu
    m_window.setSize({MENU_WINDOW_WIDTH, MENU_WINDOW_HEIGHT});
    m_window.setView(sf::View(sf::FloatRect({0, 0}, {static_cast<float>(MENU_WINDOW_WIDTH), static_cast<float>(MENU_WINDOW_HEIGHT)})));

    // Restart background music if it was stopped
    if (m_backgroundMusic.getStatus() != sf::SoundSource::Status::Playing)
    {
        m_backgroundMusic.play();
    }

    // Prepare for next game start
    m_clock.restart();
    m_skipNextFrame = true;
}
