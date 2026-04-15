#include "enemy.h"
#include <cstdlib>

Enemy::Enemy(int x, int y)
    : Character(x, y) {}

void Enemy::moveRandom(int gridSize, int map[20][20])
{
    int dir = rand() % 4;

    int newX = x;
    int newY = y;

    if (dir == 0) newY--;
    if (dir == 1) newY++;
    if (dir == 2) newX--;
    if (dir == 3) newX++;

    if (newX < 0 || newX >= gridSize || newY < 0 || newY >= gridSize)
        return;

    if (map[newX][newY] == 1)
        return;

    x = newX;
    y = newY;
}
