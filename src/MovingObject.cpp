#include "MovingObject.h"
#include "Constants.h"

// Speed and velocity accessors
float MovingObject::getSpeed() const { return m_speed; }
void MovingObject::setSpeed(float speed) { m_speed = speed; }

sf::Vector2f MovingObject::getVelocity() const { return m_velocity; }
void MovingObject::setVelocity(const sf::Vector2f& velocity) { m_velocity = velocity; }
void MovingObject::setVelocityX(float vx) { m_velocity.x = vx; }
void MovingObject::setVelocityY(float vy) { m_velocity.y = vy; }
void MovingObject::addVelocityY(float delta) { m_velocity.y += delta; }

// State flag accessors
void MovingObject::setOnGround(bool grounded) { m_onGround = grounded; }
void MovingObject::setOnLadder(bool onLadder) { m_onLadder = onLadder; }
void MovingObject::setOnPole(bool onPole) { m_onPole = onPole; }
void MovingObject::setLadderCenterX(float x) { m_ladderCenterX = x; }

bool MovingObject::isOnGround() const { return m_onGround; }
bool MovingObject::isOnLadder() const { return m_onLadder; }
bool MovingObject::isOnPole() const { return m_onPole; }
float MovingObject::getLadderCenterX() const { return m_ladderCenterX; }

// Reset state flags - base implementation
void MovingObject::resetStateFlags()
{
    m_onGround = false;
    m_onLadder = false;
    m_onPole = false;
}

// Reset movement state to stable grounded position
void MovingObject::resetMovementState()
{
    m_velocity = {0.f, 0.f};
    m_onGround = true;
    m_onLadder = false;
    m_onPole = false;
}

// Common physics: apply gravity when not grounded/on ladder/on pole
void MovingObject::applyGravity(float deltaTime)
{
    if (!m_onGround && !m_onLadder && !m_onPole)
    {
        m_velocity.y += GameConstants::GRAVITY * deltaTime;
        if (m_velocity.y > GameConstants::MAX_FALL_SPEED)
        {
            m_velocity.y = GameConstants::MAX_FALL_SPEED;
        }
    }
    else if (m_onGround && !m_onLadder)
    {
        m_velocity.y = 0;
    }
}

// Update sprite direction based on velocity
void MovingObject::updateSpriteDirection(float texWidth, float scaleX, float scaleY)
{
    if (!m_sprite.has_value()) return;

    if (m_velocity.x > 0)
    {
        m_sprite->setScale({-scaleX, scaleY});
        m_sprite->setOrigin({texWidth, 0.f});
    }
    else if (m_velocity.x < 0)
    {
        m_sprite->setScale({scaleX, scaleY});
        m_sprite->setOrigin({0.f, 0.f});
    }
}

// Sync sprite position with m_position
void MovingObject::syncSpritePosition()
{
    if (m_sprite.has_value())
    {
        m_sprite->setPosition(m_position);
    }
}

// Get texture scale factors for TILE_SIZE scaling
MovingObject::TextureScale MovingObject::getTextureScale() const
{
    TextureScale scale;
    if (m_sprite.has_value())
    {
        const sf::Texture& tex = m_sprite->getTexture();
        sf::Vector2u texSize = tex.getSize();
        scale.texWidth = static_cast<float>(texSize.x);
        scale.texHeight = static_cast<float>(texSize.y);
        scale.scaleX = GameConstants::TILE_SIZE / scale.texWidth;
        scale.scaleY = GameConstants::TILE_SIZE / scale.texHeight;
    }
    return scale;
}

MovingObject::CollisionResult MovingObject::resolveCollisionWithSolid(const sf::FloatRect& solidBounds)
{
    CollisionResult result;
    sf::FloatRect myBounds = getBounds();

    // Calculate overlap on all sides
    float overlapLeft = (myBounds.position.x + myBounds.size.x) - solidBounds.position.x;
    float overlapRight = (solidBounds.position.x + solidBounds.size.x) - myBounds.position.x;
    float overlapTop = (myBounds.position.y + myBounds.size.y) - solidBounds.position.y;
    float overlapBottom = (solidBounds.position.y + solidBounds.size.y) - myBounds.position.y;

    // Find smallest overlap in each direction
    float minOverlapX = (overlapLeft < overlapRight) ? -overlapLeft : overlapRight;
    float minOverlapY = (overlapTop < overlapBottom) ? -overlapTop : overlapBottom;

    // Push in direction of smallest overlap
    if (std::abs(minOverlapX) < std::abs(minOverlapY))
    {
        // Horizontal collision
        m_position.x += minOverlapX;
        result.wasHorizontal = true;
    }
    else
    {
        // Vertical collision
        m_position.y += minOverlapY;
        if (minOverlapY < 0)
        {
            // Landed on top of solid
            result.landedOnTop = true;
        }
    }

    // Update sprite position
    syncSpritePosition();

    return result;
}
