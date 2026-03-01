#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>

class ResourceManager
{
public:
    static ResourceManager& instance();
    sf::Texture& getTexture(const std::string& name);
    sf::SoundBuffer& getSound(const std::string& name);
    sf::Font& getFont(const std::string& name);

private:
    ResourceManager() = default;
	std::map<std::string, sf::Texture> m_textures;// Map to store loaded textures
    std::map<std::string, sf::SoundBuffer> m_sounds;
    std::map<std::string, sf::Font> m_fonts;
};