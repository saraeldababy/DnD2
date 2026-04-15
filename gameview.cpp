#include "gameview.h"
#include <QPainter>
#include <QKeyEvent>

GameView::GameView(QWidget *parent)
    : QWidget(parent), tileSize(40)
{
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(10 * tileSize, 10 * tileSize);
}

void GameView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    Level &level = game.getLevel();

    for (int i = 0; i < level.gridSize; i++)
    {
        for (int j = 0; j < level.gridSize; j++)
        {
            int x = i * tileSize;
            int y = j * tileSize;

            if (level.map[i][j] == 1)
                painter.fillRect(x,y,tileSize,tileSize,Qt::black);

            else if (level.map[i][j] == 2)
                painter.fillRect(x,y,tileSize,tileSize,Qt::green);

            painter.drawRect(x,y,tileSize,tileSize);
        }
    }

    //RECTANGLES
    // // player
    // painter.fillRect(game.getPlayer().getX()*tileSize,
    //                  game.getPlayer().getY()*tileSize,
    //                  tileSize, tileSize, Qt::blue);
    // // enemy
    // painter.fillRect(game.getEnemy().getX()*tileSize,
    //                  game.getEnemy().getY()*tileSize,
    //                  tileSize, tileSize, Qt::red);

    //ELLIPSE
    //player
    painter.setBrush(Qt::blue);
    painter.drawEllipse(
        game.getPlayer().getX()*tileSize,
        game.getPlayer().getY()*tileSize,
        tileSize, tileSize);
    //enemy
    painter.setBrush(Qt::red);
    painter.drawEllipse(
        game.getEnemy().getX()*tileSize,
        game.getEnemy().getY()*tileSize,
        tileSize, tileSize);

    //4PM WORKING
    //health
    // painter.drawText(10, 20,
    //                  "HP: " + QString::number(game.getPlayer().getHealth()));
    // QString ui =
    //     "HP: " + QString::number(game.getPlayer().getHealth()) +
    //     "  Player Roll: " + QString::number(game.getLastPlayerRoll()) +
    //     "  Enemy Roll: " + QString::number(game.getLastEnemyRoll());

    //painter.drawText(10, 20, ui);
}

void GameView::keyPressEvent(QKeyEvent *event)
{
    int dx = 0, dy = 0;

    if (event->key() == Qt::Key_Up) dy--;
    else if (event->key() == Qt::Key_Down) dy++;
    else if (event->key() == Qt::Key_Left) dx--;
    else if (event->key() == Qt::Key_Right) dx++;

    if (dx || dy)
    {
        game.movePlayer(dx, dy);
        game.updateEnemy();

        if (game.checkLose()) emit gameLost();
        if (game.checkWin()) emit gameWon();

        update();
    }
}
void GameView::resetGame(int size)
{
    game = Game(size);
    setFixedSize(size * tileSize, size * tileSize);
    update();
}
