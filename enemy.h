// #ifndef ENEMY_H
// #define ENEMY_H

// #pragma once
// #include "character.h"

// class Enemy : public Character
// {
// public:
//     Enemy(int x = 5, int y = 5);

//     void moveToward(int targetX, int targetY, int gridSize, int map[20][20]);
// };
// #endif // ENEMY_H

//CODEX
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
