#pragma once
#include <vector>
#include <memory>
#include <string>
#include "GameObject.h"
#include "Enemy.h"

// Result of parsing a single level
struct LevelData
{
    std::vector<std::unique_ptr<GameObject>> objects;
    std::vector<Enemy> enemies;
    sf::Vector2f playerStartPos;
    float width = 0.f;
    float height = 0.f;
};

// Responsible for loading and parsing level files
class LevelLoader
{
public:
    // Load all levels from a file (levels separated by empty lines)
    bool loadFromFile(const std::string& filename);

    // Get number of loaded levels
    int getLevelCount() const;

    // Parse a specific level and return its data
    // Returns nullptr if levelIndex is invalid
    std::unique_ptr<LevelData> parseLevel(int levelIndex) const;

private:
    std::vector<std::vector<std::string>> m_levelLines;  // Raw lines for each level
};
