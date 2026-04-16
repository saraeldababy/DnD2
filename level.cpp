#include "level.h"

Level::Level(int size)
{
    gridSize = size;

    map = new int*[gridSize];

    for (int i = 0; i < gridSize; i++)
    {
        map[i] = new int[gridSize];
        for (int j = 0; j < gridSize; j++)
            map[i][j] = GRASS;
    }

    // forest
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < gridSize; j++)
            map[i][j] = TREE;

    // river
    for (int j = 2; j < gridSize - 2; j++)
        map[5][j] = RIVER;

    // cottage (goal)
    map[8][8] = COTTAGE;
}

Level::~Level()
{
    for (int i = 0; i < gridSize; i++)
        delete[] map[i];

    delete[] map;
}
