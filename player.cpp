#include "player.h"

Player::Player()
{
    x = 1;
    y = 1;
    health = 100;
    role = "Wizard";
}

int Player::getX() const { return x; }
int Player::getY() const { return y; }

void Player::setX(int v) { x = v; }
void Player::setY(int v) { y = v; }
void Player::move(int dx, int dy)
{
    x += dx;
    y += dy;
}
QString Player::getRole() const { return role; }
void Player::setRole(const QString &r) { role = r; }
