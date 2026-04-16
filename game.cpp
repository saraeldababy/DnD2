#include "game.h"

Game::Game(int size)
{
    level = Level(size);
    player = Player();
    enemy = Enemy();
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
    // Make level 1 forgiving: enemy only moves every other player turn.
    if (turns % 2 != 0)
        return;

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
