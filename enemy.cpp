#include "enemy.h"

Enemy::Enemy()
{
    x = 7;
    y = 7;
}

int Enemy::getX() const { return x; }
int Enemy::getY() const { return y; }

void Enemy::setX(int v) { x = v; }
void Enemy::setY(int v) { y = v; }
void Enemy::moveToward(int tx, int ty)
{
    if (x < tx) x++;
    else if (x > tx) x--;

    if (y < ty) y++;
    else if (y > ty) y--;
}
