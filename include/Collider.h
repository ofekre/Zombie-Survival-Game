#pragma once

// Forward declarations for double dispatch
class Player;
class Enemy;
class Wall;
class Floor;
class DiggableFloor;
class Ladder;
class Pole;
class Coin;

// Interface for collision handling using double dispatch pattern
class Collider
{
public:
    virtual ~Collider() = default;
    
    // Double dispatch - step 1: call processCollision on the colliding object
    virtual void processCollision(Collider& other) = 0;
    
    // Double dispatch - step 2: type-specific collision handlers
    virtual void handleCollision(Player& player);
    virtual void handleCollision(Enemy& enemy);
    virtual void handleCollision(Wall& wall);
    virtual void handleCollision(Floor& floor);
    virtual void handleCollision(DiggableFloor& floor);
    virtual void handleCollision(Ladder& ladder);
    virtual void handleCollision(Pole& pole);
    virtual void handleCollision(Coin& coin);
};
