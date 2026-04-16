// #include "game.h"
// #include "level.h"
// #include "player.h"
// #include "enemy.h"

// //4PM LIB FOR RAND()IN DICE
// //#include <cstdlib>

// Game::Game(int size)
// {
//     level = Level(size);
//     player = Player();
//     enemy = Enemy();

//     //4PM
//     //inCombat = false;
// }

// void Game::movePlayer(int dx, int dy)
// {
//     int newX = player.getX() + dx;
//     int newY = player.getY() + dy;

//     // boundary check
//     if (newX < 0 || newX >= level.gridSize ||
//         newY < 0 || newY >= level.gridSize)
//         return;

//     // wall check
//     if (level.map[newX][newY] == 1)
//         return;

//     player.move(dx, dy);
// }

// void Game::updateEnemy()
// {
//     // enemy.moveRandom(level.gridSize, level.map);
//     enemy.moveToward(
//         player.getX(),
//         player.getY(),
//         level.gridSize,
//         level.map
//         );

//     //4PM
//     // combat happens if same tile
//     // if (player.getX() == enemy.getX() &&
//     //     player.getY() == enemy.getY())
//     // {
//     //     int playerRoll = rollDice();
//     //     int enemyRoll = rollDice();

//     //     if (enemyRoll > playerRoll)
//     //     {
//     //         player.takeDamage(1);
//     //     }
//     // }
// }
// //4PM
// //SUPPOSED TO RESET BUT NOT WORKING
// // void Game::updateEnemy()
// // {
// //     // enemy.moveToward(player.getX(), player.getY());
// //     Level &level = getLevel();

// //     enemy.moveToward(
// //         player.getX(),
// //         player.getY(),
// //         level.gridSize,
// //         level.map
// //         );
// //     if (player.getX() == enemy.getX() &&
// //         player.getY() == enemy.getY())
// //     {
// //         if (!inCombat) // ONLY trigger once
// //         {
// //             int playerRoll = rollDice();
// //             int enemyRoll = rollDice();

// //             if (enemyRoll > playerRoll)
// //             {
// //                 player.takeDamage(1);
// //             }

// //             inCombat = true;
// //         }
// //     }
// //     else
// //     {
// //         inCombat = false; // reset when separated
// //     }
// // }

// //4PM
// // void Game::resolveCombat()
// // {
// //     if (player.getX() == enemy.getX() &&
// //         player.getY() == enemy.getY())
// //     {
// //         lastPlayerRoll = rollDice();
// //         lastEnemyRoll = rollDice();

// //         if (lastEnemyRoll > lastPlayerRoll)
// //         {
// //             player.takeDamage(1);
// //         }

// //         // IMPORTANT: push player back
// //         // player.setPosition(player.getX() - 1, player.getY());
// //         int backX = player.getX();
// //         int backY = player.getY();

// //         // simple opposite push (random direction fallback)
// //         if (enemy.getX() > player.getX()) backX--;
// //         else if (enemy.getX() < player.getX()) backX++;

// //         if (enemy.getY() > player.getY()) backY--;
// //         else if (enemy.getY() < player.getY()) backY++;

// //         player.setPosition(backX, backY);
// //         if (backX >= 0 && backX < level.gridSize &&
// //             backY >= 0 && backY < level.gridSize &&
// //             level.map[backX][backY] != 1)
// //         {
// //             player.setPosition(backX, backY);
// //         }
// //     }
// // }

// //4PM
// // int Game::rollDice()            //DICE
// // {
// //     return rand() % 6 + 1;
// // }

// bool Game::checkWin()
// {
//     return level.map[player.getX()][player.getY()] == 2;
// }

// bool Game::checkLose()
// {
//     return player.getX() == enemy.getX() &&
//            player.getY() == enemy.getY();
// }

// //4PM
// // bool Game::checkLose()
// // {
// //     return player.getHealth() <= 0;
// // }

// Player& Game::getPlayer() { return player; }
// Enemy& Game::getEnemy() { return enemy; }
// Level& Game::getLevel() { return level; }

// //4PM
// // int Game::getLastPlayerRoll() const { return lastPlayerRoll; }
// // int Game::getLastEnemyRoll() const { return lastEnemyRoll; }


//CODEX
#include "game.h"
#include <cstdlib>

Game::Game(int size)
    : player(), enemy(), level(size)
{
}

void Game::movePlayer(int dx, int dy)
{
    if (dx == 0 && dy == 0)
        return;

    const int nextX = player.getX() + dx;
    const int nextY = player.getY() + dy;

    if (!level.isWalkable(nextX, nextY))
        return;

    player.move(dx, dy);
    ++turns;

    const int tile = level.map[player.getX()][player.getY()];
    if (tile == Level::BRIDGE && storyState < 1)
    {
        storyState = 1;
    }
    else if (tile == Level::PATH && player.getX() >= 6 && storyState < 2)
    {
        storyState = 2;
    }
}

void Game::updateEnemy()
{
   // // Make level 1 forgiving: enemy only moves every other player turn.
   //  if (turns % 2 != 0)
   //      return;

   //  const int px = player.getX();
   //  const int py = player.getY();

   //  int ex = enemy.getX();
   //  int ey = enemy.getY();

   //  const int stepX = (px > ex) ? 1 : ((px < ex) ? -1 : 0);
   //  const int stepY = (py > ey) ? 1 : ((py < ey) ? -1 : 0);

    // Harder pacing: enemy advances every player turn.
    const int px = player.getX();
    const int py = player.getY();

    int ex = enemy.getX();
    int ey = enemy.getY();

    const int stepX = (px > ex) ? 1 : ((px < ex) ? -1 : 0);
    const int stepY = (py > ey) ? 1 : ((py < ey) ? -1 : 0);

    if (stepX != 0 && level.isWalkable(ex + stepX, ey))
    {
        ex += stepX;
    }
    else if (stepY != 0 && level.isWalkable(ex, ey + stepY))
    {
        ey += stepY;
    }

    enemy.setX(ex);
    enemy.setY(ey);


   // // Smarter chase: one-axis step per turn with fallback if blocked.
   // const int px = player.getX();
   // const int py = player.getY();

   // int ex = enemy.getX();
   // int ey = enemy.getY();

   // const int dx = px - ex;
   // const int dy = py - ey;
   // const int stepX = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);
   // const int stepY = (dy > 0) ? 1 : ((dy < 0) ? -1 : 0);

   // auto tryStep = [&](int nx, int ny) -> bool {
   //     if (!level.isWalkable(nx, ny))
   //         return false;

   //     enemy.setX(nx);
   //     enemy.setY(ny);
   //     return true;
   // };

   // const bool preferX = std::abs(dx) >= std::abs(dy);

   // if (preferX)
   // {
   //     if (stepX != 0 && tryStep(ex + stepX, ey))
   //         return;
   //     if (stepY != 0)
   //         tryStep(ex, ey + stepY);
   // }
   // else
   // {
   //     if (stepY != 0 && tryStep(ex, ey + stepY))
   //         return;
   //     if (stepX != 0)
   //         tryStep(ex + stepX, ey);
   // }
}

bool Game::checkWin()
{
    int tile = level.map[player.getX()][player.getY()];
    return tile == Level::COTTAGE;
}

bool Game::checkLose()
{
    return player.getX() == enemy.getX() &&
           player.getY() == enemy.getY();
}

Player &Game::getPlayer() { return player; }
Enemy &Game::getEnemy() { return enemy; }
Level &Game::getLevel() { return level; }

void Game::advanceStory()
{
    storyState++;
}

int Game::getStoryState() const
{
    return storyState;
}

QString Game::storyHint() const
{
    if (storyState == 0)
        return "Find the bridge and follow the lantern path to the cottage.";

    if (storyState == 1)
        return "You crossed the old bridge. The cottage lights are close.";

    return "Final stretch: reach the cottage door before the shadow catches you.";
}
