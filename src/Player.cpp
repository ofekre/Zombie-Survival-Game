#include "Player.h"
#include "Resourcemanager.h"
#include "Constants.h"
#include "StaticObject.h"
#include "Enemy.h"

using namespace GameConstants;

Player::Player()
{
    setTexture(ResourceManager::instance().getTexture(Textures::PLAYER));
    setSpeed(PLAYER_SPEED);
    setVelocity({0.f, 0.f});
    
    // Initialize sound effects
    m_coinSound.emplace(ResourceManager::instance().getSound("CoinCollected"));
    m_digSound.emplace(ResourceManager::instance().getSound("digging"));
    m_jumpSound.emplace(ResourceManager::instance().getSound("jump"));
    m_enemyHitSound.emplace(ResourceManager::instance().getSound("enemy hit"));
    m_fallingSound.emplace(ResourceManager::instance().getSound("falling"));
    if (m_fallingSound) m_fallingSound->setLooping(true);  // Loop while falling
}

void Player::draw(sf::RenderWindow& window)
{
    if (m_sprite.has_value())
        window.draw(*m_sprite);
}

sf::FloatRect Player::getBounds()
{
    // Use fixed hitbox based on TILE_SIZE (not sprite size)
    // This prevents hitbox changes when switching textures (standing/running/etc.)
    const float SHRINK = GameConstants::PLAYER_HITBOX_SHRINK;
    
    return sf::FloatRect(
        {m_position.x + SHRINK, m_position.y + SHRINK},
        {GameConstants::TILE_SIZE - SHRINK * 2, GameConstants::TILE_SIZE - SHRINK * 2}
    );
}

void Player::move(float deltaTime)
{
    sf::Vector2f velocity = getVelocity();

    // Apply gravity if not on ground, ladder, or pole
    if (!isOnGround() && !isOnLadder() && !isOnPole())
    {
        velocity.y += GameConstants::GRAVITY * deltaTime;

        // Limit max fall speed to prevent tunneling through floors
        if (velocity.y > GameConstants::MAX_FALL_SPEED)
        {
            velocity.y = GameConstants::MAX_FALL_SPEED;
        }

        // Play falling sound only if:
        // - Actually falling (positive velocity = going down)
        // - NOT from a player-initiated jump
        // - NOT already playing
        // - NOT in spawn immunity period
        // The sound plays when: walking off edge, falling through dug floor, dropping from pole
        if (velocity.y > FALL_SOUND_THRESHOLD && !m_isFallingSoundPlaying && !m_isJumping && m_fallSoundImmunityTimer <= 0.f)
        {
            if (m_fallingSound) m_fallingSound->play();
            m_isFallingSoundPlaying = true;
        }

        setVelocity(velocity);
    }
    else if (isOnGround() && !isOnLadder())
    {
        // On ground (but not on ladder) - stop falling
        // Don't reset velocity if on ladder - allow climbing!
        setVelocityY(0);

        // Landing - reset jump flag and stop falling sound
        m_isJumping = false;

        if (m_isFallingSoundPlaying)
        {
            if (m_fallingSound) m_fallingSound->stop();
            m_isFallingSoundPlaying = false;
        }
    }
    // If on ladder or pole, velocity.y is controlled by handleInput()
    // Stop falling sound and reset jump flag if on ladder or pole
    if ((isOnLadder() || isOnPole()) && m_isFallingSoundPlaying)
    {
        if (m_fallingSound) m_fallingSound->stop();
        m_isFallingSoundPlaying = false;
    }
    if (isOnLadder() || isOnPole())
    {
        m_isJumping = false;  // Reset jump flag when grabbing ladder/pole
    }

    velocity = getVelocity();
    m_position.x += velocity.x * deltaTime;
    m_position.y += velocity.y * deltaTime;

    updateSprite();
}

void Player::updateSprite()
{
    if (!m_sprite.has_value()) return;

    sf::Vector2f velocity = getVelocity();

    // Determine target texture based on state
    bool shouldBeLadderTexture = false;
    bool shouldBePoleTexture = false;
    bool shouldBeStandingTexture = false;

    if (isOnPole())
    {
        shouldBePoleTexture = true;
    }
    else if (isOnLadder())
    {
        if (velocity.y != 0 || (velocity.x == 0 && velocity.y == 0))
        {
            shouldBeLadderTexture = true;
        }
    }
    else if (velocity.x == 0 && isOnGround())
    {
        shouldBeStandingTexture = true;
    }

    // Remember facing direction BEFORE potentially changing texture
    float facingSign = (m_sprite->getScale().x < 0) ? -1.f : 1.f;

    // Apply texture switch only if state changes
    if (shouldBePoleTexture && !m_isPoleTextureActive)
    {
         setTexture(ResourceManager::instance().getTexture(Textures::PLAYER_POLE));
         m_isPoleTextureActive = true;
         m_isLadderTextureActive = false;
         m_isStandingTextureActive = false;
    }
    else if (shouldBeLadderTexture && !m_isLadderTextureActive)
    {
         setTexture(ResourceManager::instance().getTexture(Textures::PLAYER_LADDER));
         m_isLadderTextureActive = true;
         m_isPoleTextureActive = false;
         m_isStandingTextureActive = false;
    }
    else if (shouldBeStandingTexture && !m_isStandingTextureActive)
    {
         setTexture(ResourceManager::instance().getTexture(Textures::PLAYER_STANDING));
         m_isStandingTextureActive = true;
         m_isLadderTextureActive = false;
         m_isPoleTextureActive = false;
    }
    else if (!shouldBeLadderTexture && !shouldBePoleTexture && !shouldBeStandingTexture &&
             (m_isLadderTextureActive || m_isPoleTextureActive || m_isStandingTextureActive))
    {
         setTexture(ResourceManager::instance().getTexture(Textures::PLAYER));
         m_isLadderTextureActive = false;
         m_isPoleTextureActive = false;
         m_isStandingTextureActive = false;
    }

    // Get texture size to calculate proper scale
    TextureScale ts = getTextureScale();
    float scaleX = ts.scaleX;
    float scaleY = ts.scaleY;

    // Standing texture needs to be slightly larger to match running size
    if (m_isStandingTextureActive)
    {
        scaleX *= STANDING_SCALE_BOOST;
        scaleY *= STANDING_SCALE_BOOST;
    }

    // Flip sprite based on direction
    // Note: Pole sprite is drawn facing opposite direction, so we invert the flip
    float flipMultiplier = m_isPoleTextureActive ? -1.f : 1.f;

    if (velocity.x > 0)
    {
        m_sprite->setScale({flipMultiplier * scaleX, scaleY});
        if (flipMultiplier < 0) m_sprite->setOrigin({ts.texWidth, 0.f});
        else m_sprite->setOrigin({0.f, 0.f});
    }
    else if (velocity.x < 0)
    {
        m_sprite->setScale({-flipMultiplier * scaleX, scaleY});
        if (flipMultiplier < 0) m_sprite->setOrigin({0.f, 0.f});
        else m_sprite->setOrigin({ts.texWidth, 0.f});
    }
    else
    {
         // Preserve previous facing direction
         m_sprite->setScale({facingSign * scaleX, scaleY});
         if (facingSign < 0) m_sprite->setOrigin({ts.texWidth, 0.f});
         else m_sprite->setOrigin({0.f, 0.f});
    }
    m_sprite->setPosition(m_position);
}

void Player::handleInput()
{
    // Reset drop flag for next frame logic (Collision runs before Input)
    m_droppingFromPole = false;

    float speed = getSpeed();

    // Only reset horizontal velocity - vertical is handled by gravity/climbing
    setVelocityX(0);

    // Left/Right movement always allowed
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        setVelocityX(-speed);
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
        setVelocityX(speed);
    }

    // Dropping from pole
    if ((isOnPole() || m_isTouchingPole) && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
    {
        // Initial push down to ensure we leave the pole area
        if (getVelocity().y < speed)
        {
            setVelocityY(speed);
        }
        m_droppingFromPole = true; // Signal next frame's collision to ignore pole snapping
    }

    // UP/DOWN movement only on ladder - override gravity
    if (isOnLadder())
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        {
            setVelocityY(-speed);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        {
            setVelocityY(speed);
        }
        else
        {
            setVelocityY(0);  // Stop on ladder if no input
        }

        // Snap to ladder center X if moving vertically AND not trying to move sideways
        sf::Vector2f velocity = getVelocity();
        if (velocity.y != 0 && velocity.x == 0)
        {
            float ladderX = getLadderCenterX();
            float diff = ladderX - m_position.x;
            if (std::abs(diff) > LADDER_SNAP_THRESHOLD)
            {
                // Smoothly move towards center
                setVelocityX((diff > 0) ? speed * LADDER_SNAP_SPEED_FACTOR : -speed * LADDER_SNAP_SPEED_FACTOR);
            }
            else
            {
                // Snap exactly when close enough
                m_position.x = ladderX;
                setVelocityX(0);
            }
        }
    }

    // Jump with Space key - only when on ground and not on ladder/pole
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && isOnGround() && !isOnLadder() && !isOnPole())
    {
        setVelocityY(-GameConstants::JUMP_FORCE);
        setOnGround(false);  // No longer on ground after jumping
        m_isJumping = true;  // Mark as player-initiated jump (no fall sound)
        if (m_jumpSound) m_jumpSound->play();  // Play jump sound
    }

    // Handle digging with Z and X keys (only on key press, not hold)
    bool zPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z);
    bool xPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X);
    
    if (zPressed && !m_zKeyWasPressed)
    {
        dig(true);  // Dig left
    }
    else if (xPressed && !m_xKeyWasPressed)
    {
        dig(false);  // Dig right
    }
    
    m_zKeyWasPressed = zPressed;
    m_xKeyWasPressed = xPressed;
}

void Player::dig(bool left)
{
    // Set dig request flag (Board will handle the actual digging)
    if (left)
    {
        m_wantsDigLeft = true;
    }
    else
    {
        m_wantsDigRight = true;
    }
    // Play digging sound
    if (m_digSound) m_digSound->play();
}

// ===== Collision Handlers =====

void Player::handleCollision(Wall& wall)
{
    CollisionResult result = resolveCollisionWithSolid(wall.getBounds());
    if (result.landedOnTop)
    {
        setOnGround(true);
        setVelocityY(0);
    }
}

void Player::handleCollision(Floor& floor)
{
    CollisionResult result = resolveCollisionWithSolid(floor.getBounds());
    if (result.landedOnTop)
    {
        setOnGround(true);
        setVelocityY(0);
    }
}

void Player::handleCollision(DiggableFloor& floor)
{
    if (!floor.isDigged())
    {
        CollisionResult result = resolveCollisionWithSolid(floor.getBounds());
        if (result.landedOnTop)
        {
            setOnGround(true);
            setVelocityY(0);
        }
    }
    // If digged, player falls through
}

void Player::handleCollision(Ladder& ladder)
{
    setOnLadder(true);

    // Save ladder center X for snapping
    sf::FloatRect ladderBounds = ladder.getBounds();
    setLadderCenterX(ladderBounds.position.x);
}

void Player::handleCollision(Pole& pole)
{
    m_isTouchingPole = true; // Always track that we are touching a pole

    if (m_droppingFromPole) return;

    setOnPole(true);
    setVelocityY(0);
    
    // Snap to pole position - player hangs FROM the pole
    // The pole bar is visually at the CENTER of the pole tile
    // We want the player's hands (top of sprite, ~5-10 pixels down) to be at that bar level
    sf::FloatRect poleBounds = pole.getBounds();

    // Pole bar center = top of tile + half tile size
    // Player hands are at top of sprite, so position player so top aligns with bar
    // The wooden pole texture is at the TOP of the tile, so use top position
    const float POLE_TOP = poleBounds.position.y;
    // Use percentage of TILE_SIZE for consistency across all systems
    // Player hangs from pole - position so hands (top of sprite) are at pole level
    // ADD offset to move player DOWN so hands align with pole
    const float HAND_OFFSET = GameConstants::TILE_SIZE * GameConstants::POLE_HAND_OFFSET;
    m_position.y = POLE_TOP + HAND_OFFSET;

    syncSpritePosition();
}

void Player::handleCollision(Coin& coin)
{
    if (!coin.isCollected())    
    {
        coin.collect();
        if (m_coinSound) m_coinSound->play();  // Play coin collection sound
        // Score is added by Board::update() with level multiplier
    }
}

void Player::handleCollision(Enemy& enemy)
{
    (void)enemy;
    // Only trigger reset if not invincible
    // This prevents infinite reset loop when respawning near enemies
    if (!isInvincible())
    {
        if (m_enemyHitSound) m_enemyHitSound->play();  // Play enemy hit sound
        loseLife();
        m_needsReset = true;  // Only reset when actually damaged
    }
}

void Player::update(float deltaTime)
{
    // Count down invincibility timer
    if (m_invincibilityTimer > 0)
    {
        m_invincibilityTimer -= deltaTime;
    }
    
    // Count down fall sound immunity timer
    if (m_fallSoundImmunityTimer > 0)
    {
        m_fallSoundImmunityTimer -= deltaTime;
    }
}

void Player::loseLife()
{
    if (m_lives > 0 && !isInvincible())
    {
        --m_lives;
        m_invincibilityTimer = GameConstants::INVINCIBILITY_DURATION;
    }
}

void Player::processCollision(Collider& other)
{
    other.handleCollision(*this);
}

int Player::getLives() const
{
    return m_lives;
}

int Player::getScore() const
{
    return m_score;
}

void Player::addScore(int points)
{
    m_score += points;
}

void Player::saveStartPosition()
{
    m_startPosition = m_position;
}

void Player::resetToStart()
{
    // Reset position to start
    m_position = m_startPosition;

    // Reset common movement state
    resetMovementState();
    m_needsReset = false;  // Clear the reset flag!

    // Stop falling sound
    if (m_isFallingSoundPlaying)
    {
        if (m_fallingSound) m_fallingSound->stop();
        m_isFallingSoundPlaying = false;
    }

    // Reset texture state
    m_isLadderTextureActive = false;
    m_isPoleTextureActive = false;
    m_isStandingTextureActive = false;
    m_droppingFromPole = false;
    m_isJumping = false;
    m_fallSoundImmunityTimer = FALL_SOUND_IMMUNITY;  // Brief immunity after respawn

    // Reset to normal player texture
    setTexture(ResourceManager::instance().getTexture(Textures::PLAYER));
    syncSpritePosition();
}

void Player::stabilize()
{
    resetMovementState();
    m_needsReset = false;
    m_droppingFromPole = false;
    m_isJumping = false;
    syncSpritePosition();
}

void Player::reset()
{
    m_lives = STARTING_LIVES;
    m_score = 0;
    m_invincibilityTimer = 0;
    resetMovementState();
    m_droppingFromPole = false;
    syncSpritePosition();
}

bool Player::isInvincible() const
{
    return m_invincibilityTimer > 0;
}

bool Player::needsPositionReset() const
{
    return m_needsReset;
}

void Player::resetStateFlags()
{
    MovingObject::resetStateFlags();
    m_isTouchingPole = false;
}

bool Player::wantsToDigLeft() const
{
    return m_wantsDigLeft;
}

bool Player::wantsToDigRight() const
{
    return m_wantsDigRight;
}

void Player::clearDigFlags()
{
    m_wantsDigLeft = false;
    m_wantsDigRight = false;
}

void Player::setMuted(bool muted)
{
    float volume = muted ? 0.f : 100.f;
    if (m_coinSound) m_coinSound->setVolume(volume);
    if (m_digSound) m_digSound->setVolume(volume);
    if (m_jumpSound) m_jumpSound->setVolume(volume);
    if (m_enemyHitSound) m_enemyHitSound->setVolume(volume);
    if (m_fallingSound) m_fallingSound->setVolume(volume);
}

void Player::stopFallingSound()
{
    if (m_isFallingSoundPlaying)
    {
        if (m_fallingSound) m_fallingSound->stop();
        m_isFallingSoundPlaying = false;
    }
}
