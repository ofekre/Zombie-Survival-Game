#include "Collider.h"
#include "Player.h"
#include "Enemy.h"
#include "StaticObject.h"

// Default implementations do nothing
void Collider::handleCollision(Player& player) { (void)player; }
void Collider::handleCollision(Enemy& enemy) { (void)enemy; }
void Collider::handleCollision(Wall& wall) { (void)wall; }
void Collider::handleCollision(Floor& floor) { (void)floor; }
void Collider::handleCollision(DiggableFloor& floor) { (void)floor; }
void Collider::handleCollision(Ladder& ladder) { (void)ladder; }
void Collider::handleCollision(Pole& pole) { (void)pole; }
void Collider::handleCollision(Coin& coin) { (void)coin; }
