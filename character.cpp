#include "character.h"
#include <algorithm>

Character::Character(int startX, int startY, int hp)
    : x(startX), y(startY), health(hp), maxHealth(hp) {}

int Character::getX() const { return x; }
int Character::getY() const { return y; }
int Character::getHealth() const { return health; }
int Character::getMaxHealth() const { return maxHealth; }
bool Character::isAlive() const { return health > 0; }

void Character::setPosition(int newX, int newY) { x = newX; y = newY; }

void Character::takeDamage(int amount)
{
    health -= amount;

    if (health < 0)
        health = 0;
}

void Character::heal(int amount)
{
    health += amount;

    if (health > maxHealth)
        health = maxHealth;
}
