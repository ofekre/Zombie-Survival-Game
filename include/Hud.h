#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "Constants.h"

class HUD
{
public:
    void draw(sf::RenderWindow& window);
    void update(int level, int score, int lives, float time);

private:
    std::string m_levelStr;
    std::string m_scoreStr;
    std::string m_livesStr;
    std::string m_timerStr;
    int m_lives = GameConstants::STARTING_LIVES;  // Current lives for icon display
};