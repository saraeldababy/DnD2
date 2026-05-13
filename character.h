#ifndef CHARACTER_H
#define CHARACTER_H

class Character
{
protected:
    int x, y;
    int health;
    int maxHealth;

public:
    Character(int startX = 0, int startY = 0, int hp = 100);

    virtual ~Character() = default;

    int getX() const;
    int getY() const;
    int getHealth() const;
    int getMaxHealth() const;
    bool isAlive() const;

    void setPosition(int newX, int newY);
    void takeDamage(int amount);
    void heal(int amount);
};

#endif
