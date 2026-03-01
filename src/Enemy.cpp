#include "Enemy.h"
#include "Resourcemanager.h"
#include "Constants.h"
#include "StaticObject.h"
#include <cmath>

using namespace GameConstants;

Enemy::Enemy()
{
    setTexture(ResourceManager::instance().getTexture(Textures::ENEMY));
    setSpeed(ENEMY_SPEED);
    setVelocity({0.f, 0.f});
    m_patrolDirection = 1;  // Start moving right
}

void Enemy::draw(sf::RenderWindow& window)
{
    if (m_sprite.has_value())
    {
        window.draw(*m_sprite);
    }
}

sf::FloatRect Enemy::getBounds()
{
    const float SHRINK = GameConstants::ENEMY_HITBOX_SHRINK;

    if (m_sprite.has_value())
    {
        sf::FloatRect bounds = m_sprite->getGlobalBounds();
        return sf::FloatRect(
            {bounds.position.x + SHRINK, bounds.position.y + SHRINK},
            {bounds.size.x - SHRINK * 2, bounds.size.y - SHRINK * 2}
        );
    }
    return sf::FloatRect(
        {m_position.x + SHRINK, m_position.y + SHRINK},
        {TILE_SIZE - SHRINK * 2, TILE_SIZE - SHRINK * 2}
    );
}

void Enemy::move(float deltaTime)
{
    // Apply gravity using common helper
    applyGravity(deltaTime);

    sf::Vector2f velocity = getVelocity();
    m_position.x += velocity.x * deltaTime;
    m_position.y += velocity.y * deltaTime;

    // Snap to ladder center when on ladder (only if not moving horizontally)
    if (isOnLadder() && std::abs(velocity.x) < 0.1f)
    {
        float ladderX = getLadderCenterX();
        float diff = ladderX - m_position.x;
        if (std::abs(diff) > LADDER_SNAP_THRESHOLD)
        {
            m_position.x += diff * ENEMY_LADDER_SNAP_FACTOR;
        }
        else
        {
            m_position.x = ladderX;
        }
    }

    updateSprite();
}

void Enemy::updateSprite()
{
    if (!m_sprite.has_value()) return;

    // Handle texture switching for pole
    if (isOnPole())
    {
        setTexture(ResourceManager::instance().getTexture(Textures::ENEMY_POLE));
    }
    else
    {
        setTexture(ResourceManager::instance().getTexture(Textures::ENEMY));
    }

    // Calculate scale and update sprite direction
    TextureScale ts = getTextureScale();
    updateSpriteDirection(ts.texWidth, ts.scaleX, ts.scaleY);
    syncSpritePosition();
}


void Enemy::updateEnemy(sf::Vector2f playerPos)
{
    float speed = getSpeed();

    // Convert positions to tile coordinates
    int enemyTileX = static_cast<int>(m_position.x / TILE_SIZE);
    int enemyTileY = static_cast<int>((m_position.y - HUD_HEIGHT) / TILE_SIZE);
    int playerTileX = static_cast<int>(playerPos.x / TILE_SIZE);
    int playerTileY = static_cast<int>((playerPos.y - HUD_HEIGHT) / TILE_SIZE);

    // Calculate Manhattan distance
    float manhattanDist = static_cast<float>(std::abs(enemyTileX - playerTileX) + std::abs(enemyTileY - playerTileY));

    bool sameRow = (enemyTileY == playerTileY);
    bool sameColumn = (enemyTileX == playerTileX);
    bool inRange = (manhattanDist <= ENEMY_CHASE_DISTANCE);

    // State transitions
    if (m_behaviorState == EnemyState::Patrol)
    {
        if (inRange)
        {
            m_behaviorState = EnemyState::Chase;
        }
    }
    else // Chase mode
    {
        if (!inRange)
        {
            m_behaviorState = EnemyState::Patrol;
        }
    }

    if (m_behaviorState == EnemyState::Chase)
    {
        // === CHASE MODE ===
        // Move directly toward player

        if (sameRow)
        {
            // Same row - move horizontally toward player
            int desiredDir = (playerTileX > enemyTileX) ? 1 : -1;
            setVelocityX(desiredDir * speed);
            m_patrolDirection = desiredDir;
        }
        else if (sameColumn && isOnLadder())
        {
            // Same column and on ladder - move vertically toward player
            setVelocityX(0);
            if (playerTileY > enemyTileY)
            {
                setVelocityY(speed);  // Move down
                m_lastVerticalDirection = 1;
            }
            else if (playerTileY < enemyTileY)
            {
                setVelocityY(-speed);  // Move up
                m_lastVerticalDirection = -1;
            }
        }
        else
        {
            // Not on same row - try to close the X distance
            int desiredDir = 0;
            if (playerTileX > enemyTileX)
                desiredDir = 1;
            else if (playerTileX < enemyTileX)
                desiredDir = -1;
            else
                desiredDir = m_patrolDirection;

            setVelocityX(desiredDir * speed);
            m_patrolDirection = desiredDir;
        }
    }
    else
    {
        // === PATROL MODE ===
        // Simply move in patrol direction, wall collisions will reverse direction
        setVelocityX(m_patrolDirection * speed);
    }

    // If on pole, can move horizontally but not fall
    if (isOnPole())
    {
        setVelocityY(0);
    }

    // If on ladder and Patrol mode and on ground, stop vertical movement
    if (isOnLadder() && m_behaviorState == EnemyState::Patrol && isOnGround())
    {
        setVelocityY(0);
    }

    // Reset vertical direction when on ground or not on ladder
    if (isOnGround() || !isOnLadder())
    {
        m_lastVerticalDirection = 0;
    }
}

// Called when enemy hits a wall - reverse direction
void Enemy::reverseDirection()
{
    m_patrolDirection = -m_patrolDirection;
    setVelocityX(m_patrolDirection * getSpeed());
}

// ===== Position Management =====

void Enemy::resetToStart()
{
    m_position = m_startPosition;
    resetMovementState();
    m_behaviorState = EnemyState::Patrol;
    m_patrolDirection = 1;
    syncSpritePosition();
}

void Enemy::saveStartPosition()
{
    m_startPosition = m_position;
}

// ===== Collision Handlers =====

void Enemy::handleCollision(Wall& wall)
{
    CollisionResult result = resolveCollisionWithSolid(wall.getBounds());
    if (result.wasHorizontal)
    {
        reverseDirection();
    }
    if (result.landedOnTop)
    {
        setOnGround(true);
        setVelocityY(0);
    }
}

void Enemy::handleCollision(Floor& floor)
{
    CollisionResult result = resolveCollisionWithSolid(floor.getBounds());
    if (result.wasHorizontal)
    {
        reverseDirection();
    }
    if (result.landedOnTop)
    {
        setOnGround(true);
        setVelocityY(0);
    }
}

void Enemy::handleCollision(DiggableFloor& floor)
{
    if (!floor.isDigged())
    {
        CollisionResult result = resolveCollisionWithSolid(floor.getBounds());
        if (result.wasHorizontal)
        {
            reverseDirection();
        }
        if (result.landedOnTop)
        {
            setOnGround(true);
            setVelocityY(0);
        }
    }
}

void Enemy::handleCollision(Ladder& ladder)
{
    setOnLadder(true);
    sf::FloatRect ladderBounds = ladder.getBounds();
    setLadderCenterX(ladderBounds.position.x);
}

void Enemy::handleCollision(Pole& pole)
{
    setOnPole(true);
    setVelocityY(0);

    sf::FloatRect poleBounds = pole.getBounds();
    const float HAND_OFFSET = GameConstants::TILE_SIZE * GameConstants::POLE_HAND_OFFSET;
    m_position.y = poleBounds.position.y + HAND_OFFSET;

    syncSpritePosition();
}

void Enemy::handleCollision(Enemy& other)
{
    // Prevent enemies from merging
    sf::FloatRect thisBounds = getBounds();
    sf::FloatRect otherBounds = other.getBounds();

    float overlapLeft = (otherBounds.position.x + otherBounds.size.x) - thisBounds.position.x;
    float overlapRight = (thisBounds.position.x + thisBounds.size.x) - otherBounds.position.x;
    
    // Only handle horizontal overlapping for patrol merging prevention
    if (overlapLeft > 0 && overlapRight > 0)
    {
        float minOverlapX = (overlapLeft < overlapRight) ? -overlapLeft : overlapRight;
        
        // Push the other enemy out using public API
        sf::Vector2f otherPos = other.getPosition();
        other.setPosition(otherPos.x + minOverlapX, otherPos.y);
        
        // Reverse directions for both
        other.reverseDirection();
        reverseDirection();
    }
}

void Enemy::processCollision(Collider& other)
{
    other.handleCollision(*this);
}

void Enemy::resetStateFlags()
{
    MovingObject::resetStateFlags();
}

bool Enemy::isRemoved() const
{
    return m_isRemoved;
}

void Enemy::remove()
{
    m_isRemoved = true;
}
