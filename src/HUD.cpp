#include "Hud.h"
#include "Resourcemanager.h"
#include "Constants.h"
#include <string>

void HUD::draw(sf::RenderWindow& window)
{
    sf::Font& font = ResourceManager::instance().getFont(GameConstants::Fonts::HUD);  // Horror font with readable numbers
    float windowWidth = static_cast<float>(window.getSize().x);
    
    // Evenly distribute 4 elements across the window
    float spacing = windowWidth / 5.f;  // 5 sections for 4 elements
    float yPos = 12.f;  // Centered vertically in HUD
    
    // Level - leftmost
    sf::Text levelText(font);
    levelText.setString(m_levelStr);
    levelText.setCharacterSize(24);
    levelText.setFillColor(sf::Color::White);
    sf::FloatRect levelBounds = levelText.getGlobalBounds();
    levelText.setPosition({spacing * 0.5f, yPos});
    window.draw(levelText);
    
    // Score - second from left
    sf::Text scoreText(font);
    scoreText.setString(m_scoreStr);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition({spacing * 1.5f, yPos});
    window.draw(scoreText);
    
    // Coin pile icon next to score
    sf::Texture& coinTexture = ResourceManager::instance().getTexture(GameConstants::Textures::PILE_OF_COINS);
    sf::Sprite coinIcon(coinTexture);
    float coinScale = GameConstants::HUD_ICON_HEIGHT / coinTexture.getSize().y;
    coinIcon.setScale({coinScale, coinScale});
    sf::FloatRect scoreBounds = scoreText.getGlobalBounds();
    coinIcon.setPosition({scoreBounds.position.x + scoreBounds.size.x + 10.f, 8.f});
    window.draw(coinIcon);
    
    // Lives text - center left
    sf::Text livesText(font);
    livesText.setString(m_livesStr);
    livesText.setCharacterSize(24);
    livesText.setFillColor(sf::Color::White);
    livesText.setPosition({spacing * 2.5f, yPos});
    window.draw(livesText);
    
    // Draw lives icon based on current lives - next to lives text
    if (m_lives >= 1 && m_lives <= GameConstants::STARTING_LIVES)
    {
        std::string livesTextureName = "HUD/lives" + std::to_string(m_lives);
        sf::Texture& livesTexture = ResourceManager::instance().getTexture(livesTextureName);
        sf::Sprite livesIcon(livesTexture);
        
        // Fixed target size for consistent icon display
        float targetHeight = GameConstants::HUD_LIVES_ICON_HEIGHT;
        float iconScale = targetHeight / livesTexture.getSize().y;
        livesIcon.setScale({iconScale, iconScale});
        livesIcon.setPosition({spacing * 2.5f + 90.f, 2.f});  // Right after lives text
        window.draw(livesIcon);
    }
    
    // Timer - rightmost
    sf::Text timerText(font);
    timerText.setString(m_timerStr);
    timerText.setCharacterSize(24);
    timerText.setFillColor(sf::Color::White);
    timerText.setPosition({spacing * 4.f, yPos});
    window.draw(timerText);
    
    // Timer icon next to time
    sf::Texture& timerTexture = ResourceManager::instance().getTexture(GameConstants::Textures::TIMER);
    sf::Sprite timerIcon(timerTexture);
    float timerScale = GameConstants::HUD_ICON_HEIGHT / timerTexture.getSize().y;
    timerIcon.setScale({timerScale, timerScale});
    sf::FloatRect timerBounds = timerText.getGlobalBounds();
    timerIcon.setPosition({timerBounds.position.x + timerBounds.size.x + 10.f, 8.f});
    window.draw(timerIcon);
}

void HUD::update(int level, int score, int lives, float time)
{
    m_levelStr = "Level: " + std::to_string(level);
    m_scoreStr = "Score: " + std::to_string(score);
    m_livesStr = "Lives: " + std::to_string(lives);
    m_lives = lives;  // Store for icon drawing
    
    // Show minimum of 1 so timer goes 1 -> TIME'S UP (no 0 displayed)
    int displayTime = static_cast<int>(time);
    if (displayTime < 1) displayTime = 1;
    m_timerStr = "Time: " + std::to_string(displayTime);
}
