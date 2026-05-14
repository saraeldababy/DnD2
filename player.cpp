#include "player.h"

Player::Player()
    : Character(1, 8, 100),
    name("Adventurer"), role("Wizard"),
    attackPower(20), score(0),
    spellCharges(3), hasShield(false),
    hasStealth(false), hasBless(false),
    potions(2), keys(0)
{}

void Player::move(int dx, int dy) { x += dx; y += dy; }
void Player::setX(int v) { x = v; }
void Player::setY(int v) { y = v; }

QString Player::getName() const { return name; }
QString Player::getRole() const { return role; }
void Player::setName(const QString &n) { name = n; }
void Player::setRole(const QString &r)
{
    role = r;
    // Set class-specific starting stats
    if (r == "Wizard")    { attackPower = 25; spellCharges = 5; }
    else if (r == "Fighter") { attackPower = 30; hasShield = true; }
    else if (r == "Rogue")   { attackPower = 20; hasStealth = true; }
    else if (r == "Cleric")  { attackPower = 15; hasBless = true; potions = 4; }
}

int Player::getAttackPower() const { return attackPower; }
void Player::setAttackPower(int v) { attackPower = v; }

int Player::getSpellCharges() const { return spellCharges; }
void Player::useSpellCharge() { if (spellCharges > 0) spellCharges--; }
bool Player::getHasShield() const { return hasShield; }
void Player::setHasShield(bool v) { hasShield = v; }
bool Player::getHasStealth() const { return hasStealth; }
void Player::setHasStealth(bool v) { hasStealth = v; }
bool Player::getHasBless() const { return hasBless; }
void Player::setHasBless(bool v) { hasBless = v; }

int Player::getPotions() const { return potions; }
void Player::addPotion() { potions++; }
bool Player::usePotion()
{
    if (potions <= 0) return false;
    potions--;
    heal(40);
    return true;
}

int Player::getKeys() const { return keys; }
void Player::addKey() { keys++; }
bool Player::useKey()
{
    if (keys <= 0) return false;
    keys--;
    return true;
}

int Player::getScore() const { return score; }
void Player::addScore(int v) { score += v; }
void Player::setScore(int v) { score = v; }

QStringList Player::getInventory() const { return inventory; }
void Player::addItem(const QString &item) { if (!inventory.contains(item)) inventory.append(item); }
bool Player::hasItem(const QString &item) const { return inventory.contains(item); }

void Player::resetForLevel(int startX, int startY)
{
    x = startX;
    y = startY;
    // Heal slightly between levels but don't full restore (challenge!)
    health = std::min(maxHealth, health + 30);
    keys = 0;
}
