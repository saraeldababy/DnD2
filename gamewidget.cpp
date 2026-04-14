#include "gamewidget.h"
#include <QPainter>
#include <QKeyEvent>



GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    tileSize = 40;

    resetGame(10);
}

void GameWidget::resetGame(int size)
{
    gridSize = size;
    setFixedSize(gridSize * tileSize, gridSize * tileSize);

    playerX = 0;
    playerY = 0;

    setupMap();

    update();
}

void GameWidget::setupMap()
{
    // clear everything
    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            map[i][j] = 0;
        }
    }

    // walls
    map[2][2] = 1;
    map[2][3] = 1;
    map[2][4] = 1;

    // exit
    map[gridSize - 1][gridSize - 1] = 2;

    // enemy
    map[5][5] = 3;

}

void GameWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // Draw grid
    for (int i = 0; i < gridSize; i++)
    {
        for (int j = 0; j < gridSize; j++)
        {
            int x = i * tileSize;
            int y = j * tileSize;

            // walls
            if (map[i][j] == 1)
                painter.fillRect(x, y, tileSize, tileSize, Qt::black);

            // exit
            if (map[i][j] == 2)
                painter.fillRect(x, y, tileSize, tileSize, Qt::green);

            // enemy
            if (map[i][j] == 3)
                painter.fillRect(x, y, tileSize, tileSize, Qt::red);

            painter.drawRect(i * tileSize, j * tileSize, tileSize, tileSize);
        }
    }

    // Draw player
    painter.fillRect(playerX * tileSize,
                     playerY * tileSize,
                     tileSize,
                     tileSize,
                     Qt::blue);
}

void GameWidget::keyPressEvent(QKeyEvent *event)
{
    int newX = playerX;
    int newY = playerY;

    if (event->key() == Qt::Key_Up) newY--;
    if (event->key() == Qt::Key_Down) newY++;
    if (event->key() == Qt::Key_Left) newX--;
    if (event->key() == Qt::Key_Right) newX++;

    // boundary check
    if (newX < 0 || newX >= gridSize || newY < 0 || newY >= gridSize)
        return;

    // wall check
    if (map[newX][newY] == 1)
        return;

    // move player
    playerX = newX;
    playerY = newY;

    // enemy collision
    if (map[playerX][playerY] == 3)
    {
        emit gameLost();
        return;
    }

    // win condition
    if (map[playerX][playerY] == 2)
    {
        emit gameWon();
        return;
    }

    update(); // redraw screen
}
