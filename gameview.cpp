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
    painter.setRenderHint(QPainter::Antialiasing, true);

    constexpr int tileSize = 48;
    constexpr int offsetX = 160;
    constexpr int offsetY = 70;

    painter.fillRect(rect(), QColor(17, 19, 31));

    // Painted sky gradient for an indie storybook mood.
    QLinearGradient sky(0, 0, 0, height());
    sky.setColorAt(0.0, QColor(20, 23, 44));
    sky.setColorAt(0.6, QColor(27, 42, 38));
    sky.setColorAt(1.0, QColor(38, 52, 35));
    painter.fillRect(rect(), sky);

    Level &level = game.getLevel();

    // Draw tile map.
    for (int x = 0; x < level.gridSize; ++x)
    {
        for (int y = 0; y < level.gridSize; ++y)
        {
            const QRect tile(offsetX + x * tileSize,
                             offsetY + y * tileSize,
                             tileSize,
                             tileSize);

            const int kind = level.map[x][y];
            if (kind == Level::GRASS)
            {
                painter.fillRect(tile, QColor(72, 105, 61));
            }
            else if (kind == Level::TREE)
            {
                painter.fillRect(tile, QColor(45, 74, 45));
                painter.setBrush(QColor(31, 52, 31));
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(tile.adjusted(10, 6, -10, -14));
                painter.setBrush(QColor(88, 56, 37));
                painter.drawRect(tile.x() + tile.width() / 2 - 4, tile.y() + tile.height() / 2, 8, 18);
            }
            else if (kind == Level::RIVER)
            {
                painter.fillRect(tile, QColor(49, 109, 184));
            }
            else if (kind == Level::PATH)
            {
                painter.fillRect(tile, QColor(130, 114, 84));
            }
            else if (kind == Level::BRIDGE)
            {
                painter.fillRect(tile, QColor(141, 94, 53));
                painter.setPen(QColor(88, 58, 33));
                for (int i = 8; i < tileSize; i += 10)
                    painter.drawLine(tile.x() + i, tile.y() + 4, tile.x() + i, tile.y() + tileSize - 4);
            }
            else if (kind == Level::COTTAGE)
            {
                painter.fillRect(tile, QColor(168, 130, 86));
            }

            painter.setPen(QColor(0, 0, 0, 50));
            painter.drawRect(tile);
        }
    }

    // Cottage roof overlay for detail.
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(113, 64, 46));
    QPoint roof[3] = {
        QPoint(offsetX + 7 * tileSize, offsetY + 2 * tileSize),
        QPoint(offsetX + 9 * tileSize + tileSize, offsetY + 2 * tileSize),
        QPoint(offsetX + 8 * tileSize + tileSize / 2, offsetY + tileSize)
    };
    painter.drawPolygon(roof, 3);

    // Player sprite: cloak, head, and staff.
    const int px = offsetX + game.getPlayer().getX() * tileSize;
    const int py = offsetY + game.getPlayer().getY() * tileSize;
    painter.setBrush(QColor(73, 123, 214));
    painter.drawRoundedRect(px + 12, py + 14, 24, 26, 8, 8);
    painter.setBrush(QColor(236, 207, 169));
    painter.drawEllipse(px + 16, py + 4, 16, 16);
    painter.setPen(QPen(QColor(210, 190, 126), 3));
    painter.drawLine(px + 34, py + 14, px + 40, py + 34);

    // Enemy sprite: shadow body and glowing eyes.
    const int ex = offsetX + game.getEnemy().getX() * tileSize;
    const int ey = offsetY + game.getEnemy().getY() * tileSize;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(63, 24, 63));
    painter.drawEllipse(ex + 8, ey + 10, 32, 30);
    painter.setBrush(QColor(255, 83, 123));
    painter.drawEllipse(ex + 15, ey + 20, 5, 5);
    painter.drawEllipse(ex + 26, ey + 20, 5, 5);

    painter.setPen(QColor(241, 231, 205));
    painter.setFont(QFont("Georgia", 18, QFont::Bold));
    painter.drawText(24, 34, "Level 1 - The Whispering Forest");
    painter.setFont(QFont("Georgia", 11));
    painter.drawText(24, 56, "Arrow Keys: Move | Reach the cottage to win");
    painter.drawText(24, 80, game.storyHint());
}

void GameView::keyPressEvent(QKeyEvent *event)
{
    int dx = 0, dy = 0;

    if (event->key() == Qt::Key_Up)
        dy -= 1;
    else if (event->key() == Qt::Key_Down)
        dy += 1;
    else if (event->key() == Qt::Key_Left)
        dx -= 1;
    else if (event->key() == Qt::Key_Right)
        dx += 1;

    game.movePlayer(dx, dy);
    game.updateEnemy();

    if (game.checkWin())
        emit gameWon();
    if (game.checkLose())
        emit gameLost();

    update();
}

void GameView::resetGame(int size)
{
    game = Game(size);
    update();
}
