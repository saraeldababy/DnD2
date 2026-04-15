#include "level.h"

Level::Level(int size)
{
    gridSize = size;
    setup();
}

void Level::setup()
{
    for (int i = 0; i < gridSize; i++)
        for (int j = 0; j < gridSize; j++)
            map[i][j] = 0;

    // walls
    map[2][2] = 1;
    map[2][3] = 1;
    map[2][4] = 1;

    // exit
    map[gridSize - 1][gridSize - 1] = 2;
}
