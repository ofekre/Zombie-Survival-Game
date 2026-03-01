#include "Board.h"
#include "StaticObject.h"
#include "Resourcemanager.h"
#include "Constants.h"
#include <cmath>

using namespace GameConstants;

void Board::loadFromFile(const std::string& filename)
{
    if (!m_levelLoader.loadFromFile(filename))
    {
        return;
    }

    // Initialize falling to hole sound
    m_fallingToHoleSound.emplace(ResourceManager::instance().getSound("fallingtohole"));

    // Initialize board background sprite
    m_boardBackground.emplace(ResourceManager::instance().getTexture(Textures::BOARD_BACKGROUND));

    // Load the first level
    m_currentLevel = 0;
    m_player.reset();  // Reset lives and score for fresh start
    if (m_levelLoader.getLevelCount() > 0)
    {
        loadLevel(0);
    }
}

void Board::loadLevel(int levelIndex)
{
    auto levelData = m_levelLoader.parseLevel(levelIndex);
    if (!levelData)
    {
        return;
    }

    // Reset death pause state (in case we're loading after a pit fall)
    m_isDeathPaused = false;
    m_deathPauseTimer = 0.f;

    m_currentLevel = levelIndex;
    applyLevelData(std::move(levelData));
}

void Board::nextLevel()
{
    if (hasMoreLevels())
    {
        loadLevel(m_currentLevel + 1);
    }
    else
    {
        // Loop back to first level (infinite loop)
        loadLevel(0);
    }
}

void Board::reloadCurrentLevel()  // Reload level keeping score (for time up)
{
    auto levelData = m_levelLoader.parseLevel(m_currentLevel);
    if (!levelData)
    {
        return;
    }

    // Save the current score and lives before reloading
    int savedScore = m_player.getScore();
    int savedLives = m_player.getLives();

    // Reload the level (resets everything)
    applyLevelData(std::move(levelData));

    // Restore score
    while (m_player.getScore() < savedScore)
    {
        m_player.addScore(1);
    }

    // Restore lives (applyLevelData doesn't touch score/lives anymore)
    while (m_player.getLives() > savedLives)
    {
        m_player.loseLife();
    }

    // Reset player movement state (prevent falling through floor)
    m_player.resetToStart();
}

void Board::applyLevelData(std::unique_ptr<LevelData> data)
{
    // Move objects and enemies from parsed data
    m_objects = std::move(data->objects);
    m_enemies = std::move(data->enemies);
    m_width = data->width;
    m_height = data->height;

    // Set player position
    m_player.setPosition(data->playerStartPos.x, data->playerStartPos.y);
    m_player.saveStartPosition();

    // Save starting positions for enemies
    for (size_t i = 0; i < m_enemies.size(); ++i)
    {
        m_enemies[i].saveStartPosition();
        m_enemies[i].setOnGround(true);  // Stabilize enemy to prevent falling on first frame
    }

    // Reset player physics state to prevent falling on first frame
    m_player.stabilize();

    // Set fall immunity to prevent player from dying on first few frames
    m_fallImmunityTimer = 1.0f;
}

void Board::draw(sf::RenderWindow& window)
{
    // Don't draw anything during death pause
    if (m_isDeathPaused)
    {
        return;
    }

    // Draw background first (behind everything)
    // Offset by one tile up so digging bottom row reveals black below
    if (m_boardBackground)
    {
        sf::Vector2u windowSize = window.getSize();
        sf::Vector2u textureSize = m_boardBackground->getTexture().getSize();
        m_boardBackground->setScale({
            static_cast<float>(windowSize.x) / textureSize.x,
            static_cast<float>(windowSize.y - TILE_SIZE) / textureSize.y
        });
        m_boardBackground->setPosition({0.f, 0.f});
        window.draw(*m_boardBackground);
    }

	for (size_t i = 0; i < m_objects.size(); ++i)
    {
        m_objects[i]->draw(window);
    }
    
    for (size_t i = 0; i < m_enemies.size(); ++i)
    {
        if (!m_enemies[i].isRemoved())
        {
            m_enemies[i].draw(window);
        }
    }
    
    m_player.draw(window);
}

void Board::update(float deltaTime)
{
    // Skip physics update if deltaTime is 0 (first frame after load)
    if (deltaTime <= 0.f)
    {
        return;
    }

    // Handle death pause (after falling into pit)
    if (m_isDeathPaused)
    {
        m_deathPauseTimer -= deltaTime;
        if (m_deathPauseTimer <= 0.f)
        {
            // End death pause - reset positions
            m_isDeathPaused = false;
            m_player.resetToStart();
            resetAllEnemies();
            m_fallImmunityTimer = 1.0f;
        }
        return;  // Don't update anything during death pause
    }

    // Update fall immunity timer
    if (m_fallImmunityTimer > 0.f)
    {
        m_fallImmunityTimer -= deltaTime;
    }

    m_player.update(deltaTime);  // Update invincibility timer
    
    // Update diggable floor animations
    for (size_t i = 0; i < m_objects.size(); ++i)
    {
        if (m_objects[i]->isDiggable())
        {
            static_cast<DiggableFloor*>(m_objects[i].get())->update(deltaTime);
        }
    }

    // 1. Reset state flags
    m_player.resetStateFlags();

    sf::FloatRect playerBounds = m_player.getBounds();

    // CRITICAL FIX: If player just respawned (fallImmunityTimer > 0),
    // we need to ensure m_onGround is set correctly BEFORE physics runs.
    // The resetStateFlags() above cleared it, but resetToStart() expects it to stay true.
    // Solution: Always run Ground Probe early if immunity is active.
    if (m_fallImmunityTimer > 0.f)
    {
        // Force player to be grounded during immunity period
        // This prevents gravity from being applied right after respawn
        m_player.setOnGround(true);
    }

    // 2. Ladder/Pole Pre-pass: Detect if on ladder/pole BEFORE moving or wall checks
    checkLadderPoleForObject(m_player);

    // 3. Ground Probe: Check for solid ground below w/o displacing
    checkGroundForObject(m_player);
    
    // Check if player fell off the world (below board bottom)
    float bottomEdge = m_height + HUD_HEIGHT;
    float playerCenterY = playerBounds.position.y + (playerBounds.size.y / 2.f);

    // Only check fall if immunity timer has expired
    if (m_fallImmunityTimer <= 0.f && playerCenterY > bottomEdge)
    {
        // Player fell off the world - lose a life
        if (!m_player.isInvincible())
        {
            m_player.loseLife();
        }

        // Stop regular falling sound and play pit fall sound
        m_player.stopFallingSound();
        if (m_fallingToHoleSound && !m_isMuted)
        {
            m_fallingToHoleSound->play();
        }

        // Start death pause - hide board for 3 seconds
        m_isDeathPaused = true;
        m_deathPauseTimer = DEATH_PAUSE_DURATION;

        // Skip the rest of the update
        return;
    }

    // 4. Handle Input & Move
    m_player.handleInput();
    m_player.move(deltaTime);

    // 5. Collision Resolution (Post-move) - Displace player from walls/floors
    for (size_t i = 0; i < m_objects.size(); ++i)
    {
        auto& obj = m_objects[i];
        if (m_player.getBounds().findIntersection(obj->getBounds()))
        {
            obj->processCollision(m_player);
        }
        
        // Check if a coin was just collected - add score with level multiplier
        if (obj->isCoin())
        {
            Coin* coin = static_cast<Coin*>(obj.get());
            if (coin->wasJustCollected())
            {
                addCoinScore();  // 2 × level number
                coin->clearJustCollected();
            }
        }
    }
    
    // 6. Handle digging requests
    if (m_player.wantsToDigLeft() || m_player.wantsToDigRight())
    {
        sf::Vector2f playerPos = m_player.getPosition();
        
        // Use player center for more natural digging
        float playerCenterX = playerPos.x + TILE_SIZE / 2.f;
        
        // Calculate which tile column the player is in
        int playerTileX = static_cast<int>(playerCenterX / TILE_SIZE);

        // Use player's FEET position (bottom of sprite) to find the floor they're standing on
        float playerFeetY = playerPos.y + TILE_SIZE;
        int playerFeetTileY = static_cast<int>((playerFeetY - HUD_HEIGHT) / TILE_SIZE);

        int targetTileX;
        if (m_player.wantsToDigLeft())
        {
            targetTileX = playerTileX - 1;  // One tile to the left
        }
        else
        {
            targetTileX = playerTileX + 1;  // One tile to the right
        }
        // The floor the player is standing ON is at their feet level
        int targetTileY = playerFeetTileY;
        
        // Convert back to pixel coordinates
        float targetX = static_cast<float>(targetTileX) * TILE_SIZE;
        float targetY = static_cast<float>(targetTileY) * TILE_SIZE + HUD_HEIGHT;
        
        // Create target area with small margin for detection
        sf::FloatRect targetArea({targetX + 1.f, targetY + 1.f}, {TILE_SIZE - 2.f, TILE_SIZE - 2.f});
        
        for (size_t i = 0; i < m_objects.size(); ++i)
        {
            auto& obj = m_objects[i];
            if (obj->isDiggable())
            {
                DiggableFloor* dig = static_cast<DiggableFloor*>(obj.get());
                // Use getPosition() instead of getBounds() to avoid extended hitbox issues
                sf::Vector2f objPos = dig->getPosition();
                sf::FloatRect originalBounds({objPos.x, objPos.y}, {TILE_SIZE, TILE_SIZE});

                if (!dig->isDigged() && targetArea.findIntersection(originalBounds))
                {
                    dig->dig();
                    break;
                }
            }
        }
        
        m_player.clearDigFlags();
    }
    
    // Check collisions with enemies
    for (size_t i = 0; i < m_enemies.size(); ++i)
    {
        auto& enemy = m_enemies[i];
        // Skip removed enemies
        if (enemy.isRemoved())
        {
            continue;
        }
        
        // Reset enemy state flags (same as player)
        enemy.resetStateFlags();
        
        // If fall immunity is active (just loaded level), force enemies to be grounded too
        if (m_fallImmunityTimer > 0.f)
        {
            enemy.setOnGround(true);
        }
        
        sf::FloatRect enemyBounds = enemy.getBounds();
        
        // Ladder/Pole pre-pass for enemy
        checkLadderPoleForObject(enemy);
        
        // Ground probe for enemy
        checkGroundForObject(enemy);
        
        // Move enemy - pass player position
        enemy.updateEnemy(m_player.getPosition());
        enemy.move(deltaTime);
        
        // Post-move collision resolution for enemy with static objects
        for (size_t j = 0; j < m_objects.size(); ++j)
        {
            auto& obj = m_objects[j];
            if (enemy.getBounds().findIntersection(obj->getBounds()))
            {
                obj->processCollision(enemy);
            }
        }
        
        // Check if enemy fell below the board (through a dug floor at first row)
        sf::FloatRect enemyBottomBounds = enemy.getBounds();
        if (enemyBottomBounds.position.y > bottomEdge)
        {
            // Enemy fell off the world - remove permanently
            enemy.remove();
            continue;
        }
        
        // Check collision with other enemies (prevent merging)
        for (size_t j = 0; j < m_enemies.size(); ++j)
        {
            if (i == j) continue;
            
            auto& otherEnemy = m_enemies[j];
            if (otherEnemy.isRemoved()) continue;
            
            if (enemy.getBounds().findIntersection(otherEnemy.getBounds()))
            {
                enemy.processCollision(otherEnemy);
            }
        }

        // Check player-enemy collision
        if (m_player.getBounds().findIntersection(enemy.getBounds()))
        {
            enemy.processCollision(m_player);  // Double dispatch!
        }
    }
    
    // Check if player needs position reset (hit enemy)
    if (m_player.needsPositionReset())
    {
        // Keep holes open when hit by enemy (player's fault)
        // Only reset positions, not the digged floors

        m_player.resetToStart();
        resetAllEnemies();

        // Set immunity timer to prevent immediate re-fall
        m_fallImmunityTimer = 1.0f;
    }
}

bool Board::isLevelComplete() const
{
    // Level is complete when all coins are collected
    for (size_t i = 0; i < m_objects.size(); ++i)
    {
        if (m_objects[i]->isCoin())
        {
            const Coin* coin = static_cast<const Coin*>(m_objects[i].get());
            if (!coin->isCollected())
            {
                return false;  // Still have uncollected coins
            }
        }
    }
    return true;  // All coins collected!
}

float Board::getLevelTimeLimit() const
{
    switch (m_currentLevel)
    {
        case 0:
            return LEVEL_1_TIME;
        case 1:
            return LEVEL_2_TIME;
        case 2:
            return LEVEL_3_TIME;
        default: return DEFAULT_LEVEL_TIME;
    }
}

int Board::getPlayerScore() const { return m_player.getScore(); }
int Board::getPlayerLives() const { return m_player.getLives(); }
void Board::losePlayerLife() { m_player.loseLife(); }

void Board::addCoinScore() { m_player.addScore(COIN_SCORE_MULTIPLIER * getLevelNumber()); }
void Board::addLevelCompleteScore() { m_player.addScore(LEVEL_COMPLETE_MULTIPLIER * getLevelNumber()); }

int Board::getLevelNumber() const { return m_currentLevel + 1; }
bool Board::hasMoreLevels() const { return m_currentLevel + 1 < m_levelLoader.getLevelCount(); }

float Board::getWidth() const { return m_width; }
float Board::getHeight() const { return m_height; }

void Board::setMuted(bool muted)
{
    m_isMuted = muted;
    m_player.setMuted(muted);
    if (m_fallingToHoleSound)
    {
        m_fallingToHoleSound->setVolume(muted ? 0.f : 100.f);
    }
}

void Board::stopPlayerFallingSound()
{
    m_player.stopFallingSound();
}

void Board::checkGroundForObject(MovingObject& movingObj)
{
    sf::FloatRect bounds = movingObj.getBounds();
    sf::FloatRect groundProbe = bounds;
    
    // Make probe narrower - only check center 60% of object width
    float probeMargin = bounds.size.x * 0.2f;
    groundProbe.position.x += probeMargin;
    groundProbe.size.x -= probeMargin * 2;
    groundProbe.position.y += 2.f;  // Shift probe down
    
    for (size_t i = 0; i < m_objects.size(); ++i)
    {
        auto& gameObj = m_objects[i];
        if (isSolidObject(gameObj.get()) && groundProbe.findIntersection(gameObj->getBounds()))
        {
            movingObj.setOnGround(true);
            return;  // Found ground, no need to check more
        }
    }
}

void Board::checkLadderPoleForObject(MovingObject& movingObj)
{
    sf::FloatRect bounds = movingObj.getBounds();

    for (size_t i = 0; i < m_objects.size(); ++i)
    {
        auto& obj = m_objects[i];
        if (obj->isLadder() || obj->isPole())
        {
            if (bounds.findIntersection(obj->getBounds()))
            {
                obj->processCollision(movingObj);
            }
        }
    }
}

void Board::resetAllEnemies()
{
    for (size_t i = 0; i < m_enemies.size(); ++i)
    {
        m_enemies[i].resetToStart();
    }
}

bool Board::isSolidObject(GameObject* obj) const
{
    return obj->isSolid();
}

