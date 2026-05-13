#ifndef PLAYER_H
#define PLAYER_H

#include "character.h"
#include <QString>
#include <QStringList>

class Player : public Character
{
private:
    QString name;
    QString role;
    int attackPower;
    int score;
    QStringList inventory;
    int spellCharges; // for wizard
    bool hasShield;   // for fighter
    bool hasStealth;  // for rogue
    bool hasBless;    // for cleric
    int potions;
    int keys;

public:
    Player();

    // Movement
    void move(int dx, int dy);
    void setX(int v);
    void setY(int v);

    // Identity
    QString getName() const;
    QString getRole() const;
    void setName(const QString &n);
    void setRole(const QString &r);

    // Combat
    int getAttackPower() const;
    void setAttackPower(int v);

    // Special abilities per class
    int getSpellCharges() const;
    void useSpellCharge();
    bool getHasShield() const;
    void setHasShield(bool v);
    bool getHasStealth() const;
    void setHasStealth(bool v);
    bool getHasBless() const;
    void setHasBless(bool v);

    // Items
    int getPotions() const;
    void addPotion();
    bool usePotion();
    int getKeys() const;
    void addKey();
    bool useKey();

    // Score
    int getScore() const;
    void addScore(int v);

    // Inventory
    QStringList getInventory() const;
    void addItem(const QString &item);
    bool hasItem(const QString &item) const;

    // Reset for new level (keep health, role, name)
    void resetForLevel(int startX, int startY);
};

#endif
