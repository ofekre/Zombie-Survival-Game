#pragma once
#include "MovingObject.h"
#include <SFML/Audio.hpp>
#include <optional>
#include "Constants.h"

// Forward declarations
class Wall;
class Floor;
class DiggableFloor;
class Ladder;
class Pole;
class Coin;
class Enemy;

class Player : public MovingObject
{
public:
    Player();
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() override;
    void move(float deltaTime) override;
    void updateSprite();  // Update sprite texture and direction
    void handleInput();
    void dig(bool left);
    void update(float deltaTime);  // Update invincibility timer
    
    // Double dispatch collision
    void processCollision(Collider& other) override;
    
    // Collision handlers for each type
    void handleCollision(Wall& wall) override;
    void handleCollision(Floor& floor) override;
    void handleCollision(DiggableFloor& floor) override;
    void handleCollision(Ladder& ladder) override;
    void handleCollision(Pole& pole) override;
    void handleCollision(Coin& coin) override;
    void handleCollision(Enemy& enemy) override;
    
    // Getters for HUD
    int getLives() const;
    int getScore() const;
    void addScore(int points);
    void loseLife();
    
    // Position management
    void saveStartPosition();
    void resetToStart();  // Return to start position (on enemy hit)
    
    // Stabilize player at current position (prevent falling)
    void stabilize();
    
    void reset();  // Full reset for new game
    
    // State getters
    bool isInvincible() const;
    bool needsPositionReset() const;
    
    // Dig request (Board will handle the actual digging)
    bool wantsToDigLeft() const;
    bool wantsToDigRight() const;
    void clearDigFlags();
    
    // Reset state flags (called by Board before collision detection)
    void resetStateFlags() override;
    
    // Mute control
    void setMuted(bool muted);
    
    // Stop falling sound (called when falling into pit)
    void stopFallingSound();

private:
    int m_lives = GameConstants::STARTING_LIVES;
    int m_score = 0;
    bool m_needsReset = false;  // Flag to signal Board to reset positions
    float m_invincibilityTimer = 0.f;
    sf::Vector2f m_startPosition;
    
    // Texture state
    bool m_isLadderTextureActive = false;
    bool m_isPoleTextureActive = false;
    bool m_isStandingTextureActive = false;
    
    // Sound effects
    std::optional<sf::Sound> m_coinSound;
    std::optional<sf::Sound> m_digSound;
    std::optional<sf::Sound> m_jumpSound;
    std::optional<sf::Sound> m_enemyHitSound;
    std::optional<sf::Sound> m_fallingSound;
    bool m_isFallingSoundPlaying = false;
    bool m_isJumping = false;  // Track if player jumped (don't play fall sound if they did)
    float m_fallSoundImmunityTimer = 0.5f;  // Don't play fall sound immediately after spawn
    
    // Dig request flags
    bool m_wantsDigLeft = false;
    bool m_wantsDigRight = false;
    bool m_droppingFromPole = false;
    bool m_isTouchingPole = false;
    
    // Key state tracking (to prevent repeated digging while holding key)
    bool m_zKeyWasPressed = false;
    bool m_xKeyWasPressed = false;
};