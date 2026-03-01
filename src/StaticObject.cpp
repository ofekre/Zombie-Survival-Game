#include "StaticObject.h"
#include "Resourcemanager.h"
#include "Constants.h"

using namespace GameConstants;

// ===== Wall =====
Wall::Wall()
{
    setTexture(ResourceManager::instance().getTexture(Textures::WALL));
}

void Wall::draw(sf::RenderWindow& window)
{
    if (m_sprite.has_value())
    {
        window.draw(*m_sprite);
    }
}

sf::FloatRect Wall::getBounds()
{
    if (m_sprite.has_value())
    {
        return m_sprite->getGlobalBounds();
    }
    return sf::FloatRect({m_position.x, m_position.y}, {TILE_SIZE, TILE_SIZE});
}

// ===== BaseFloor (abstract) =====
void BaseFloor::draw(sf::RenderWindow& window)
{
    if (m_sprite.has_value())
    {
        window.draw(*m_sprite);
    }
}

sf::FloatRect BaseFloor::getBounds()
{
    if (m_sprite.has_value())
    {
        return m_sprite->getGlobalBounds();
    }
    return sf::FloatRect({m_position.x, m_position.y}, {TILE_SIZE, TILE_SIZE});
}

// ===== Floor =====
Floor::Floor()
{
    setTexture(ResourceManager::instance().getTexture(Textures::GROUND));
}

sf::FloatRect Floor::getBounds()
{
    // Extend the hitbox horizontally to prevent enemies/players from falling between adjacent floor tiles
    // This adds a small overlap on both sides so adjacent floors create a continuous collision surface
    constexpr float HORIZONTAL_EXTENSION = 2.0f;  // Extend 2 pixels on each side
    
    sf::FloatRect baseBounds;
    if (m_sprite.has_value())
    {
        baseBounds = m_sprite->getGlobalBounds();
    }
    else
    {
        baseBounds = sf::FloatRect({m_position.x, m_position.y}, {TILE_SIZE, TILE_SIZE});
    }
    
    // Return extended bounds: moved left by EXTENSION, width increased by 2*EXTENSION
    return sf::FloatRect(
        {baseBounds.position.x - HORIZONTAL_EXTENSION, baseBounds.position.y},
        {baseBounds.size.x + HORIZONTAL_EXTENSION * 2.0f, baseBounds.size.y}
    );
}

// ===== DiggableFloor =====
DiggableFloor::DiggableFloor()
{
    setTexture(ResourceManager::instance().getTexture(Textures::GROUND));
}

void DiggableFloor::dig()
{
    m_isDigged = true;
    m_showDigAnimation = true;
    m_digAnimTimer = DIG_ANIM_DURATION;
}

void DiggableFloor::update(float deltaTime)
{
    if (m_showDigAnimation)
    {
        m_digAnimTimer -= deltaTime;
        if (m_digAnimTimer <= 0.f)
        {
            m_showDigAnimation = false;
        }
    }
}

bool DiggableFloor::isDigged() const
{
    return m_isDigged;
}

void DiggableFloor::draw(sf::RenderWindow& window)
{
    if (!m_isDigged)
    {
        // Draw normal ground texture
        if (m_sprite.has_value())
        {
            window.draw(*m_sprite);
        }
    }
    else if (m_showDigAnimation)
    {
        // Show digging ground texture during animation
        sf::Texture& digTexture = ResourceManager::instance().getTexture(Textures::DIGGING_GROUND);
        sf::Sprite digSprite(digTexture);
        digSprite.setPosition(m_position);
        
        // Scale to tile size
        sf::Vector2u texSize = digTexture.getSize();
        digSprite.setScale({
            TILE_SIZE / static_cast<float>(texSize.x),
            TILE_SIZE / static_cast<float>(texSize.y)
        });
        
        window.draw(digSprite);
    }
    // When digged and animation finished, draw nothing (hole)
}

sf::FloatRect DiggableFloor::getBounds()
{
    // Return empty bounds when digged so collisions pass through
    if (m_isDigged)
    {
        return sf::FloatRect({0.f, 0.f}, {0.f, 0.f});
    }
    
    // Extend the hitbox horizontally to prevent enemies/players from falling between adjacent floor tiles
    constexpr float HORIZONTAL_EXTENSION = 2.0f;  // Same as Floor
    
    sf::FloatRect baseBounds;
    if (m_sprite.has_value())
    {
        baseBounds = m_sprite->getGlobalBounds();
    }
    else
    {
        baseBounds = sf::FloatRect({m_position.x, m_position.y}, {TILE_SIZE, TILE_SIZE});
    }
    
    return sf::FloatRect(
        {baseBounds.position.x - HORIZONTAL_EXTENSION, baseBounds.position.y},
        {baseBounds.size.x + HORIZONTAL_EXTENSION * 2.0f, baseBounds.size.y}
    );
}

// ===== Ladder =====
Ladder::Ladder()
{
    setTexture(ResourceManager::instance().getTexture(Textures::LADDER));
}

void Ladder::draw(sf::RenderWindow& window)
{
    if (m_sprite.has_value())
    {
        window.draw(*m_sprite);
    }
}

sf::FloatRect Ladder::getBounds()
{
    // Always return full tile bounds for physics
    return sf::FloatRect({m_position.x, m_position.y}, {TILE_SIZE, TILE_SIZE});
}

// ===== Pole =====
Pole::Pole()
{
    setTexture(ResourceManager::instance().getTexture(Textures::POLE));
}

void Pole::draw(sf::RenderWindow& window)
{
    if (m_sprite.has_value())
    {
        window.draw(*m_sprite);
    }
}

sf::FloatRect Pole::getBounds()
{
    // Always return full tile bounds for physics, regardless of sprite size
    // This ensures checking for 'm_onPole' works even if the sprite is thin
    return sf::FloatRect({m_position.x, m_position.y}, {TILE_SIZE, TILE_SIZE});
}

// ===== Coin =====
Coin::Coin()
{
    setTexture(ResourceManager::instance().getTexture(Textures::COIN));
}

void Coin::draw(sf::RenderWindow& window)
{
    if (!m_collected && m_sprite.has_value())
    {
        window.draw(*m_sprite);
    }
}

sf::FloatRect Coin::getBounds()
{
    if (m_sprite.has_value())
    {
        return m_sprite->getGlobalBounds();
    }
    return sf::FloatRect({m_position.x, m_position.y}, {TILE_SIZE, TILE_SIZE});
}

void Coin::collect()
{
    m_collected = true;
    m_justCollected = true;
}

bool Coin::isCollected() const
{
    return m_collected;
}

bool Coin::wasJustCollected() const
{
    return m_justCollected;
}

void Coin::clearJustCollected()
{
    m_justCollected = false;
}

bool Wall::isSolid() const
{
    return true;
}

bool Floor::isSolid() const
{
    return true;
}

bool DiggableFloor::isSolid() const
{
    return !m_isDigged;
}

bool DiggableFloor::isDiggable() const
{
    return true;
}

bool Ladder::isLadder() const
{
    return true;
}

bool Pole::isPole() const
{
    return true;
}

bool Coin::isCoin() const
{
    return true;
}

void Wall::processCollision(Collider& other)
{
    other.handleCollision(*this);
}

void Floor::processCollision(Collider& other)
{
    other.handleCollision(*this);
}

void DiggableFloor::processCollision(Collider& other) 
{
    other.handleCollision(*this);
}

void Ladder::processCollision(Collider& other)
{
    other.handleCollision(*this);
}

void Pole::processCollision(Collider& other)
{
    other.handleCollision(*this);
}

void Coin::processCollision(Collider& other)
{
    other.handleCollision(*this);
}
