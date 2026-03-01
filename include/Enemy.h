#pragma once

#include "MovingObject.h"
#include "Constants.h"
#include <SFML/Graphics.hpp>
#include <vector>

// Forward declarations
class Wall;
class Floor;
class DiggableFloor;
class Ladder;
class Pole;
class Enemy;

enum class EnemyState
{
    Patrol,  // Default: move left/right on platforms
    Chase    // Triggered when close to player
};

class Enemy : public MovingObject
{
public:
    Enemy();
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() override;
    void move(float deltaTime) override;
    void updateSprite();  // Update sprite texture and direction

    void updateEnemy(sf::Vector2f playerPos);
    
    // Position management
    void saveStartPosition();
    void resetToStart();
    
    // Double dispatch
    void processCollision(Collider& other) override;
    
    // Collision handlers for each type
    void handleCollision(Wall& wall) override;
    void handleCollision(Floor& floor) override;
    void handleCollision(DiggableFloor& floor) override;
    void handleCollision(Ladder& ladder) override;
    void handleCollision(Pole& pole) override;
    void handleCollision(Enemy& enemy) override;
    
    // Reset state flags (called by Board before collision detection)
    void resetStateFlags() override;
    
    // Reverse patrol direction (called on wall collision)
    void reverseDirection();
    
    // Check if enemy is removed from game
    bool isRemoved() const;
    void remove();

private:
    sf::Vector2f m_startPosition;
    bool m_isRemoved = false;
    
    // Behavior State
    EnemyState m_behaviorState = EnemyState::Patrol;
    int m_patrolDirection = 1;  // 1 = right, -1 = left
    int m_lastVerticalDirection = 0; // 1 (down), -1 (up), 0 (none)
};
