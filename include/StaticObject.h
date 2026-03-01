#pragma once
#include "GameObject.h"

class Wall : public GameObject
{
public:
    Wall();
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() override;
    void processCollision(Collider& other) override;
    bool isSolid() const override;
};

// Abstract base class for all floor types
class BaseFloor : public GameObject
{
public:
    virtual ~BaseFloor() = default;
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() override;
    
protected:
    BaseFloor() = default;  // Can't instantiate directly
};

class Floor : public BaseFloor
{
public:
    Floor();
    sf::FloatRect getBounds() override;  // Extended hitbox to prevent entities falling between tiles
    void processCollision(Collider& other) override;
    bool isSolid() const override;
};

class DiggableFloor : public BaseFloor
{
public:
    DiggableFloor();
    void draw(sf::RenderWindow& window) override;  // Override to hide when digged
    sf::FloatRect getBounds() override;  // Override to return empty when digged
    void dig();
    bool isDigged() const;
    void update(float deltaTime);  // Update dig animation timer
    void processCollision(Collider& other) override;
    bool isSolid() const override;
    bool isDiggable() const override;

private:
    bool m_isDigged = false;
    bool m_showDigAnimation = false;
    float m_digAnimTimer = 0.f;
    static constexpr float DIG_ANIM_DURATION = 0.3f;  // Show digging texture for 0.3 seconds
};

class Ladder : public GameObject
{
public:
    Ladder();
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() override;
    void processCollision(Collider& other) override;
    bool isLadder() const override;
};

class Pole : public GameObject
{
public:
    Pole();
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() override;
    void processCollision(Collider& other) override;
    bool isPole() const override;
};

class Coin : public GameObject
{
public:
    Coin();
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() override;
    void collect();
    bool isCollected() const;
    bool wasJustCollected() const;
    void clearJustCollected();
    void processCollision(Collider& other) override;
    bool isCoin() const override;

private:
    bool m_collected = false;
    bool m_justCollected = false;
};