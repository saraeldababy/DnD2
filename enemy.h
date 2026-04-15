#ifndef ENEMY_H
#define ENEMY_H

#pragma once
#include "character.h"

class Enemy : public Character
{
public:
    Enemy(int x = 5, int y = 5);

    void moveToward(int targetX, int targetY, int gridSize, int map[20][20]);
};
#endif // ENEMY_H
