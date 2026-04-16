#include "gameview.h"
#include <QPainter>

GameView::GameView(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(800, 600);
}

void GameView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // ===== SKY / BACKGROUND =====
    painter.fillRect(rect(), QColor(25, 25, 40));

    // ===== FOREST GROUND =====
    painter.setBrush(QColor(34, 85, 34));
    painter.drawRect(0, 200, 800, 400);

    // ===== RIVER =====
    painter.setBrush(QColor(30, 144, 255));
    painter.drawRect(350, 0, 100, 600);

    // ===== COTTAGE =====
    painter.setBrush(QColor(139, 69, 19));
    painter.drawRect(600, 250, 120, 120);

    // ===== PLAYER =====
    painter.setBrush(QColor(120, 180, 255));
    painter.drawEllipse(game.getPlayer().getX(),
                        game.getPlayer().getY(),
                        30, 30);

    // ===== ENEMY =====
    painter.setBrush(QColor(200, 60, 60));
    painter.drawEllipse(game.getEnemy().getX(),
                        game.getEnemy().getY(),
                        35, 35);

    // ===== STORY TEXT =====
    painter.setPen(Qt::white);
    painter.setFont(QFont("Times", 14));

    painter.drawText(20, 30, "Level 1: The Haunted Forest");
    painter.drawText(20, 55, "Escape... or defeat the shadow beast near the cottage.");
if (game.getStoryState() == 1)
{
    painter.drawText(20, 80, "You reached the cottage... something feels wrong.");
}
}

void GameView::keyPressEvent(QKeyEvent *event)
{
    int dx = 0, dy = 0;

    if (event->key() == Qt::Key_Up) dy -= 10;
    else if (event->key() == Qt::Key_Down) dy += 10;
    else if (event->key() == Qt::Key_Left) dx -= 10;
    else if (event->key() == Qt::Key_Right) dx += 10;

    game.movePlayer(dx, dy);
    game.updateEnemy();

    if (game.checkWin()) emit gameWon();
    if (game.checkLose()) emit gameLost();

    update();
}

void GameView::resetGame(int size)
{
    game = Game(size);
    update();
}
