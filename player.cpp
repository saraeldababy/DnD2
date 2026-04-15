#include "player.h"

Player::Player(int x, int y)
    : Character(x, y) {}

//4PM
// Player::Player(int x, int y)
//     : Character(x, y), health(3) {}  //HEALTH(3) ADDED

void Player::move(int dx, int dy)
{
    x += dx;
    y += dy;
}

// //4PM
// int Player::getHealth() const   //HEALTH
// {
//     return health;
// }

// void Player::takeDamage(int dmg) //HEALTH
// {
//     health -= dmg;
// }
