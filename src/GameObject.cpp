#include "GameObject.h"
#include "Constants.h"

void GameObject::setPosition(float x, float y)
{
    m_position = {x, y};
    if (m_sprite.has_value())
    {
        m_sprite->setPosition(m_position);
    }
}

void GameObject::setTexture(const sf::Texture& texture)
{
    m_sprite.emplace(texture);
    m_sprite->setPosition(m_position);
    
    // Scale to fit TILE_SIZE
	sf::Vector2u textureSize = texture.getSize();//get size returns Vector2u
    float scaleX = GameConstants::TILE_SIZE / static_cast<float>(textureSize.x);
    float scaleY = GameConstants::TILE_SIZE / static_cast<float>(textureSize.y);
    m_sprite->setScale({scaleX, scaleY});
}

sf::Vector2f GameObject::getPosition() const
{
    return m_position;
}

// Type queries
bool GameObject::isSolid() const
{
    return false;
}

bool GameObject::isLadder() const
{
    return false;
}

bool GameObject::isPole() const
{
    return false;
}

bool GameObject::isCoin() const
{
    return false;
}

bool GameObject::isDiggable() const
{
    return false;
}

// Default implementation - default does nothing
void GameObject::processCollision(Collider& other) 
{
    (void)other;
}
