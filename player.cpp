#include "player.h"

Player::Player(int x, int y)
    : Character(x, y) {}

void Player::move(int dx, int dy)
{
    x += dx;
    y += dy;
}
