#ifndef LEVEL_H
#define LEVEL_H

class Level
{
public:
    int gridSize;
    int **map;

    enum TileType {
        GRASS    = 0,
        TREE     = 1,
        COTTAGE  = 2,
        RIVER    = 3,
        PATH     = 4,
        BRIDGE   = 5,
        WALL     = 6,
        FLOOR    = 7,
        DOOR     = 8,
        LOCKED   = 9,
        KEY_TILE = 10,
        CHEST    = 11,
        STAIRS   = 12,
        FIRE     = 13,
        CELL     = 14,
        SWORD_TILE = 15
    };

    Level(int size = 10);
    Level(const Level &other);
    Level &operator=(const Level &other);
    Level(Level &&other) noexcept;
    Level &operator=(Level &&other) noexcept;
    ~Level();

    bool isWalkable(int x, int y) const;
    bool isWalkableForEnemy(int x, int y) const;

private:
    void allocateAndFill(int size);
    void clear();
};

#endif
