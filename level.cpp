#include "level.h"
#include <algorithm>

namespace {
inline bool inBounds(int v, int n) { return v >= 0 && v < n; }

inline void setTileIfValid(int **map, int n, int x, int y, int tile)
{
    if (inBounds(x, n) && inBounds(y, n))
        map[x][y] = tile;
}
} // namespace

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
    if (!map) return;
    for (int i = 0; i < gridSize; i++)
        delete[] map[i];
    delete[] map;
    map = nullptr;
    gridSize = 0;
}

Level::Level(int size)
    : gridSize(0), map(nullptr)
{
    if (size < 3)
        size = 3; // minimum sane map size

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
        setTileIfValid(map, gridSize, 2, y, TREE);
        setTileIfValid(map, gridSize, 3, y, TREE);
    }

    for (int y = 6; y <= 7; ++y)
    {
        setTileIfValid(map, gridSize, 6, y, TREE);
        setTileIfValid(map, gridSize, 7, y, TREE);
    }

    // A river crossing the level.
    if (gridSize > 5)
    {
        for (int y = 1; y < gridSize - 1; ++y)
            map[5][y] = RIVER;
    }

    // One easy crossing point.
    setTileIfValid(map, gridSize, 5, 5, BRIDGE);

    // Path route.
    for (int x = 1; x <= 8; ++x)
        setTileIfValid(map, gridSize, x, 8, PATH);

    for (int y = 5; y <= 8; ++y)
        setTileIfValid(map, gridSize, 5, y, PATH);

    setTileIfValid(map, gridSize, 5, 5, BRIDGE);

    // Cottage goal tile.
    setTileIfValid(map, gridSize, 8, 2, COTTAGE);
    setTileIfValid(map, gridSize, 8, 3, COTTAGE);
    setTileIfValid(map, gridSize, 7, 2, COTTAGE);
    setTileIfValid(map, gridSize, 7, 3, COTTAGE);
}

Level::Level(const Level &other) : gridSize(0), map(nullptr)
{
    allocateAndFill(other.gridSize);
    for (int i = 0; i < gridSize; ++i)
        for (int j = 0; j < gridSize; ++j)
            map[i][j] = other.map[i][j];
}

Level &Level::operator=(const Level &other)
{
    if (this == &other) return *this;
    clear();
    allocateAndFill(other.gridSize);
    for (int i = 0; i < gridSize; ++i)
        for (int j = 0; j < gridSize; ++j)
            map[i][j] = other.map[i][j];
    return *this;
}

Level::Level(Level &&other) noexcept : gridSize(other.gridSize), map(other.map)
{
    other.gridSize = 0; other.map = nullptr;
}

Level &Level::operator=(Level &&other) noexcept
{
    if (this == &other) return *this;
    clear();
    gridSize = other.gridSize; map = other.map;
    other.gridSize = 0; other.map = nullptr;
    return *this;
}

Level::~Level() { clear(); }

bool Level::isWalkable(int x, int y) const
{
    if (x < 0 || y < 0 || x >= gridSize || y >= gridSize) return false;
    const int t = map[x][y];
    return t != TREE && t != RIVER && t != WALL && t != LOCKED && t != FIRE;
}

bool Level::isWalkableForEnemy(int x, int y) const
{
    if (x < 0 || y < 0 || x >= gridSize || y >= gridSize) return false;
    const int t = map[x][y];
    return t != TREE && t != RIVER && t != WALL;
}
