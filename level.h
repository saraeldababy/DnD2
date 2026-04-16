// #ifndef LEVEL_H
// #define LEVEL_H

// class Level
// {
// public:
//     int gridSize;
//     int map[20][20];

//     Level(int size = 10);
//     void setup();
// };

// #endif


//CODEX

#ifndef LEVEL_H
#define LEVEL_H

class Level
{
public:
    int gridSize;
    int **map;

    enum TileType {
        GRASS = 0,
        TREE = 1,
        COTTAGE = 2,
        RIVER = 3,
        PATH = 4,
        BRIDGE = 5
    };

    Level(int size = 10);
    Level(const Level &other);
    Level &operator=(const Level &other);
    Level(Level &&other) noexcept;
    Level &operator=(Level &&other) noexcept;
    ~Level();

    bool isWalkable(int x, int y) const;


private:
    void allocateAndFill(int size);
    void clear();
};

#endif
