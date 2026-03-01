#include "Resourcemanager.h"
#include <stdexcept>

ResourceManager& ResourceManager::instance()
{
	static ResourceManager instance;//create a single instance
    return instance;
}

sf::Texture& ResourceManager::getTexture(const std::string& name)
{
    auto it = m_textures.find(name);
    if (it != m_textures.end())
    {
		return it->second;// Return the cached texture if it exists
    }

    sf::Texture texture;
    // name can include subfolder path like "HUD/timer" or "player/player"
    std::string path = "resources/textures/" + name + ".png";
    
    if (!texture.loadFromFile(path))
    {
        throw std::runtime_error("Failed to load texture: " + path);
    }

	m_textures[name] = std::move(texture);//move the texture into the map
    return m_textures[name];
}

sf::SoundBuffer& ResourceManager::getSound(const std::string& name)
{
    auto it = m_sounds.find(name);
    if (it != m_sounds.end())
    {
        return it->second;
    }

    sf::SoundBuffer buffer;
    std::string path = "resources/sounds/" + name + ".wav";
    
    if (!buffer.loadFromFile(path))
    {
        throw std::runtime_error("Failed to load sound: " + path);
    }

    m_sounds[name] = std::move(buffer);
    return m_sounds[name];
}

sf::Font& ResourceManager::getFont(const std::string& name)
{
    auto it = m_fonts.find(name);
    if (it != m_fonts.end())
    {
        return it->second;
    }

    sf::Font font;
    std::string path = "resources/fonts/" + name + ".ttf";
    
    if (!font.openFromFile(path))
    {
        throw std::runtime_error("Failed to load font: " + path);
    }

    m_fonts[name] = std::move(font);
    return m_fonts[name];
}
