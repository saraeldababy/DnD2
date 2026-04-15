#ifndef GAME_H
#define GAME_H

#pragma once

#include "player.h"
#include "enemy.h"
#include "level.h"

class Game
{
private:
    Player player;
    Enemy enemy;
    Level level;

public:
    Game(int size = 10);

    void movePlayer(int dx, int dy);
    void updateEnemy();

    bool checkWin();
    bool checkLose();

    Level& getLevel();
    Player& getPlayer();
    Enemy& getEnemy();
};

#endif // GAME_H
