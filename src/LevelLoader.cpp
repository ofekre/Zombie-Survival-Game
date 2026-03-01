#include "LevelLoader.h"
#include "StaticObject.h"
#include "Constants.h"
#include <fstream>
#include <cctype>

using namespace GameConstants;

bool LevelLoader::loadFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        return false;
    }

    m_levelLines.clear();

    std::string line;
    std::vector<std::string> currentLevel;

    // Read all lines and split by empty lines
    while (std::getline(file, line))
    {
        // Trim trailing whitespace
        while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back())))
        {
            line.pop_back();
        }

        // Empty line (after trimming) = level separator
        if (line.empty())
        {
            if (!currentLevel.empty())
            {
                m_levelLines.push_back(currentLevel);
                currentLevel.clear();
            }
        }
        else
        {
            currentLevel.push_back(line);
        }
    }

    // Don't forget the last level (no empty line at end)
    if (!currentLevel.empty())
    {
        m_levelLines.push_back(currentLevel);
    }

    return !m_levelLines.empty();
}

int LevelLoader::getLevelCount() const
{
    return static_cast<int>(m_levelLines.size());
}

std::unique_ptr<LevelData> LevelLoader::parseLevel(int levelIndex) const
{
    if (levelIndex < 0 || levelIndex >= static_cast<int>(m_levelLines.size()))
    {
        return nullptr;
    }

    auto levelData = std::make_unique<LevelData>();
    const auto& lines = m_levelLines[levelIndex];

    size_t maxCols = 0;
    int row = 0;

    for (const auto& line : lines)
    {
        if (line.size() > maxCols)
        {
            maxCols = line.size();
        }

        for (size_t col = 0; col < line.size(); ++col)
        {
            char c = line[col];
            float x = static_cast<float>(col) * TILE_SIZE;
            float y = static_cast<float>(row) * TILE_SIZE + HUD_HEIGHT;

            switch (c)
            {
                case Chars::WALL:
                {
                    auto wall = std::make_unique<Wall>();
                    wall->setPosition(x, y);
                    levelData->objects.push_back(std::move(wall));
                    break;
                }
                case Chars::DIGGABLE_FLOOR:
                {
                    auto floor = std::make_unique<DiggableFloor>();
                    floor->setPosition(x, y);
                    levelData->objects.push_back(std::move(floor));
                    break;
                }
                case Chars::LADDER:
                {
                    auto ladder = std::make_unique<Ladder>();
                    ladder->setPosition(x, y);
                    levelData->objects.push_back(std::move(ladder));
                    break;
                }
                case Chars::POLE:
                {
                    auto pole = std::make_unique<Pole>();
                    pole->setPosition(x, y);
                    levelData->objects.push_back(std::move(pole));
                    break;
                }
                case Chars::COIN:
                {
                    auto coin = std::make_unique<Coin>();
                    coin->setPosition(x, y);
                    levelData->objects.push_back(std::move(coin));
                    break;
                }
                case Chars::PLAYER:
                {
                    levelData->playerStartPos = {x, y};
                    break;
                }
                case Chars::ENEMY:
                {
                    Enemy enemy;
                    enemy.setPosition(x, y);
                    levelData->enemies.push_back(std::move(enemy));
                    break;
                }
            }
        }
        ++row;
    }

    // Calculate dimensions
    levelData->width = static_cast<float>(maxCols) * TILE_SIZE;
    levelData->height = static_cast<float>(row) * TILE_SIZE;

    return levelData;
}
