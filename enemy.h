#ifndef ENEMY_H
#define ENEMY_H

class Enemy
{
private:
    int x, y;

public:
    Enemy();

    int getX() const;
    int getY() const;

    void setX(int);
    void setY(int);
void moveToward(int tx, int ty);
};

#endif
