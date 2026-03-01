#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include "Collider.h"

class GameObject : public Collider
{
public:
    virtual ~GameObject() = default;
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual sf::FloatRect getBounds() = 0;
    void setPosition(float x, float y);
    sf::Vector2f getPosition() const;

    // Check if this object blocks movement (solid collision)
    virtual bool isSolid() const;

    // Type query methods (reduces need for dynamic_cast)
    virtual bool isLadder() const;
    virtual bool isPole() const;
    virtual bool isCoin() const;
    virtual bool isDiggable() const;

    // Default implementation - does nothing, derived classes override
    void processCollision(Collider& other) override;

protected:
    void setTexture(const sf::Texture& texture);
    std::optional<sf::Sprite> m_sprite;
    sf::Vector2f m_position;
};

