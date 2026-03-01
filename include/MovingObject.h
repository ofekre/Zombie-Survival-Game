#pragma once
#include "GameObject.h"
#include <cmath>

class MovingObject : public GameObject
{
public:
    virtual void move(float deltaTime) = 0;

    // State flag setters (called by collision handlers)
    void setOnGround(bool grounded);
    void setOnLadder(bool onLadder);
    void setOnPole(bool onPole);
    void setLadderCenterX(float x);

    // Reset all movement state flags (called before collision detection)
    virtual void resetStateFlags();

protected:
    // Reset movement state to stable grounded position (used by resetToStart)
    void resetMovementState();
    // Speed and velocity accessors for derived classes
    float getSpeed() const;
    void setSpeed(float speed);

    sf::Vector2f getVelocity() const;
    void setVelocity(const sf::Vector2f& velocity);
    void setVelocityX(float vx);
    void setVelocityY(float vy);
    void addVelocityY(float delta);

    // State flag getters
    bool isOnGround() const;
    bool isOnLadder() const;
    bool isOnPole() const;
    float getLadderCenterX() const;

    // Common physics helpers
    void applyGravity(float deltaTime);
    void updateSpriteDirection(float texWidth, float scaleX, float scaleY);
    void syncSpritePosition();

    // Texture scale info
    struct TextureScale
    {
        float texWidth = 0.f;
        float texHeight = 0.f;
        float scaleX = 1.f;
        float scaleY = 1.f;
    };
    TextureScale getTextureScale() const;

    // Collision resolution result
    struct CollisionResult
    {
        bool wasHorizontal = false;  // True if pushed horizontally
        bool landedOnTop = false;    // True if landed on top of solid
    };

    // Resolve collision with a solid object - returns collision info
    CollisionResult resolveCollisionWithSolid(const sf::FloatRect& solidBounds);

private:
    float m_speed = 0.f;
    sf::Vector2f m_velocity = {0.f, 0.f};

    // State flags - shared by Player and Enemy (accessed via getters/setters)
    bool m_onGround = false;
    bool m_onLadder = false;
    bool m_onPole = false;
    float m_ladderCenterX = 0.f;
};