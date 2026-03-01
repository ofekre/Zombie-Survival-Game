#pragma once
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <SFML/Audio.hpp>
#include "GameObject.h"
#include "MovingObject.h"
#include "Player.h"
#include "Enemy.h"
#include "LevelLoader.h"

class Board
{
public:
    void loadFromFile(const std::string& filename);
    void nextLevel();                // Load next level
    void reloadCurrentLevel();       // Reload level keeping score (for time up)
    void draw(sf::RenderWindow& window);
    void update(float deltaTime);
    bool isLevelComplete() const;

    // Get board dimensions in pixels
    float getWidth() const;
    float getHeight() const;

    // Level info
    int getLevelNumber() const;
    bool hasMoreLevels() const;

    // Time limit per level (in seconds)
    float getLevelTimeLimit() const;

    // Getters for HUD
    int getPlayerScore() const;
    int getPlayerLives() const;
    void losePlayerLife();

    // Scoring with level multiplier
    void addCoinScore();
    void addLevelCompleteScore();

    // Mute control
    void setMuted(bool muted);

    // Stop player's falling sound (for end game states)
    void stopPlayerFallingSound();

private:
    void loadLevel(int levelIndex);  // Load specific level (internal use only)
    void applyLevelData(std::unique_ptr<LevelData> data);  // Apply parsed level data
    void checkGroundForObject(MovingObject& obj);  // Check if object is on solid ground
    void checkLadderPoleForObject(MovingObject& obj);  // Check ladder/pole collision
    void resetAllEnemies();  // Reset all enemies to start position
    bool isSolidObject(GameObject* obj) const;  // Check if object blocks movement

    LevelLoader m_levelLoader;
    std::vector<std::unique_ptr<GameObject>> m_objects;
    Player m_player;
    std::vector<Enemy> m_enemies;

    int m_currentLevel = 0;

    float m_width = 0.f;
    float m_height = 0.f;

    // Timer for fall immunity after reset (simple and reliable)
    float m_fallImmunityTimer = 0.f;

    // Death pause timer - hide board for 3 seconds after falling into pit
    float m_deathPauseTimer = 0.f;
    bool m_isDeathPaused = false;

    // Sound for falling into pit
    std::optional<sf::Sound> m_fallingToHoleSound;
    bool m_isMuted = false;

    // Board background sprite
    std::optional<sf::Sprite> m_boardBackground;
};