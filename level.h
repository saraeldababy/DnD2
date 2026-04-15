#ifndef LEVEL_H
#define LEVEL_H

class Level
{
public:
    int gridSize;
    int map[20][20];

    Level(int size = 10);
    void setup();
};

#endif
