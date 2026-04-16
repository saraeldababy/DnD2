#include "level.h"

void Level::allocateAndFill(int size)
{
    gridSize = size;
    map = new int *[gridSize];

    for (int i = 0; i < gridSize; i++)
    {
        map[i] = new int[gridSize];
        for (int j = 0; j < gridSize; j++)
            map[i][j] = GRASS;
    }
}

void Level::clear()
{
    if (!map)
        return;

    for (int i = 0; i < gridSize; i++)
        delete[] map[i];

    delete[] map;
    map = nullptr;
    gridSize = 0;
}

Level::Level(int size)
    : gridSize(0), map(nullptr)
{
    allocateAndFill(size);

    // Tree border for a forest feel.
    for (int i = 0; i < gridSize; ++i)
    {
        map[i][0] = TREE;
        map[i][gridSize - 1] = TREE;
        map[0][i] = TREE;
        map[gridSize - 1][i] = TREE;
    }

    // Dense forest patches.
    for (int y = 2; y <= 4; ++y)
    {
        map[2][y] = TREE;
        map[3][y] = TREE;
    }

    for (int y = 6; y <= 7; ++y)
    {
        map[6][y] = TREE;
        map[7][y] = TREE;
    }

    // A river crossing the level.
    for (int y = 1; y < gridSize - 1; ++y)
    {
        map[5][y] = RIVER;
    }

    // One easy crossing point.
    map[5][5] = BRIDGE;

    // Story path from start to cottage.
    for (int x = 1; x <= 8; ++x)
    {
        map[x][8] = PATH;
    }
    for (int y = 5; y <= 8; ++y)
    {
        map[5][y] = PATH;
    }
    map[5][5] = BRIDGE;

    // Cottage goal tile.
    map[8][2] = COTTAGE;
    map[8][3] = COTTAGE;
    map[7][2] = COTTAGE;
    map[7][3] = COTTAGE;
}

Level::Level(const Level &other)
    : gridSize(0), map(nullptr)
{
    allocateAndFill(other.gridSize);

    for (int i = 0; i < gridSize; ++i)
        for (int j = 0; j < gridSize; ++j)
            map[i][j] = other.map[i][j];
}

Level &Level::operator=(const Level &other)
{
    if (this == &other)
        return *this;

    clear();
    allocateAndFill(other.gridSize);

    for (int i = 0; i < gridSize; ++i)
        for (int j = 0; j < gridSize; ++j)
            map[i][j] = other.map[i][j];

    return *this;
}

Level::Level(Level &&other) noexcept
    : gridSize(other.gridSize), map(other.map)
{
    other.gridSize = 0;
    other.map = nullptr;
}

Level &Level::operator=(Level &&other) noexcept
{
    if (this == &other)
        return *this;

    clear();
    gridSize = other.gridSize;
    map = other.map;

    other.gridSize = 0;
    other.map = nullptr;
    return *this;
}

Level::~Level()
{
    clear();
}

bool Level::isWalkable(int x, int y) const
{
    if (x < 0 || y < 0 || x >= gridSize || y >= gridSize)
        return false;

    const int tile = map[x][y];
    return tile != RIVER;
}
