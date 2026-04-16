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
        RIVER = 3
    };

    Level(int size = 10);
    ~Level();
};

#endif
