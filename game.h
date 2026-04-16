// #ifndef GAME_H
// #define GAME_H

// #pragma once

// #include "player.h"
// #include "enemy.h"
// #include "level.h"

// class Game
// {
// private:
//     Player player;
//     Enemy enemy;
//     Level level;

//     //4PM
//     // int rollDice();     //DICE
//     // bool inCombat;
//     // int lastPlayerRoll = 0;
//     // int lastEnemyRoll = 0;

// public:
//     Game(int size = 10);

//     void movePlayer(int dx, int dy);
//     void updateEnemy();

//     //4PM
//     // void resolveCombat();

//     bool checkWin();
//     bool checkLose();

//     Level& getLevel();
//     Player& getPlayer();
//     Enemy& getEnemy();

//     //4PM
//     // int getLastPlayerRoll() const;
//     // int getLastEnemyRoll() const;
// };

// #endif // GAME_H

//CODEX
#ifndef GAME_H
#define GAME_H

#pragma once

#include <QString>

#include "enemy.h"
#include "level.h"
#include "player.h"

class Game
{
private:
    Player player;
    Enemy enemy;
    Level level;
    int storyState = 0;
    int turns = 0;

public:
    Game(int size = 10);

    int getStoryState() const;
    QString storyHint() const;

    void movePlayer(int dx, int dy);
    void updateEnemy();
    void advanceStory();

    bool checkWin();
    bool checkLose();

    Level &getLevel();
    Player &getPlayer();
    Enemy &getEnemy();
};

#endif // GAME_H
