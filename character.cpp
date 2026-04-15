#include "character.h"

Character::Character(int startX, int startY)
    : x(startX), y(startY) {}

int Character::getX() const { return x; }
int Character::getY() const { return y; }

void Character::setPosition(int newX, int newY)
{
    x = newX;
    y = newY;
}
