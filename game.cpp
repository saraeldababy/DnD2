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
    // Smarter chase: one-axis step per turn with fallback if blocked.
    const int px = player.getX();
    const int py = player.getY();

    int ex = enemy.getX();
    int ey = enemy.getY();

    const int dx = px - ex;
    const int dy = py - ey;
    const int stepX = (dx > 0) ? 1 : ((dx < 0) ? -1 : 0);
    const int stepY = (dy > 0) ? 1 : ((dy < 0) ? -1 : 0);

    auto tryStep = [&](int nx, int ny) -> bool {
        if (!level.isWalkable(nx, ny))
            return false;

        enemy.setX(nx);
        enemy.setY(ny);
        return true;
    };

    const bool preferX = std::abs(dx) >= std::abs(dy);

    if (preferX)
    {
        if (stepX != 0 && tryStep(ex + stepX, ey))
            return;
        if (stepY != 0)
            tryStep(ex, ey + stepY);
    }
    else
    {
        if (stepY != 0 && tryStep(ex, ey + stepY))
            return;
        if (stepX != 0)
            tryStep(ex + stepX, ey);
    }
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
