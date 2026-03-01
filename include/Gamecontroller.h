#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include "Board.h"
#include "Menu.h"
#include "HUD.h"
#include "ResourceManager.h"

enum class GameState
{
    Menu,
    Help,
    Playing,
    Victory,
    GameOver,
    TimeUp
};

class GameController
{
public:
    void run();

private:
    // Core game loop
    void handleEvents();
    void update();
    void render();
    
    // Game functions
    void startNewGame();
    void loadNextLevel();
    void showEndScreen(const std::string& message, sf::Color color);
    void resetToMenu();
    
    // Mute handling
    void handleMuteInput();
    void updateSoundVolumes();  // Update all sound volumes based on mute state

    sf::RenderWindow m_window;
    Board m_board;
    Menu m_menu;
    HUD m_hud;
    sf::Clock m_clock;
    sf::Clock m_gameTimer;      // Timer for HUD display
    sf::Clock m_endScreenTimer; // Timer for 3,2,1 screen display
    
    GameState m_state = GameState::Menu;
    std::string m_endMessage;
    sf::Color m_endMessageColor;
    bool m_skipNextFrame = false;  // Skip first frame after level load to prevent falling
    bool m_countdownPlaying = false;  // Track if countdown sound is playing
    bool m_mKeyWasPressed = false;  // Track M key for mute toggle
    bool m_lastMuteState = false;  // Track mute state changes
    
    // Sound effects
    std::optional<sf::Sound> m_gameOverSound;
    std::optional<sf::Sound> m_winSound;
    std::optional<sf::Sound> m_countdownSound;
    
    // Background music
    sf::Music m_backgroundMusic;
};