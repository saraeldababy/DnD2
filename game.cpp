#include "game.h"
#include "level.h"
#include "player.h"
#include "enemy.h"
Game::Game(int size)
{
    level = Level(size);
    player = Player();
    enemy = Enemy();
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
    enemy.moveRandom(level.gridSize, level.map);
}

bool Game::checkWin()
{
    return level.map[player.getX()][player.getY()] == 2;
}

bool Game::checkLose()
{
    return player.getX() == enemy.getX() &&
           player.getY() == enemy.getY();
}

Player& Game::getPlayer() { return player; }
Enemy& Game::getEnemy() { return enemy; }
Level& Game::getLevel() { return level; }
