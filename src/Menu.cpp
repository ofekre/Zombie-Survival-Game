#include "Menu.h"
#include "Resourcemanager.h"
#include "Constants.h"

Menu::Menu()
{
    m_selectedIndex = 0;
    m_optionChosen = false;
    m_showHelp = false;
    m_isMuted = false;
}

void Menu::draw(sf::RenderWindow& window)
{
    sf::Font& font = ResourceManager::instance().getFont(GameConstants::Fonts::MENU);
    
    // Get window size for centering
    sf::Vector2u windowSize = window.getSize();
    float centerX = windowSize.x / 2.f;
    
    // Draw background image
    sf::Texture& bgTexture = ResourceManager::instance().getTexture(GameConstants::Textures::BACKGROUND);
    sf::Sprite background(bgTexture);
    // Scale to fill window
    sf::Vector2u texSize = bgTexture.getSize();
    background.setScale({
        static_cast<float>(windowSize.x) / texSize.x,
        static_cast<float>(windowSize.y) / texSize.y
    });
    window.draw(background);
    
    // Draw title - Blood red for zombie theme
    sf::Text title(font, "LODE RUNNER", 60);
    title.setFillColor(GameConstants::Colors::MENU_TITLE);
    title.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = title.getGlobalBounds();
    title.setPosition({centerX - titleBounds.size.x / 2.f, 50.f});
    window.draw(title);
    
    // Draw menu options with frames
    float startY = 160.f;      // Moved down a bit
    float spacing = 80.f;
    float frameWidth = 220.f;
    float frameHeight = 70.f;
    
    for (size_t i = 0; i < m_options.size(); ++i)
    {
        float buttonY = startY + i * spacing;
        
        // Draw frame
        sf::RectangleShape frame({frameWidth, frameHeight});
        frame.setPosition({centerX - frameWidth / 2.f, buttonY});
        frame.setFillColor(sf::Color(0, 0, 0, 150));  // Semi-transparent black
        
        // Save button bounds for mouse click detection
        if (m_buttonBounds.size() <= i)
        {
            m_buttonBounds.push_back(sf::FloatRect({centerX - frameWidth / 2.f, buttonY}, {frameWidth, frameHeight}));
        }
        else
        {
            m_buttonBounds[i] = sf::FloatRect({centerX - frameWidth / 2.f, buttonY}, {frameWidth, frameHeight});
        }
        
        // Highlight selected option - toxic green for zombie theme
        if (static_cast<int>(i) == m_selectedIndex)
        {
            frame.setOutlineColor(GameConstants::Colors::MENU_SELECTED);
            frame.setOutlineThickness(3.f);
        }
        else
        {
            frame.setOutlineColor(GameConstants::Colors::MENU_FRAME);
            frame.setOutlineThickness(2.f);
        }
        window.draw(frame);
        
        // Draw option text with highlighted first letter (keyboard shortcut)
		std::string firstLetter = m_options[i].substr(0, 1);//first character
        std::string restOfText = m_options[i].substr(1);
        
        // Create texts for measuring total width
        sf::Text firstLetterText(font, firstLetter, 32);
        sf::Text restText(font, restOfText, 32);
        
        float totalWidth = firstLetterText.getGlobalBounds().size.x + restText.getGlobalBounds().size.x;
        
        // Center text vertically in frame
        float textY;
        if (i == 0)  // NEW GAME - leave room for subtitle
        {
            textY = buttonY + 10.f;
        }
        else  // HELP and EXIT - center vertically
        {
            sf::FloatRect bounds = firstLetterText.getGlobalBounds();
            textY = buttonY + (frameHeight - bounds.size.y) / 2.f - 5.f;
        }
        
        float startX = centerX - totalWidth / 2.f;
        
        // Draw first letter in highlight color (dark green)
        firstLetterText.setFillColor(sf::Color(0, 150, 80));  // Dark green for shortcut
        firstLetterText.setPosition({startX, textY});
        window.draw(firstLetterText);
        
        // Draw rest of text in normal/selected color
        if (static_cast<int>(i) == m_selectedIndex)
        {
            restText.setFillColor(GameConstants::Colors::MENU_SELECTED);
        }
        else
        {
            restText.setFillColor(GameConstants::Colors::MENU_NORMAL);
        }
        restText.setPosition({startX + firstLetterText.getGlobalBounds().size.x, textY});
        window.draw(restText);
        
        // Add subtitle under selected button
        if (static_cast<int>(i) == m_selectedIndex)
        {
            sf::Text subtitle(font, "Click or Press Enter", 14);
            subtitle.setFillColor(sf::Color(200, 200, 200));
            sf::FloatRect subBounds = subtitle.getGlobalBounds();
            subtitle.setPosition({centerX - subBounds.size.x / 2.f, buttonY + 50.f});
            window.draw(subtitle);
        }
    }
    
    // Draw creators credit
    sf::Text credits(font, "Creators: OFEK AND AVIA", 20);
    credits.setFillColor(GameConstants::Colors::MENU_NORMAL);
    sf::FloatRect creditBounds = credits.getGlobalBounds();
    credits.setPosition({centerX - creditBounds.size.x / 2.f, windowSize.y - 50.f});
    window.draw(credits);
}

void Menu::handleEvent(const sf::Event& event, sf::RenderWindow& window)
{
    (void)window;  // Unused parameter
    
    // Handle keyboard input
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::Up)
        {
            m_selectedIndex--;
            if (m_selectedIndex < 0)
            {
                m_selectedIndex = static_cast<int>(m_options.size()) - 1;//if at top, go to bottom
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Down)
        {
            m_selectedIndex++;
            if (m_selectedIndex >= static_cast<int>(m_options.size()))
            {
                m_selectedIndex = 0;
            }
        }
        else if (keyPressed->code == sf::Keyboard::Key::Enter)
        {
            m_optionChosen = true;
        }
        // Keyboard shortcuts for menu options
        else if (keyPressed->code == sf::Keyboard::Key::N)
        {
            m_selectedIndex = 0;  // New Game
            m_optionChosen = true;
        }
        else if (keyPressed->code == sf::Keyboard::Key::H)
        {
            m_selectedIndex = 1;  // Help
            m_optionChosen = true;
        }
        else if (keyPressed->code == sf::Keyboard::Key::E || 
                 keyPressed->code == sf::Keyboard::Key::Escape)
        {
            m_selectedIndex = 2;  // Exit
            m_optionChosen = true;
        }
    }
    
    // Handle mouse click
    if (const auto* mouseClick = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouseClick->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mousePos(static_cast<float>(mouseClick->position.x), 
                                  static_cast<float>(mouseClick->position.y));
            
            for (size_t i = 0; i < m_buttonBounds.size(); ++i)
            {
                if (m_buttonBounds[i].contains(mousePos))
                {
                    m_selectedIndex = static_cast<int>(i);
                    m_optionChosen = true;
                    break;
                }
            }
        }
    }
}

MenuOption Menu::getSelectedOption() const
{
	return static_cast<MenuOption>(m_selectedIndex);//cast index to MenuOption enum
}

bool Menu::isOptionChosen() const
{
    return m_optionChosen;
}

void Menu::reset()
{
    m_selectedIndex = 0;
    m_optionChosen = false;
}

void Menu::drawHelpScreen(sf::RenderWindow& window)
{
    // Draw background
    sf::Texture& bgTexture = ResourceManager::instance().getTexture(GameConstants::Textures::BACKGROUND);
    sf::Sprite background(bgTexture);
    sf::Vector2u windowSize = window.getSize();
    sf::Vector2u texSize = bgTexture.getSize();
    background.setScale({
        static_cast<float>(windowSize.x) / texSize.x,
        static_cast<float>(windowSize.y) / texSize.y
    });
    window.draw(background);
    
    sf::Font& font = ResourceManager::instance().getFont(GameConstants::Fonts::MENU);
    float centerX = windowSize.x / 2.f;
    
    // Title
    sf::Text title(font, "HOW TO PLAY", 48);
    title.setFillColor(sf::Color(120, 20, 20));
    title.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = title.getGlobalBounds();
    title.setPosition({centerX - titleBounds.size.x / 2.f, 30.f});
    window.draw(title);
    
    // Instructions
    std::vector<std::string> instructions = {
        "Arrow Keys - Move",
        "Space - Jump",
        "Z - Dig Left",
        "X - Dig Right",
        "M - Mute/Unmute",
        "Collect all coins to win!",
        "Avoid the zombies!",
        "",
        "Press ESC or ENTER to go back"
    };
    
    float startY = 120.f;
    float lineSpacing = 40.f;
    
    for (size_t i = 0; i < instructions.size(); ++i)
    {
        sf::Text line(font, instructions[i], 24);
        line.setFillColor(sf::Color::White);  // White for better visibility
        sf::FloatRect lineBounds = line.getGlobalBounds();
        line.setPosition({centerX - lineBounds.size.x / 2.f, startY + i * lineSpacing});
        window.draw(line);
    }
}

void Menu::setShowHelp(bool showHelp)
{
    m_showHelp = showHelp;
}

bool Menu::isShowingHelp() const
{
    return m_showHelp;
}

// ========== Sound Mute Functions ==========

void Menu::toggleMute()
{
    m_isMuted = !m_isMuted;
}

void Menu::drawMuteIcon(sf::RenderWindow& window)
{
    // Determine which texture to use
    const char* textureName = m_isMuted ? GameConstants::Textures::VOLUME_OFF : GameConstants::Textures::VOLUME_ON;
    sf::Texture& texture = ResourceManager::instance().getTexture(textureName);
    
    // Draw icon at top-right corner
    sf::Sprite icon(texture);
    
    // Scale to standard HUD icon size (e.g. 30px height)
    float targetHeight = 30.f;
    float scale = targetHeight / texture.getSize().y;
    icon.setScale({scale, scale});
    
    float windowWidth = static_cast<float>(window.getSize().x);
    float iconX = windowWidth - (texture.getSize().x * scale) - 10.f;  // 10px from right edge
    float iconY = 10.f;
    
    icon.setPosition({iconX, iconY});
    window.draw(icon);
}

bool Menu::isMuted() const
{
    return m_isMuted;
}

void Menu::setMuted(bool muted)
{
    m_isMuted = muted;
}

sf::FloatRect Menu::getMuteIconBounds(sf::RenderWindow& window) const
{
    const char* textureName = m_isMuted ? GameConstants::Textures::VOLUME_OFF : GameConstants::Textures::VOLUME_ON;
    sf::Texture& texture = ResourceManager::instance().getTexture(textureName);
    
    float targetHeight = 30.f;
    float scale = targetHeight / texture.getSize().y;
    
    float windowWidth = static_cast<float>(window.getSize().x);
    float iconX = windowWidth - (texture.getSize().x * scale) - 10.f;
    float iconY = 10.f;
    float iconWidth = texture.getSize().x * scale;
    float iconHeight = texture.getSize().y * scale;
    
    return sf::FloatRect({iconX, iconY}, {iconWidth, iconHeight});
}
