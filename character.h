#ifndef CHARACTER_H
#define CHARACTER_H


class Character
{
protected:
    int x, y;

public:
    Character(int startX = 0, int startY = 0);

    int getX() const;
    int getY() const;

    void setPosition(int newX, int newY);
};

#endif // CHARACTER_H
