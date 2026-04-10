#pragma once
#include <SFML/Graphics.hpp>

namespace GameConstants
{
    // Tile size in pixels
	constexpr float TILE_SIZE = 50.0f;//size of one tile in pixels
    
    // HUD settings
	constexpr float HUD_HEIGHT = 50.0f;  // height of HUD area at top of screen
    
    // Window settings
	constexpr unsigned int FRAME_RATE = 60;// Target frame rate
    
    // Player settings
    constexpr float PLAYER_SPEED = 250.0f;
    constexpr float PLAYER_HITBOX_SHRINK = 6.0f;  // Shrink hitbox by this many pixels on each side
    constexpr float STANDING_SCALE_BOOST = 1.15f;  // Standing texture scale multiplier
    constexpr float FALL_SOUND_THRESHOLD = 100.0f;  // Velocity threshold for falling sound
    constexpr float FALL_SOUND_IMMUNITY = 0.5f;  // Brief immunity after respawn
    constexpr float LADDER_SNAP_THRESHOLD = 1.0f;  // Distance threshold for snapping to ladder
    constexpr float LADDER_SNAP_SPEED_FACTOR = 0.5f;  // Speed factor for ladder centering
    
    // Enemy settings
    constexpr float ENEMY_SPEED = 50.0f;
    constexpr float ENEMY_HITBOX_SHRINK = 4.0f;  // Shrink hitbox by this many pixels on each side
    constexpr float ENEMY_CHASE_DISTANCE = 20.0f;  // Manhattan distance in tiles to trigger chase
    constexpr float ENEMY_LADDER_SNAP_FACTOR = 0.3f;  // Speed factor for enemy ladder centering
    
    // Physics
    constexpr float GRAVITY = 5000.0f;
    constexpr float MAX_FALL_SPEED = 500.0f;  // Max falling speed (10 tiles/sec at 50px tile)
    constexpr float JUMP_FORCE = 1100.0f;
    constexpr float POLE_HAND_OFFSET = 0.35f;  // Player hand offset on pole (as fraction of tile)
    constexpr float INVINCIBILITY_DURATION = 2.0f; // Seconds of invincibility after hit
    constexpr float DEATH_PAUSE_DURATION = 3.0f;  // Seconds to hide board after falling into pit
    constexpr int STARTING_LIVES = 3;

    // Colors
    namespace Colors
    {
        inline const sf::Color MENU_TITLE = sf::Color(120, 20, 20);      // Dark blood red
        inline const sf::Color MENU_SELECTED = sf::Color(120, 140, 40);  // Sickly mucus green
        inline const sf::Color MENU_NORMAL = sf::Color(150, 150, 150);   // Gray
        inline const sf::Color MENU_FRAME = sf::Color(70, 20, 20);       // Dark red frame
        inline const sf::Color TIME_UP = sf::Color(195, 185, 90);        // Sickly pale yellow-green
        inline const sf::Color VICTORY = sf::Color::Green;
        inline const sf::Color GAME_OVER = sf::Color::Red;
    }
    
    // Scoring
    constexpr int COIN_SCORE_MULTIPLIER = 2;    // score = 2 * level
    constexpr int LEVEL_COMPLETE_MULTIPLIER = 50; // score = 50 * level
    
    // Level time limits (in seconds)
    constexpr float LEVEL_1_TIME = 40.0f;
    constexpr float LEVEL_2_TIME = 70.0f;
    constexpr float LEVEL_3_TIME = 50.0f;
    constexpr float DEFAULT_LEVEL_TIME = 60.0f;
    
    // Menu settings
    constexpr unsigned int MENU_WINDOW_WIDTH = 1000;
    constexpr unsigned int MENU_WINDOW_HEIGHT = 750;
    
    // HUD icon settings
    constexpr float HUD_ICON_HEIGHT = 35.0f;
    constexpr float HUD_LIVES_ICON_HEIGHT = 45.0f;
    
    // Board file characters
    namespace Chars
    {
        constexpr char PLAYER = '@';
        constexpr char ENEMY = '%';
        constexpr char COIN = '*';
        constexpr char WALL = '#';
        constexpr char DIGGABLE_FLOOR = '^';  // Shift+6
        constexpr char LADDER = 'H';
        constexpr char POLE = '-';
    }//
    
    // Resource paths (relative to resources/textures/ folder)
    namespace Textures
    {
        // Player textures (in player/ subfolder)
        constexpr const char* PLAYER = "player/player";
        constexpr const char* PLAYER_LADDER = "player/playerupanddown";
        constexpr const char* PLAYER_POLE = "player/playerclimb";
        constexpr const char* PLAYER_STANDING = "player/playerStanding";
        
        // Enemy textures (in enemies/ subfolder)
        constexpr const char* ENEMY = "enemies/enemy";
        constexpr const char* ENEMY_POLE = "enemies/enemyclimb";
        
        // Item textures (in items/ subfolder)
        constexpr const char* COIN = "items/coin";
        
        // Tile textures (in tiles/ subfolder)
        constexpr const char* WALL = "tiles/wall";
        constexpr const char* GROUND = "tiles/ground";
        constexpr const char* DIGGING_GROUND = "tiles/diggingGround";
        constexpr const char* LADDER = "tiles/ladder";
        constexpr const char* POLE = "tiles/pole";
        
        // HUD textures (in HUD/ subfolder)
        constexpr const char* PILE_OF_COINS = "HUD/pileOfCoins";
        constexpr const char* TIMER = "HUD/timer";
        constexpr const char* LIVES1 = "HUD/lives1";
        constexpr const char* LIVES2 = "HUD/lives2";
        constexpr const char* LIVES3 = "HUD/lives3";
        constexpr const char* VOLUME_ON = "HUD/volume-on";
        constexpr const char* VOLUME_OFF = "HUD/volume-off";
        
        // Background textures (in backgrounds/ subfolder)
        constexpr const char* BACKGROUND = "backgrounds/background";
        constexpr const char* GAME_OVER_BACKGROUND = "backgrounds/gameOverBackground";
        constexpr const char* VICTORY_BACKGROUND = "backgrounds/victoryBackground";
        constexpr const char* BOARD_BACKGROUND = "backgrounds/boardbackground";
    }
    
    namespace Fonts
    {
        constexpr const char* MENU = "Eater-Regular";
        constexpr const char* HUD = "Creepster-Regular";
    }
    
    namespace Sounds
    {
        constexpr const char* WIN = "Win"; // resources/sounds/Win.wav
        constexpr const char* FALLING = "falling"; // resources/sounds/falling.wav
    }
}


