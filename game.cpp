#include "game.h"
#include "level.h"
#include "player.h"
#include "enemy.h"

//4PM LIB FOR RAND()IN DICE
//#include <cstdlib>

Game::Game(int size)
{
    level = Level(size);
    player = Player();
    enemy = Enemy();

    //4PM
    //inCombat = false;
}

void Game::movePlayer(int dx, int dy)
{
    int newX = player.getX() + dx;
    int newY = player.getY() + dy;

    // boundary check
    if (newX < 0 || newX >= level.gridSize ||
        newY < 0 || newY >= level.gridSize)
        return;

    // wall check
    if (level.map[newX][newY] == 1)
        return;

    player.move(dx, dy);
}

void Game::updateEnemy()
{
    // enemy.moveRandom(level.gridSize, level.map);
    enemy.moveToward(
        player.getX(),
        player.getY(),
        level.gridSize,
        level.map
        );

    //4PM
    // combat happens if same tile
    // if (player.getX() == enemy.getX() &&
    //     player.getY() == enemy.getY())
    // {
    //     int playerRoll = rollDice();
    //     int enemyRoll = rollDice();

    //     if (enemyRoll > playerRoll)
    //     {
    //         player.takeDamage(1);
    //     }
    // }
}
//4PM
//SUPPOSED TO RESET BUT NOT WORKING
// void Game::updateEnemy()
// {
//     // enemy.moveToward(player.getX(), player.getY());
//     Level &level = getLevel();

//     enemy.moveToward(
//         player.getX(),
//         player.getY(),
//         level.gridSize,
//         level.map
//         );
//     if (player.getX() == enemy.getX() &&
//         player.getY() == enemy.getY())
//     {
//         if (!inCombat) // ONLY trigger once
//         {
//             int playerRoll = rollDice();
//             int enemyRoll = rollDice();

//             if (enemyRoll > playerRoll)
//             {
//                 player.takeDamage(1);
//             }

//             inCombat = true;
//         }
//     }
//     else
//     {
//         inCombat = false; // reset when separated
//     }
// }

//4PM
// void Game::resolveCombat()
// {
//     if (player.getX() == enemy.getX() &&
//         player.getY() == enemy.getY())
//     {
//         lastPlayerRoll = rollDice();
//         lastEnemyRoll = rollDice();

//         if (lastEnemyRoll > lastPlayerRoll)
//         {
//             player.takeDamage(1);
//         }

//         // IMPORTANT: push player back
//         // player.setPosition(player.getX() - 1, player.getY());
//         int backX = player.getX();
//         int backY = player.getY();

//         // simple opposite push (random direction fallback)
//         if (enemy.getX() > player.getX()) backX--;
//         else if (enemy.getX() < player.getX()) backX++;

//         if (enemy.getY() > player.getY()) backY--;
//         else if (enemy.getY() < player.getY()) backY++;

//         player.setPosition(backX, backY);
//         if (backX >= 0 && backX < level.gridSize &&
//             backY >= 0 && backY < level.gridSize &&
//             level.map[backX][backY] != 1)
//         {
//             player.setPosition(backX, backY);
//         }
//     }
// }

//4PM
// int Game::rollDice()            //DICE
// {
//     return rand() % 6 + 1;
// }

bool Game::checkWin()
{
    return level.map[player.getX()][player.getY()] == 2;
}

bool Game::checkLose()
{
    return player.getX() == enemy.getX() &&
           player.getY() == enemy.getY();
}

//4PM
// bool Game::checkLose()
// {
//     return player.getHealth() <= 0;
// }

Player& Game::getPlayer() { return player; }
Enemy& Game::getEnemy() { return enemy; }
Level& Game::getLevel() { return level; }

//4PM
// int Game::getLastPlayerRoll() const { return lastPlayerRoll; }
// int Game::getLastEnemyRoll() const { return lastEnemyRoll; }
