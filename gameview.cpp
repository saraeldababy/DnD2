// #include "gameview.h"
// #include <QPainter>
// #include <QKeyEvent>

// GameView::GameView(QWidget *parent)
//     : QWidget(parent), tileSize(40)
// {
//     setFocusPolicy(Qt::StrongFocus);
//     setFixedSize(10 * tileSize, 10 * tileSize);
// }

// void GameView::paintEvent(QPaintEvent *)
// {
//     QPainter painter(this);

//     Level &level = game.getLevel();

//     for (int i = 0; i < level.gridSize; i++)
//     {
//         for (int j = 0; j < level.gridSize; j++)
//         {
//             int x = i * tileSize;
//             int y = j * tileSize;

//             if (level.map[i][j] == 1)
//                 painter.fillRect(x,y,tileSize,tileSize,Qt::black);

//             else if (level.map[i][j] == 2)
//                 painter.fillRect(x,y,tileSize,tileSize,Qt::green);

//             painter.drawRect(x,y,tileSize,tileSize);
//         }
//     }

//     //RECTANGLES
//     // // player
//     // painter.fillRect(game.getPlayer().getX()*tileSize,
//     //                  game.getPlayer().getY()*tileSize,
//     //                  tileSize, tileSize, Qt::blue);
//     // // enemy
//     // painter.fillRect(game.getEnemy().getX()*tileSize,
//     //                  game.getEnemy().getY()*tileSize,
//     //                  tileSize, tileSize, Qt::red);

//     //ELLIPSE
//     //player
//     painter.setBrush(Qt::blue);
//     painter.drawEllipse(
//         game.getPlayer().getX()*tileSize,
//         game.getPlayer().getY()*tileSize,
//         tileSize, tileSize);
//     //enemy
//     painter.setBrush(Qt::red);
//     painter.drawEllipse(
//         game.getEnemy().getX()*tileSize,
//         game.getEnemy().getY()*tileSize,
//         tileSize, tileSize);

//     //4PM WORKING
//     //health
//     // painter.drawText(10, 20,
//     //                  "HP: " + QString::number(game.getPlayer().getHealth()));
//     // QString ui =
//     //     "HP: " + QString::number(game.getPlayer().getHealth()) +
//     //     "  Player Roll: " + QString::number(game.getLastPlayerRoll()) +
//     //     "  Enemy Roll: " + QString::number(game.getLastEnemyRoll());

//     //painter.drawText(10, 20, ui);
// }

// void GameView::keyPressEvent(QKeyEvent *event)
// {
//     int dx = 0, dy = 0;

//     if (event->key() == Qt::Key_Up) dy--;
//     else if (event->key() == Qt::Key_Down) dy++;
//     else if (event->key() == Qt::Key_Left) dx--;
//     else if (event->key() == Qt::Key_Right) dx++;

//     if (dx || dy)
//     {
//         game.movePlayer(dx, dy);
//         game.updateEnemy();

//         if (game.checkLose()) emit gameLost();
//         if (game.checkWin()) emit gameWon();

//         update();
//     }
// }
// void GameView::resetGame(int size)
// {
//     game = Game(size);
//     setFixedSize(size * tileSize, size * tileSize);
//     update();
// }

//CODEX
#include "gameview.h"

#include <QPainter>
#include <QVector>

GameView::GameView(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(800, 600);
}


void GameView::setPlayerProfile(const QString &name, const QString &role)
{
    playerName = name.isEmpty() ? "Adventurer" : name;
    game.getPlayer().setRole(role);
    showIntroDialog = true;
    update();
}

void GameView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    constexpr int tileSize = 48;
    constexpr int offsetX = 160;
    constexpr int offsetY = 70;

    QLinearGradient sky(0, 0, 0, height());
    sky.setColorAt(0.0, QColor(23, 26, 50));
    sky.setColorAt(0.55, QColor(32, 58, 49));
    sky.setColorAt(1.0, QColor(35, 73, 47));
    painter.fillRect(rect(), sky);

    Level &level = game.getLevel();

    // Base forest clearing (no visible grid lines).
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(65, 102, 58));
    painter.drawRoundedRect(QRect(offsetX - 14, offsetY - 14, level.gridSize * tileSize + 28, level.gridSize * tileSize + 28), 14, 14);

    // Paint the scene by terrain type without drawing tile outlines.
    for (int x = 0; x < level.gridSize; ++x)
    {
        for (int y = 0; y < level.gridSize; ++y)
        {
            QRect tile(offsetX + x * tileSize,
                       offsetY + y * tileSize,
                       tileSize,
                       tileSize);

            const int kind = level.map[x][y];

            if (kind == Level::RIVER)
            {
                painter.setBrush(QColor(40, 101, 177));
                painter.drawRect(tile.adjusted(0, 0, 0, 0));
                painter.setPen(QPen(QColor(74, 139, 214, 140), 2));
                painter.drawLine(tile.left() + 4, tile.center().y(), tile.right() - 4, tile.center().y() - 3);
                painter.setPen(Qt::NoPen);
            }
            else if (kind == Level::TREE)
            {
                // Decorative forest clusters players can't move through.
                painter.setBrush(QColor(27, 66, 37));
                painter.drawEllipse(tile.adjusted(8, 5, -8, -18));
                painter.drawEllipse(tile.adjusted(14, 2, -12, -20));
                painter.setBrush(QColor(89, 59, 38));
                painter.drawRoundedRect(tile.x() + tile.width() / 2 - 4, tile.y() + tile.height() / 2, 8, 18, 3, 3);
            }
            else if (kind == Level::PATH)
            {
                painter.setBrush(QColor(126, 107, 79));
                painter.drawRoundedRect(tile.adjusted(6, 12, -6, -12), 8, 8);
            }
            else if (kind == Level::BRIDGE)
            {
                // Visible wooden bridge over river.
                painter.setBrush(QColor(138, 95, 60));
                painter.drawRoundedRect(tile.adjusted(2, 6, -2, -6), 6, 6);
                painter.setPen(QPen(QColor(95, 64, 38), 2));
                for (int i = 8; i <= 38; i += 8)
                     painter.drawLine(tile.left() + i, tile.top() + 8, tile.left() + i, tile.bottom() - 8);
                painter.setPen(Qt::NoPen);
            }
        }
    }

    // Lanterns along the path so the route is visible.
    const QVector<QPoint> lanternTiles = {
        QPoint(2, 8), QPoint(4, 8), QPoint(5, 7), QPoint(5, 6)
};
for (const QPoint &p : lanternTiles)
{
    const int lx = offsetX + p.x() * tileSize + 22;
    const int ly = offsetY + p.y() * tileSize + 16;
    painter.setPen(QPen(QColor(76, 55, 40), 2));
    painter.drawLine(lx, ly, lx, ly + 18);
    painter.setBrush(QColor(255, 210, 118));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(lx - 5, ly - 2, 10, 10);
    painter.setBrush(QColor(255, 206, 120, 60));
    painter.drawEllipse(lx - 14, ly - 10, 28, 28);
}

// Detailed medieval cottage.
const QRect cottageBase(offsetX + 7 * tileSize + 4, offsetY + 2 * tileSize + 10, tileSize * 2 - 8, tileSize * 2 - 14);painter.setBrush(QColor(168, 130, 94));
painter.setPen(Qt::NoPen);
painter.drawRoundedRect(cottageBase, 7, 7);

QPoint roof[3] = {
                  QPoint(cottageBase.left() - 10, cottageBase.top() + 6),
                  QPoint(cottageBase.right() + 10, cottageBase.top() + 6),
                  QPoint(cottageBase.center().x(), cottageBase.top() - 28)};
painter.setBrush(QColor(118, 66, 48));
painter.drawPolygon(roof, 3);

painter.setBrush(QColor(109, 74, 47));
painter.drawRoundedRect(cottageBase.center().x() - 10, cottageBase.bottom() - 28, 20, 28, 4, 4); // door
painter.setBrush(QColor(241, 225, 164));
painter.drawRect(cottageBase.left() + 10, cottageBase.top() + 16, 14, 12); // window 1
painter.drawRect(cottageBase.right() - 24, cottageBase.top() + 16, 14, 12); // window 2ndow 2
painter.setBrush(QColor(96, 96, 96));
painter.drawRect(cottageBase.right() - 6, cottageBase.top() - 18, 10, 20); // chimney

// Player graphics including role-based cloak color.
QColor cloakColor(76, 124, 215); // default
const QString role = game.getPlayer().getRole();
if (role == "Wizard") cloakColor = QColor(85, 102, 224);
else if (role == "Fighter") cloakColor = QColor(171, 72, 65);
else if (role == "Rogue") cloakColor = QColor(65, 138, 94);
else if (role == "Cleric") cloakColor = QColor(199, 170, 88);

const int px = offsetX + game.getPlayer().getX() * tileSize;
const int py = offsetY + game.getPlayer().getY() * tileSize;
painter.setBrush(cloakColor);
painter.setPen(Qt::NoPen);
painter.drawRoundedRect(px + 12, py + 14, 24, 26, 8, 8);
painter.setBrush(QColor(236, 207, 169));
painter.drawEllipse(px + 16, py + 4, 16, 16);
painter.setPen(QPen(QColor(210, 190, 126), 3));
painter.drawLine(px + 34, py + 14, px + 40, py + 34); // right arm
painter.drawLine(px + 14, py + 14, px + 8, py + 34);  // left arm

// Enemy sprite: shadow body and glowing eyes.
const int ex = offsetX + game.getEnemy().getX() * tileSize;
const int ey = offsetY + game.getEnemy().getY() * tileSize;
painter.setPen(Qt::NoPen);
painter.setBrush(QColor(62, 24, 63));
painter.drawEllipse(ex + 8, ey + 10, 32, 30);
painter.setBrush(QColor(255, 83, 123));
painter.drawEllipse(ex + 15, ey + 20, 5, 5);
painter.drawEllipse(ex + 26, ey + 20, 5, 5);

// Intro dialog bubbles (hide when player starts moving).
if (showIntroDialog)
{
    painter.setBrush(QColor(255, 248, 230, 230));
    painter.setPen(QPen(QColor(70, 55, 45), 1));
    painter.drawRoundedRect(QRect(px - 100, py - 62, 190, 44), 8, 8);
    painter.drawText(QRect(px - 94, py - 58, 178, 36), Qt::TextWordWrap,
                     playerName + ": I'm scared... help me reach the cottage!");

    painter.setBrush(QColor(40, 20, 48, 230));
    painter.setPen(QPen(QColor(176, 120, 176), 1));
    painter.drawRoundedRect(QRect(ex - 40, ey - 62, 190, 44), 8, 8);
    painter.setPen(QColor(240, 215, 255));
    painter.drawText(QRect(ex - 34, ey - 58, 178, 36), Qt::TextWordWrap,
                     "Shadow: You'll have to escape me first.");
}

painter.setPen(QColor(243, 232, 206));
painter.setFont(QFont("Georgia", 18, QFont::Bold));
painter.drawText(24, 34, "Level 1 - The Whispering Forest");
painter.setFont(QFont("Georgia", 11));
painter.drawText(24, 56, "Arrow Keys: Move | Follow the lanterns to the cottage");
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

    if (dx != 0 || dy != 0)
        showIntroDialog = false;

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
    const QString role = game.getPlayer().getRole();
    game = Game(size);
    game.getPlayer().setRole(role);
    showIntroDialog = true;

    update();
}
