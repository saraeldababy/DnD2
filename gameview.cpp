#include "gameview.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFont>
#include <cmath>

// -------------------------------------------------------
// Construction
// -------------------------------------------------------

GameView::GameView(QWidget *parent)
    : QWidget(parent), playerName("Adventurer"),
    showIntroDialog(true), introTimer(200),
    riddleSelected(0), animFrame(0),
    trapHideTimer(nullptr), alarmCountdownTimer(nullptr),
    alarmSecondsLeft(0), l4AlarmTimerStarted(false)
{
    setFocusPolicy(Qt::StrongFocus);
    setFixedSize(900, 680);

    animTimer = new QTimer(this);
    connect(animTimer, &QTimer::timeout, this, [this]() {
        animFrame++;
        game.updateProjectiles();
        game.clearFlashEvents();
        game.tickStoryMessage();
        update();
    });
    animTimer->start(40); // 25fps
}

void GameView::setPlayerProfile(const QString &name, const QString &role)
{
    playerName = name.isEmpty() ? "Adventurer" : name;
    game.getPlayer().setName(playerName);
    game.getPlayer().setRole(role);
    showIntroDialog = true;
    introTimer = 200;
    update();
}

void GameView::clearLevel4Timers()
{
    if (trapHideTimer)      { trapHideTimer->stop();      trapHideTimer->deleteLater();      trapHideTimer = nullptr; }
    if (alarmCountdownTimer){ alarmCountdownTimer->stop(); alarmCountdownTimer->deleteLater(); alarmCountdownTimer = nullptr; }
    l4AlarmTimerStarted = false;
    alarmSecondsLeft = 0;
}

void GameView::resetGame()
{
    clearLevel4Timers();   // ← add this
    const QString name = game.getPlayer().getName();
    const QString role = game.getPlayer().getRole();
    game.reinit();                    // safe in-place reset, no copy
    game.getPlayer().setName(name);
    game.getPlayer().setRole(role);
    showIntroDialog = true;
    introTimer = 200;
    update();
}

Game &GameView::getGame() { return game; }

// -------------------------------------------------------
// Helpers
// -------------------------------------------------------

QRect GameView::tileRect(int x, int y, int tileSize, int ox, int oy) const
{
    return QRect(ox + x * tileSize, oy + y * tileSize, tileSize, tileSize);
}

void GameView::drawHealthBar(QPainter &p, int x, int y, int w, int h,
                             int hp, int maxhp, QColor fill)
{
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(40, 20, 20));
    p.drawRoundedRect(x, y, w, h, 3, 3);
    int filled = (maxhp > 0) ? (w * hp / maxhp) : 0;
    p.setBrush(fill);
    p.drawRoundedRect(x, y, filled, h, 3, 3);
    p.setPen(QColor(255, 255, 255, 120));
    p.setFont(QFont("Georgia", 7));
    p.drawText(QRect(x, y, w, h), Qt::AlignCenter,
               QString::number(hp) + "/" + QString::number(maxhp));
}

// -------------------------------------------------------
// HUD
// -------------------------------------------------------

void GameView::drawHUD(QPainter &p)
{
    const Player &pl = game.getPlayer();
    const int W = width();

    // Dark HUD bar at top
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(15, 10, 25, 220));
    p.drawRect(0, 0, W, 52);

    // Level title
    static const char *levelTitles[] = {
        "Level 1 — The Whispering Forest",
        "Level 2 — The Witch's Cottage",
        "Level 3 — Castle Exterior",
        "Level 4 — Castle Interior",
        "Level 5 — The Dragon's Dungeon"
    };
    int lv = game.getCurrentLevel() - 1;
    if (lv < 0) lv = 0; if (lv > 4) lv = 4;

    p.setPen(QColor(255, 220, 100));
    p.setFont(QFont("Georgia", 14, QFont::Bold));
    p.drawText(14, 22, levelTitles[lv]);

    p.setFont(QFont("Georgia", 9));
    p.setPen(QColor(200, 185, 155));
    p.drawText(14, 40, game.storyHint());

    // Right side: health, potions, keys, score
    const int rX = W - 340;
    p.setPen(QColor(255, 100, 100));
    p.setFont(QFont("Georgia", 10, QFont::Bold));
    p.drawText(rX, 18, pl.getName() + " [" + pl.getRole() + "]");
    drawHealthBar(p, rX, 22, 200, 14, pl.getHealth(), pl.getMaxHealth(),
                  pl.getHealth() > 40 ? QColor(80, 200, 80) : QColor(220, 60, 60));

    p.setPen(QColor(200, 230, 255));
    p.setFont(QFont("Georgia", 9));
    p.drawText(rX + 210, 32, QString("💊 %1  🗝 %2  ⭐ %3")
                                 .arg(pl.getPotions()).arg(pl.getKeys()).arg(pl.getScore()));

    // Separator line
    p.setPen(QPen(QColor(80, 60, 40), 1));
    p.drawLine(0, 52, W, 52);

    // Alarm countdown for level 4
//     if (game.isAlarmActive() && game.getCurrentLevel() == 4)
//     {
//         int flash = 180 + (int)(75 * sin(animFrame * 0.3));
//         p.setFont(QFont("Georgia", 13, QFont::Bold));
//         p.setPen(QColor(255, flash > 255 ? 255 : flash, 0));
//         p.drawText(W / 2 - 110, 20, "!! ALARM — " +
//                                         QString::number(game.getAlarmTimer()) + " turns left !!");
//     }
// }
    if (game.isAlarmActive() && game.getCurrentLevel() == 4)
    {
        int flash = 180 + (int)(75 * sin(animFrame * 0.4));
        int c = flash > 255 ? 255 : flash;
        p.setFont(QFont("Georgia", 15, QFont::Bold));
        // if (!game.areGuardsChasing()) {
        //     p.setPen(QColor(255, c, 0));
        //     p.drawText(W / 2 - 90, 24, "!! ALARM — " + QString::number(alarmSecondsLeft) + " !!");
        // } else {
        //     p.setPen(QColor(255, c, c));
        //     p.drawText(W / 2 - 120, 24, "!! GUARDS ARE HUNTING YOU !!");
        // }
        p.setPen(QColor(255, c, c));
        p.drawText(W / 2 - 130, 24,
                   "!! GUARDS HUNTING — " + QString::number(alarmSecondsLeft) + "s !!");
    }
}

// -------------------------------------------------------
// Story banner (bottom)
// -------------------------------------------------------

void GameView::drawStoryBanner(QPainter &p)
{
    if (!game.hasStoryMessage()) return;
    const QString msg = game.getLatestStoryMessage();
    const int W = width(); const int H = height();

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(20, 14, 35, 210));
    p.drawRoundedRect(20, H - 66, W - 40, 52, 8, 8);
    p.setPen(QPen(QColor(120, 90, 160), 1));
    p.drawRoundedRect(20, H - 66, W - 40, 52, 8, 8);

    p.setPen(QColor(240, 228, 200));
    p.setFont(QFont("Georgia", 11));
    p.drawText(QRect(30, H - 62, W - 60, 44), Qt::AlignVCenter | Qt::TextWordWrap, msg);
}

// -------------------------------------------------------
// Flash events
// -------------------------------------------------------

void GameView::drawFlashEvents(QPainter &p, int ox, int oy, int tileSize)
{
    for (const FlashEvent &fe : game.getFlashEvents())
    {
        int cx = ox + fe.x * tileSize + tileSize / 2;
        int cy = oy + fe.y * tileSize + tileSize / 2;
        int alpha = (int)(255 * fe.framesLeft / 30.0);

        if (fe.type == "hit")
        {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 60, 60, alpha));
            p.drawEllipse(cx - 16, cy - 16, 32, 32);
        }
        else if (fe.type == "defeat")
        {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 200, 50, alpha));
            for (int r = 0; r < 6; ++r)
            {
                float angle = r * 60.0f * M_PI / 180.0f;
                int sx = cx + (int)(18 * cos(angle));
                int sy = cy + (int)(18 * sin(angle));
                p.drawEllipse(sx - 4, sy - 4, 8, 8);
            }
        }
        else if (fe.type == "treasure")
        {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 230, 0, alpha));
            p.drawEllipse(cx - 12, cy - 12, 24, 24);
        }
        else if (fe.type == "megafire")
        {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 80, 0, alpha));
            p.drawEllipse(cx - 22, cy - 22, 44, 44);
            p.setBrush(QColor(255, 200, 50, alpha / 2));
            p.drawEllipse(cx - 32, cy - 32, 64, 64);
        }
    }
}

// -------------------------------------------------------
// Player ui
// -------------------------------------------------------

void GameView::drawPlayer(QPainter &p, int px, int py, int tileSize)
{
    Q_UNUSED(tileSize);
    const QString role = game.getPlayer().getRole();
    QColor cloak(76, 124, 215);
    if (role == "Wizard")  cloak = QColor(85,  102, 224);
    else if (role == "Fighter") cloak = QColor(171, 72, 65);
    else if (role == "Rogue")   cloak = QColor(65, 138, 94);
    else if (role == "Cleric")  cloak = QColor(199, 170, 88);

    // Subtle bob animation
    int bob = (int)(2 * sin(animFrame * 0.18));

    p.setPen(Qt::NoPen);
    // Cloak body
    p.setBrush(cloak);
    p.drawRoundedRect(px + 12, py + 14 + bob, 24, 26, 8, 8);
    // Head
    p.setBrush(QColor(236, 207, 169));
    p.drawEllipse(px + 16, py + 4 + bob, 16, 16);
    // Arms
    p.setPen(QPen(cloak.darker(120), 3));
    p.drawLine(px + 34, py + 14 + bob, px + 40, py + 34 + bob);
    p.drawLine(px + 14, py + 14 + bob, px + 8,  py + 34 + bob);
    p.setPen(Qt::NoPen);
    // Role accessory
    if (role == "Wizard")
    {
        // Staff
        p.setPen(QPen(QColor(160, 140, 90), 2));
        p.drawLine(px + 40, py + 10 + bob, px + 40, py + 40 + bob);
        p.setBrush(QColor(150, 100, 255));
        p.setPen(Qt::NoPen);
        p.drawEllipse(px + 36, py + 6 + bob, 8, 8);
    }
    else if (role == "Fighter")
    {
        p.setBrush(QColor(180, 180, 180));
        p.drawRoundedRect(px + 8, py + 18 + bob, 6, 14, 2, 2); // shield
    }
    else if (role == "Rogue")
    {
        p.setBrush(QColor(60, 60, 60));
        p.drawEllipse(px + 14, py + 2 + bob, 18, 10); // hood
    }
    else if (role == "Cleric")
    {
        p.setPen(QPen(QColor(255, 255, 200, 180), 2));
        p.drawEllipse(px + 13, py + 0 + bob, 22, 22); // halo
        p.setPen(Qt::NoPen);
    }

    // Sword visual
    if (game.playerHasSword())
    {
        p.save();
        p.translate(px + 40, py + 22 + bob);
        p.rotate(-45);
        p.setPen(Qt::NoPen);
        // Blade body
        QPoint sblade[] = { QPoint(-2,0), QPoint(2,0), QPoint(2,-14), QPoint(-2,-14) };
        p.setBrush(QColor(210, 220, 200));
        p.drawPolygon(sblade, 4);
        // Blade tip
        QPoint stip[] = { QPoint(-2,-14), QPoint(2,-14), QPoint(0,-18) };
        p.drawPolygon(stip, 3);
        // Crossguard
        p.setBrush(QColor(180, 130, 40));
        p.drawRoundedRect(-6, 0, 12, 3, 1, 1);
        // Handle
        p.setBrush(QColor(110, 65, 25));
        p.drawRoundedRect(-2, 3, 4, 7, 1, 1);
        // Pommel glow
        p.setBrush(QColor(255, 200, 50, 80 + (int)(40*sin(animFrame*0.2))));
        p.drawEllipse(-4, 9, 8, 8);
        p.restore();
    }

    // Attack range indicator (faint circle)
    if (role == "Wizard")
    {
        p.setPen(QPen(QColor(120, 100, 255, 40), 1, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(px - 48 + 24, py - 48 + 24, 96 + 24, 96);
        p.setPen(Qt::NoPen);
    }
}

// -------------------------------------------------------
// Enemy sprites
// -------------------------------------------------------

void GameView::drawEnemy(QPainter &p, const Enemy *e, int ox, int oy, int tileSize)
{
    if (e->isDefeated()) return;
    int ex = ox + e->getX() * tileSize;
    int ey = oy + e->getY() * tileSize;
    int bob = (int)(2 * sin(animFrame * 0.12 + e->getId()));

    p.setPen(Qt::NoPen);

    switch (e->getType())
    {
    case EnemyType::SHADOW:
        p.setBrush(QColor(62, 24, 63, 210));
        p.drawEllipse(ex + 8, ey + 10 + bob, 32, 30);
        p.setBrush(QColor(255, 83, 123));
        p.drawEllipse(ex + 15, ey + 20 + bob, 5, 5);
        p.drawEllipse(ex + 26, ey + 20 + bob, 5, 5);
        // Tendrils
        p.setPen(QPen(QColor(120, 50, 130, 160), 2));
        p.drawLine(ex + 10, ey + 36 + bob, ex + 4, ey + 44 + bob);
        p.drawLine(ex + 24, ey + 38 + bob, ex + 22, ey + 46 + bob);
        p.drawLine(ex + 38, ey + 36 + bob, ex + 44, ey + 44 + bob);
        p.setPen(Qt::NoPen);
        break;

    case EnemyType::WITCH:
    {
        // Robe
        p.setBrush(QColor(60, 20, 80));
        p.drawRoundedRect(ex + 10, ey + 14 + bob, 28, 28, 6, 6);
        // Head
        p.setBrush(QColor(190, 160, 130));
        p.drawEllipse(ex + 14, ey + 4 + bob, 20, 20);
        // Hat
        QPoint hat[3] = {
            QPoint(ex + 12, ey + 10 + bob),
            QPoint(ex + 36, ey + 10 + bob),
            QPoint(ex + 24, ey - 14 + bob)
        };
        p.setBrush(QColor(30, 10, 50));
        p.drawPolygon(hat, 3);
        // Hat brim
        p.setBrush(QColor(40, 15, 60));
        p.drawEllipse(ex + 8, ey + 8 + bob, 32, 8);
        // Eyes (glowing)
        p.setBrush(QColor(0, 255, 128));
        p.drawEllipse(ex + 17, ey + 12 + bob, 4, 4);
        p.drawEllipse(ex + 26, ey + 12 + bob, 4, 4);
        // Health bar
        drawHealthBar(p, ex, ey - 14, tileSize, 8, e->getHealth(), e->getMaxHealth(),
                      QColor(180, 50, 200));
        break;
    }

    case EnemyType::ARCHER:
    {
        // Body
        p.setBrush(QColor(80, 60, 40));
        p.drawRoundedRect(ex + 10, ey + 14 + bob, 22, 24, 5, 5);
        // Head with helmet
        p.setBrush(QColor(190, 160, 120));
        p.drawEllipse(ex + 13, ey + 4 + bob, 18, 18);
        p.setBrush(QColor(100, 90, 80));
        p.drawRoundedRect(ex + 11, ey + 2 + bob, 22, 12, 4, 4); // helmet
        // Bow
        p.setPen(QPen(QColor(100, 70, 40), 2));
        p.setBrush(Qt::NoBrush);
        p.drawArc(ex + 32, ey + 10 + bob, 12, 24, 30*16, 120*16);
        p.setPen(QPen(QColor(200, 180, 130), 1));
        p.drawLine(ex + 34, ey + 11 + bob, ex + 34, ey + 33 + bob);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::NoBrush);
        // Health bar
        drawHealthBar(p, ex, ey - 10, tileSize, 7, e->getHealth(), e->getMaxHealth(),
                      QColor(60, 180, 80));
        break;
    }

    case EnemyType::TRAP:
    {
        const TrapEnemy *trap = static_cast<const TrapEnemy*>(e);
        if (!trap->isVisible()) break; // hidden
        // Show triggered trap
        p.setBrush(trap->isTriggered() ? QColor(255, 80, 20, 180) : QColor(160, 40, 40, 120));
        p.drawRect(ex + 6, ey + 20, tileSize - 12, 8);
        p.setBrush(QColor(200, 60, 40, 180));
        for (int i = 0; i < 4; ++i)
        {
            float a = i * 90.0f * M_PI / 180.0f;
            p.drawEllipse((int)(ex + 24 + 12*cos(a)) - 3, (int)(ey + 28 + 8*sin(a)) - 3, 6, 6);
        }
        break;
    }

    case EnemyType::DRAGON:
    {
        const DragonEnemy *dragon = static_cast<const DragonEnemy*>(e);
        int phase = dragon->getPhase();
        QColor bodyColor = (phase == 2) ? QColor(180, 40, 20) : QColor(60, 120, 50);

        // Wings
        p.setBrush(bodyColor.darker(140));
        QPoint wingL[4] = {
            QPoint(ex + 8, ey + 16 + bob),
            QPoint(ex - 16, ey + 4 + bob),
            QPoint(ex - 8, ey + 32 + bob),
            QPoint(ex + 8, ey + 30 + bob)
        };
        QPoint wingR[4] = {
            QPoint(ex + 40, ey + 16 + bob),
            QPoint(ex + 64, ey + 4 + bob),
            QPoint(ex + 56, ey + 32 + bob),
            QPoint(ex + 40, ey + 30 + bob)
        };
        p.drawPolygon(wingL, 4);
        p.drawPolygon(wingR, 4);
        // Body
        p.setBrush(bodyColor);
        p.drawEllipse(ex + 4, ey + 10 + bob, 40, 32);
        // Head
        p.setBrush(bodyColor.lighter(120));
        p.drawEllipse(ex + 28, ey + 4 + bob, 24, 20);
        // Eyes
        p.setBrush(phase == 2 ? QColor(255, 50, 0) : QColor(255, 200, 0));
        p.drawEllipse(ex + 30, ey + 7 + bob, 5, 5);
        p.drawEllipse(ex + 40, ey + 7 + bob, 5, 5);
        // Horns
        p.setPen(QPen(QColor(80, 40, 20), 2));
        p.drawLine(ex + 32, ey + 4 + bob, ex + 28, ey - 6 + bob);
        p.drawLine(ex + 44, ey + 4 + bob, ex + 48, ey - 6 + bob);
        // Flame glow when enraged
        if (phase == 2)
        {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 120, 0, 80 + (int)(50*sin(animFrame*0.3))));
            p.drawEllipse(ex - 10, ey - 10 + bob, 68, 60);
        }
        p.setPen(Qt::NoPen);
        // Megafire charging glow
        if (dragon->getMegaFireCountdown() > 0)
        {
            int chargeAlpha = 120 + (int)(100 * sin(animFrame * 0.6));
            p.setBrush(QColor(255, 20, 0, chargeAlpha));
            p.drawEllipse(ex - 24, ey - 24 + bob, 96, 90);
        }
        // Health bar (big)
        drawHealthBar(p, ex - 12, ey - 18, tileSize + 24, 10, e->getHealth(), e->getMaxHealth(),
                      phase == 2 ? QColor(255, 60, 20) : QColor(200, 100, 20));
        break;
    }
    case EnemyType::PATROL:
    {
        // Armored castle guard
        p.setBrush(QColor(70, 70, 90));
        p.drawRoundedRect(ex + 10, ey + 14 + bob, 24, 26, 4, 4); // body/armor
        p.setBrush(QColor(160, 150, 130));
        p.drawEllipse(ex + 13, ey + 4 + bob, 18, 18);             // head
        p.setBrush(QColor(100, 100, 120));
        p.drawRoundedRect(ex + 11, ey + 2 + bob, 22, 14, 3, 3);   // helmet
        // Spear
        p.setPen(QPen(QColor(90, 60, 30), 2));
        p.drawLine(ex + 36, ey + bob, ex + 36, ey + 44 + bob);
        p.setBrush(QColor(190, 190, 190));
        p.setPen(Qt::NoPen);
        QPoint tip[] = { {ex+33, ey+2+bob}, {ex+39, ey+2+bob}, {ex+36, ey-7+bob} };
        p.drawPolygon(tip, 3);
        // Eyes
        p.setBrush(QColor(255, 60, 60));
        p.drawEllipse(ex + 16, ey + 9 + bob, 4, 4);
        p.drawEllipse(ex + 24, ey + 9 + bob, 4, 4);
        break;
    }
    }
}

// -------------------------------------------------------
// Projectiles
// -------------------------------------------------------

void GameView::drawProjectiles(QPainter &p, int ox, int oy, int tileSize)
{
    for (const Projectile &proj : game.getProjectiles())
    {
        if (!proj.active) continue;
        int px = ox + (int)(proj.x * tileSize) + tileSize / 2;
        int py = oy + (int)(proj.y * tileSize) + tileSize / 2;

        if (proj.type == "arrow")
        {
            p.setPen(QPen(QColor(160, 120, 60), 2));
            float angle = atan2(proj.dy, proj.dx);
            int ex = px + (int)(12 * cos(angle));
            int ey = py + (int)(12 * sin(angle));
            int sx = px - (int)(12 * cos(angle));
            int sy = py - (int)(12 * sin(angle));
            p.drawLine(sx, sy, ex, ey);
            // Tip
            p.setBrush(QColor(200, 180, 100));
            p.setPen(Qt::NoPen);
            p.drawEllipse(ex - 3, ey - 3, 6, 6);
        }
        else if (proj.type == "fireball")
        {
            int glow = 16 + (int)(8 * sin(animFrame * 0.5));
            QRadialGradient grad(px, py, glow);
            grad.setColorAt(0, QColor(255, 220, 50, 255));
            grad.setColorAt(0.5, QColor(255, 100, 0, 200));
            grad.setColorAt(1, QColor(255, 50, 0, 0));
            p.setPen(Qt::NoPen);
            p.setBrush(grad);
            p.drawEllipse(px - glow, py - glow, glow * 2, glow * 2);
        }
    }
}

// -------------------------------------------------------
// Riddle dialog
// -------------------------------------------------------

void GameView::drawRiddleDialog(QPainter &p)
{
    if (!game.isRiddleActive()) return;

    const int W = width(); const int H = height();
    const int dw = 600, dh = 380;
    const int dx = (W - dw) / 2, dy = (H - dh) / 2;

    // Dark overlay
    p.setBrush(QColor(0, 0, 0, 160));
    p.setPen(Qt::NoPen);
    p.drawRect(0, 52, W, H - 52);

    // Dialog box
    p.setBrush(QColor(28, 18, 42));
    p.setPen(QPen(QColor(180, 120, 220), 2));
    p.drawRoundedRect(dx, dy, dw, dh, 14, 14);

    // Title
    p.setPen(QColor(255, 200, 80));
    p.setFont(QFont("Georgia", 16, QFont::Bold));
    p.drawText(QRect(dx, dy + 16, dw, 30), Qt::AlignCenter, "Morgana's Riddle");

    // Riddle text
    p.setPen(QColor(235, 220, 195));
    p.setFont(QFont("Georgia", 12));
    p.drawText(QRect(dx + 30, dy + 56, dw - 60, 120),
               Qt::AlignHCenter | Qt::TextWordWrap, game.getRiddleQuestion());

    // Choices
    QVector<QString> choices = game.getRiddleChoices();
    for (int i = 0; i < choices.size(); ++i)
    {
        int cy = dy + 190 + i * 40;
        bool sel = (i == riddleSelected);
        p.setBrush(sel ? QColor(100, 60, 160) : QColor(50, 35, 70));
        p.setPen(sel ? QPen(QColor(255, 200, 80), 2) : QPen(QColor(120, 90, 150), 1));
        p.drawRoundedRect(dx + 60, cy, dw - 120, 32, 8, 8);
        p.setPen(sel ? QColor(255, 220, 100) : QColor(200, 180, 220));
        p.setFont(QFont("Georgia", 11, sel ? QFont::Bold : QFont::Normal));
        p.drawText(QRect(dx + 70, cy, dw - 140, 32), Qt::AlignVCenter, choices[i]);
    }

    // Instructions
    p.setPen(QColor(150, 130, 180));
    p.setFont(QFont("Georgia", 9));
    p.drawText(QRect(dx, dy + dh - 34, dw, 24), Qt::AlignCenter,
               "↑↓ select  |  Enter to answer  |  Attempts left: " +
                   QString::number(3 - game.getRiddleAttempts()));
}

// -------------------------------------------------------
// Level-specific renderers
// -------------------------------------------------------

void GameView::drawLevel1(QPainter &p, int tileSize, int ox, int oy)
{
    Level &lv = game.getLevel();

    // Sky gradient
    QLinearGradient sky(0, 0, 0, height());
    sky.setColorAt(0.0, QColor(23, 26, 50));
    sky.setColorAt(0.55, QColor(32, 58, 49));
    sky.setColorAt(1.0, QColor(35, 73, 47));
    p.fillRect(rect(), sky);

    // Forest clearing base
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(65, 102, 58));
    p.drawRoundedRect(ox - 14, oy - 14, lv.gridSize * tileSize + 28, lv.gridSize * tileSize + 28, 14, 14);

    for (int x = 0; x < lv.gridSize; ++x)
        for (int y = 0; y < lv.gridSize; ++y)
        {
            QRect tile = tileRect(x, y, tileSize, ox, oy);
            int kind = lv.map[x][y];

            if (kind == Level::RIVER)
            {
                p.setBrush(QColor(40 + (int)(10*sin(animFrame*0.05+y)), 101, 177));
                p.setPen(Qt::NoPen);
                p.drawRect(tile);
                p.setPen(QPen(QColor(74, 139, 214, 140), 2));
                p.drawLine(tile.left()+4, tile.center().y(), tile.right()-4, tile.center().y()-3);
                p.setPen(Qt::NoPen);
            }
            else if (kind == Level::TREE)
            {
                p.setBrush(QColor(27, 66, 37));
                p.drawEllipse(tile.adjusted(8, 5, -8, -18));
                p.drawEllipse(tile.adjusted(14, 2, -12, -20));
                p.setBrush(QColor(89, 59, 38));
                p.drawRoundedRect(tile.x()+tile.width()/2-4, tile.y()+tile.height()/2, 8, 18, 3, 3);
            }
            else if (kind == Level::PATH)
            {
                p.setBrush(QColor(126, 107, 79));
                p.drawRoundedRect(tile.adjusted(6, 12, -6, -12), 8, 8);
            }
            else if (kind == Level::BRIDGE)
            {
                p.setBrush(QColor(138, 95, 60));
                p.drawRoundedRect(tile.adjusted(2, 6, -2, -6), 6, 6);
                p.setPen(QPen(QColor(95, 64, 38), 2));
                for (int i = 8; i <= 38; i += 8)
                    p.drawLine(tile.left()+i, tile.top()+8, tile.left()+i, tile.bottom()-8);
                p.setPen(Qt::NoPen);
            }
        }

    // Lanterns
    const QPoint lanternTiles[] = { {2,8},{4,8},{5,7},{5,6} };
    for (auto &lt : lanternTiles)
    {
        int lx = ox + lt.x()*tileSize + 22;
        int ly = oy + lt.y()*tileSize + 16;
        p.setPen(QPen(QColor(76, 55, 40), 2));
        p.drawLine(lx, ly, lx, ly+18);
        p.setBrush(QColor(255, 210, 118));
        p.setPen(Qt::NoPen);
        p.drawEllipse(lx-5, ly-2, 10, 10);
        int glow = 14 + (int)(4*sin(animFrame*0.12));
        p.setBrush(QColor(255, 206, 120, 55));
        p.drawEllipse(lx-glow, ly-glow+4, glow*2, glow*2);
    }

    // Cottage
    QRect cottageBase(ox+7*tileSize+4, oy+2*tileSize+10, tileSize*2-8, tileSize*2-14);
    p.setBrush(QColor(168, 130, 94)); p.setPen(Qt::NoPen);
    p.drawRoundedRect(cottageBase, 7, 7);
    QPoint roof[3] = {
        QPoint(cottageBase.left()-10, cottageBase.top()+6),
        QPoint(cottageBase.right()+10, cottageBase.top()+6),
        QPoint(cottageBase.center().x(), cottageBase.top()-28)
    };
    p.setBrush(QColor(118, 66, 48)); p.drawPolygon(roof, 3);
    p.setBrush(QColor(109, 74, 47));
    p.drawRoundedRect(cottageBase.center().x()-10, cottageBase.bottom()-28, 20, 28, 4, 4);
    p.setBrush(QColor(241, 225, 164));
    p.drawRect(cottageBase.left()+10, cottageBase.top()+16, 14, 12);
    p.drawRect(cottageBase.right()-24, cottageBase.top()+16, 14, 12);
    p.setBrush(QColor(96, 96, 96));
    p.drawRect(cottageBase.right()-6, cottageBase.top()-18, 10, 20);
    // Warm window glow
    p.setBrush(QColor(255, 200, 80, 80 + (int)(30*sin(animFrame*0.1))));
    p.drawRect(cottageBase.left()+10, cottageBase.top()+16, 14, 12);
    p.drawRect(cottageBase.right()-24, cottageBase.top()+16, 14, 12);
}

void GameView::drawLevel2(QPainter &p, int tileSize, int ox, int oy)
{
    Level &lv = game.getLevel();

    // Warm candlelit interior
    QLinearGradient bg(0, 52, 0, height());
    bg.setColorAt(0, QColor(45, 25, 15));
    bg.setColorAt(1, QColor(25, 12, 8));
    p.fillRect(0, 52, width(), height()-52, bg);

    for (int x = 0; x < lv.gridSize; ++x)
        for (int y = 0; y < lv.gridSize; ++y)
        {
            QRect tile = tileRect(x, y, tileSize, ox, oy);
            int kind = lv.map[x][y];

            if (kind == Level::FLOOR)
            {
                // Stone floor pattern
                p.setBrush(QColor(70 + (x+y)%2*10, 55 + (x+y)%2*8, 40));
                p.setPen(QPen(QColor(50,38,28), 1));
                p.drawRect(tile);
            }
            else if (kind == Level::WALL)
            {
                p.setBrush(QColor(90, 70, 55));
                p.setPen(Qt::NoPen);
                p.drawRect(tile);
                // Stone texture
                p.setBrush(QColor(75, 58, 44));
                p.drawRect(tile.adjusted(2,2,-2,-6));
            }
            else if (kind == Level::DOOR)
            {
                p.setBrush(QColor(110, 75, 45));
                p.setPen(Qt::NoPen);
                p.drawRect(tile);
                p.setBrush(QColor(180, 140, 80));
                p.drawEllipse(tile.center().x()-3, tile.center().y()-3, 6, 6);
            }
            else if (kind == Level::CHEST)
            {
                p.setBrush(QColor(120, 80, 40));
                p.setPen(QPen(QColor(200, 160, 60), 1));
                p.drawRoundedRect(tile.adjusted(4,8,-4,-8), 4, 4);
                p.setBrush(QColor(200, 160, 60));
                p.drawRect(tile.adjusted(8,12,-8,4));
            }
            else if (kind == Level::STAIRS)
            {
                p.setBrush(QColor(60, 80, 110));
                p.setPen(Qt::NoPen);
                p.drawRect(tile);
                // Draw stairs symbol
                p.setPen(QPen(QColor(120, 160, 200), 2));
                for (int s = 0; s < 4; ++s)
                    p.drawLine(tile.left()+4+s*6, tile.bottom()-4-s*6,
                               tile.right()-4, tile.bottom()-4-s*6);
            }
        }

    // Candles on shelves
    const QPoint candles[] = {{2,2},{9,2},{9,8}};
    for (auto &c : candles)
    {
        int cx = ox+c.x()*tileSize+tileSize/2;
        int cy = oy+c.y()*tileSize+tileSize/3;
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(230,220,200));
        p.drawRoundedRect(cx-3, cy, 6, 14, 2, 2);
        // Flame flicker
        int fl = 5 + (int)(2*sin(animFrame*0.3+c.x()));
        QRadialGradient flame(cx, cy-fl, fl);
        flame.setColorAt(0, QColor(255,220,80,240));
        flame.setColorAt(1, QColor(255,100,0,0));
        p.setBrush(flame);
        p.drawEllipse(cx-fl, cy-fl*2, fl*2, fl*2);
    }
}

void GameView::drawLevel3(QPainter &p, int tileSize, int ox, int oy)
{
    Level &lv = game.getLevel();

    // Dramatic twilight sky
    QLinearGradient sky(0,52,0,height());
    sky.setColorAt(0, QColor(20,10,40));
    sky.setColorAt(0.4, QColor(80,30,80));
    sky.setColorAt(1, QColor(40,20,10));
    p.fillRect(0, 52, width(), height()-52, sky);

    // Ground base
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(50,45,35));
    p.drawRect(ox-10, oy-10, lv.gridSize*tileSize+20, lv.gridSize*tileSize+20);

    for (int x = 0; x < lv.gridSize; ++x)
        for (int y = 0; y < lv.gridSize; ++y)
        {
            QRect tile = tileRect(x, y, tileSize, ox, oy);
            int kind = lv.map[x][y];

            if (kind == Level::GRASS)
            {
                p.setBrush(QColor(45 + (x+y)%3*5, 55, 30));
                p.setPen(Qt::NoPen);
                p.drawRect(tile);
            }
            else if (kind == Level::WALL)
            {
                // Castle stone wall
                p.setBrush(QColor(85, 78, 68));
                p.setPen(Qt::NoPen);
                p.drawRect(tile);
                p.setBrush(QColor(70, 64, 56));
                for (int s = 0; s < 3; ++s)
                    p.drawRect(tile.adjusted(1+s*14, 1, -(tileSize-14-s*14), -tileSize+10));
            }
            else if (kind == Level::TREE)
            {
                p.setBrush(QColor(30, 50, 25));
                p.drawEllipse(tile.adjusted(6,4,-6,-16));
                p.setBrush(QColor(70, 50, 30));
                p.drawRoundedRect(tile.x()+tile.width()/2-4, tile.y()+tile.height()/2, 8, 18, 3, 3);
            }
            else if (kind == Level::PATH)
            {
                p.setBrush(QColor(110, 95, 70));
                p.setPen(Qt::NoPen);
                p.drawRect(tile);
            }
            else if (kind == Level::STAIRS)
            {
                // Castle gate entrance
                p.setBrush(QColor(40, 30, 20));
                p.setPen(QPen(QColor(120,90,60), 2));
                p.drawRect(tile);
                p.setPen(Qt::NoPen);
                p.setBrush(QColor(200, 160, 80, 150));
                p.drawEllipse(tile.adjusted(4, 2, -4, -4));
                // Portcullis bars
                p.setPen(QPen(QColor(80,80,80), 2));
                for (int b = 0; b < 3; ++b)
                    p.drawLine(tile.left()+8+b*12, tile.top()+4, tile.left()+8+b*12, tile.bottom()-4);
                p.setPen(Qt::NoPen);
            }
        }

    // Castle battlements decoration at top
    for (int x = 4; x <= 13; ++x)
    {
        int bx = ox + x*tileSize;
        int by = oy - 12;
        if (x%2==0)
        {
            p.setBrush(QColor(85, 78, 68));
            p.setPen(Qt::NoPen);
            p.drawRect(bx+2, by, tileSize-4, 14);
        }
    }

    // Torches on wall
    for (int wx = 5; wx <= 12; wx += 3)
    {
        int tx = ox + wx*tileSize + tileSize/2;
        int ty = oy + 2*tileSize + 8;
        p.setPen(QPen(QColor(80,60,30), 2));
        p.drawLine(tx, ty, tx, ty+14);
        int fl = 5 + (int)(2*sin(animFrame*0.25+wx));
        QRadialGradient flame(tx, ty, fl+4);
        flame.setColorAt(0, QColor(255,200,60,230));
        flame.setColorAt(1, QColor(255,60,0,0));
        p.setBrush(flame); p.setPen(Qt::NoPen);
        p.drawEllipse(tx-fl, ty-fl, fl*2, fl*2);
    }

    // Archer range indicators (semi-transparent lines)
    for (const Enemy *e : game.getEnemies())
    {
        if (e->isDefeated() || e->getType() != EnemyType::ARCHER) continue;
        const ArcherEnemy *a = static_cast<const ArcherEnemy*>(e);
        int ex = ox + e->getX()*tileSize + tileSize/2;
        int ey = oy + e->getY()*tileSize + tileSize/2;
        p.setPen(QPen(QColor(255, 80, 80, 50 + (int)(20*sin(animFrame*0.2))), 1, Qt::DashLine));
        p.drawLine(ex, ey, ex, ey + a->getRange()*tileSize);
        p.drawLine(ex - a->getRange()*tileSize, ey, ex + a->getRange()*tileSize, ey);
        p.setPen(Qt::NoPen);
    }
}

// void GameView::drawLevel4(QPainter &p, int tileSize, int ox, int oy)
// {
//     Level &lv = game.getLevel();

//     // Dark stone dungeon atmosphere
//     QLinearGradient bg(0,52,width(),height());
//     bg.setColorAt(0, QColor(15,12,20));
//     bg.setColorAt(1, QColor(25,18,10));
//     p.fillRect(0, 52, width(), height()-52, bg);

//     for (int x = 0; x < lv.gridSize; ++x)
//         for (int y = 0; y < lv.gridSize; ++y)
//         {
//             QRect tile = tileRect(x, y, tileSize, ox, oy);
//             int kind = lv.map[x][y];

//             if (kind == Level::FLOOR)
//             {
//                 int shade = 45 + (x+y)%2*8;
//                 p.setBrush(QColor(shade, shade-5, shade-10));
//                 p.setPen(QPen(QColor(30,25,20), 1));
//                 p.drawRect(tile);
//             }
//             else if (kind == Level::WALL)
//             {
//                 p.setBrush(QColor(75, 68, 60));
//                 p.setPen(Qt::NoPen);
//                 p.drawRect(tile);
//                 p.setBrush(QColor(62, 56, 50));
//                 p.drawRect(tile.adjusted(1,1,-1,-tileSize/2));
//             }
//             else if (kind == Level::LOCKED)
//             {
//                 p.setBrush(QColor(100, 60, 20));
//                 p.setPen(QPen(QColor(200, 140, 40), 2));
//                 p.drawRect(tile);
//                 // Lock icon
//                 p.setBrush(QColor(200, 160, 50));
//                 p.setPen(Qt::NoPen);
//                 p.drawRoundedRect(tile.center().x()-6, tile.center().y()-2, 12, 10, 3, 3);
//                 p.setBrush(Qt::NoBrush);
//                 p.setPen(QPen(QColor(200, 160, 50), 2));
//                 p.drawArc(tile.center().x()-5, tile.center().y()-9, 10, 10, 0, 180*16);
//             }
//             else if (kind == Level::DOOR)
//             {
//                 p.setBrush(QColor(100, 70, 40));
//                 p.setPen(QPen(QColor(160, 120, 60), 1));
//                 p.drawRect(tile);
//             }
//             else if (kind == Level::KEY_TILE)
//             {
//                 p.setBrush(QColor(45, 40, 35));
//                 p.setPen(Qt::NoPen);
//                 p.drawRect(tile);
//                 // Key sparkle
//                 float glow = 4 + 2*sin(animFrame*0.2 + x*0.5);
//                 p.setBrush(QColor(255, 210, 50, 200));
//                 p.drawEllipse(tile.center().x()-(int)glow, tile.center().y()-(int)glow, (int)glow*2, (int)glow*2);
//                 p.setBrush(QColor(255, 240, 120, 120));
//                 p.drawEllipse(tile.center().x()-(int)glow*2, tile.center().y()-(int)glow*2, (int)glow*4, (int)glow*4);
//             }
//             else if (kind == Level::CHEST)
//             {
//                 p.setBrush(QColor(100, 70, 30));
//                 p.setPen(QPen(QColor(180, 140, 50), 1));
//                 p.drawRoundedRect(tile.adjusted(3,8,-3,-8), 4, 4);
//                 p.setBrush(QColor(190, 150, 55));
//                 p.drawRect(tile.adjusted(7,12,-7,4));
//             }
//             else if (kind == Level::STAIRS)
//             {
//                 p.setBrush(QColor(40, 50, 70));
//                 p.setPen(Qt::NoPen);
//                 p.drawRect(tile);
//                 p.setPen(QPen(QColor(80, 100, 140), 2));
//                 for (int s = 0; s < 4; ++s)
//                     p.drawLine(tile.left()+3+s*6, tile.bottom()-4-s*6,
//                                tile.right()-3, tile.bottom()-4-s*6);
//             }
//         }

//     // Mini-map legend
//     p.setBrush(QColor(10, 8, 15, 200));
//     p.setPen(QPen(QColor(80, 60, 100), 1));
//     p.drawRoundedRect(width()-180, 60, 168, 100, 6, 6);
//     p.setPen(QColor(160, 140, 200));
//     p.setFont(QFont("Georgia", 8, QFont::Bold));
//     p.drawText(width()-170, 74, "MAP LEGEND");
//     p.setFont(QFont("Georgia", 8));
//     p.drawText(width()-170, 90, "🗝 Key tiles");
//     p.drawText(width()-170, 105, "🔒 Locked doors");
//     p.drawText(width()-170, 120, "⚡ Hidden traps");
//     p.drawText(width()-170, 135, "⬇ Stairs (goal)");
//     // Key count
//     p.setPen(QColor(255, 210, 80));
//     p.setFont(QFont("Georgia", 9, QFont::Bold));
//     p.drawText(width()-170, 154, "Keys in hand: " + QString::number(game.getPlayer().getKeys()) + " / 4");
// }

void GameView::drawLevel4(QPainter &p, int tileSize, int ox, int oy)
{
    Level &lv = game.getLevel();

    // Dark stone atmosphere
    QLinearGradient bg(0, 52, 0, height());
    bg.setColorAt(0, QColor(15, 12, 20));
    bg.setColorAt(1, QColor(25, 18, 10));
    p.fillRect(0, 52, width(), height() - 52, bg);

    // Alarm: pulsing red border overlay
    if (game.isAlarmActive()) {
        int alpha = 40 + (int)(30 * sin(animFrame * 0.25));
        p.setBrush(QColor(200, 20, 0, alpha));
        p.setPen(Qt::NoPen);
        p.drawRect(0, 52, width(), height() - 52);
    }

    // Tiles
    for (int x = 0; x < lv.gridSize; ++x)
        for (int y = 0; y < lv.gridSize; ++y)
        {
            QRect tile = tileRect(x, y, tileSize, ox, oy);
            int kind = lv.map[x][y];

            if (kind == Level::FLOOR)
            {
                int shade = 45 + (x + y) % 2 * 8;
                p.setBrush(QColor(shade, shade - 5, shade - 10));
                p.setPen(QPen(QColor(30, 25, 20), 1));
                p.drawRect(tile);
            }
            else if (kind == Level::WALL)
            {
                p.setBrush(QColor(75, 68, 60));
                p.setPen(Qt::NoPen);
                p.drawRect(tile);
                p.setBrush(QColor(62, 56, 50));
                p.drawRect(tile.adjusted(1, 1, -1, -tileSize / 2));
            }
            else if (kind == Level::KEY_TILE)
            {
                p.setBrush(QColor(45, 40, 35));
                p.setPen(Qt::NoPen);
                p.drawRect(tile);
                int kx = tile.center().x() - 2, ky = tile.center().y();
                // Subtle glow
                p.setBrush(QColor(255, 210, 50, (int)(40 + 25*sin(animFrame*0.2 + x*0.5f))));
                p.drawEllipse(kx - 14, ky - 10, 30, 22);
                // Key shape
                p.setPen(QPen(QColor(255, 200, 50), 2));
                p.setBrush(QColor(255, 200, 50, 60));
                p.drawEllipse(kx - 11, ky - 5, 10, 10); // bow (ring)
                p.setBrush(Qt::NoBrush);
                p.drawLine(kx - 6, ky, kx + 11, ky);    // shaft
                p.drawLine(kx + 3, ky, kx + 3, ky + 5); // tooth 1
                p.drawLine(kx + 6, ky, kx + 6, ky + 4); // tooth 2
                p.drawLine(kx + 9, ky, kx + 9, ky + 3); // tooth 3
            }
            else if (kind == Level::STAIRS)
            {
                p.setBrush(QColor(40, 50, 70));
                p.setPen(Qt::NoPen);
                p.drawRect(tile);
                p.setPen(QPen(QColor(80, 100, 140), 2));
                for (int s = 0; s < 4; ++s)
                    p.drawLine(tile.left() + 3 + s * 6, tile.bottom() - 4 - s * 6,
                               tile.right() - 3, tile.bottom() - 4 - s * 6);
                p.setPen(Qt::NoPen);
            }
        }

    // Wall torches at room entrances
    const QPoint torchTiles[] = { {4,3}, {4,10}, {10,3}, {10,10} };
    for (auto &t : torchTiles) {
        int tx = ox + t.x() * tileSize + tileSize / 2;
        int ty = oy + t.y() * tileSize + 8;
        p.setPen(QPen(QColor(80, 60, 30), 2));
        p.drawLine(tx, ty, tx, ty + 12);
        int fl = 5 + (int)(2 * sin(animFrame * 0.28 + t.x()));
        QRadialGradient flame(tx, ty, fl + 4);
        flame.setColorAt(0, QColor(255, 200, 60, 230));
        flame.setColorAt(1, QColor(255, 60, 0, 0));
        p.setBrush(flame);
        p.setPen(Qt::NoPen);
        p.drawEllipse(tx - fl, ty - fl, fl * 2, fl * 2);
    }

    // Legend
    p.setBrush(QColor(10, 8, 15, 200));
    p.setPen(QPen(QColor(80, 60, 100), 1));
    p.drawRoundedRect(width() - 175, 60, 163, 80, 6, 6);
    p.setPen(QColor(160, 140, 200));
    p.setFont(QFont("Georgia", 8, QFont::Bold));
    p.drawText(width() - 165, 75, "MAP LEGEND");
    p.setFont(QFont("Georgia", 8));
    p.drawText(width() - 165, 90,  "🗝 Key (center room)");
    p.drawText(width() - 165, 105, "⚡ Hidden traps");
    p.drawText(width() - 165, 120, "⬇ Stairs (goal, right)");
    p.setPen(QColor(255, 210, 80));
    p.setFont(QFont("Georgia", 9, QFont::Bold));
    p.drawText(width() - 165, 135, "Keys held: " +
                                       QString::number(game.getPlayer().getKeys()));
}

void GameView::drawLevel5(QPainter &p, int tileSize, int ox, int oy)
{
    Level &lv = game.getLevel();

    // Deep dungeon — dark with red-orange haze
    QLinearGradient bg(0,52,0,height());
    bg.setColorAt(0, QColor(30, 10, 5));
    bg.setColorAt(1, QColor(10, 5, 20));
    p.fillRect(0, 52, width(), height()-52, bg);

    // Dragon phase atmosphere
    bool enraged = false;
    for (const Enemy *e : game.getEnemies())
        if (e->getType() == EnemyType::DRAGON && !e->isDefeated())
        {
            enraged = static_cast<const DragonEnemy*>(e)->getPhase() == 2;
            break;
        }
    if (enraged)
    {
        int alpha = 30 + (int)(20*sin(animFrame*0.1));
        p.setBrush(QColor(200, 40, 0, alpha));
        p.setPen(Qt::NoPen);
        p.drawRect(0, 52, width(), height()-52);
    }

    for (int x = 0; x < lv.gridSize; ++x)
        for (int y = 0; y < lv.gridSize; ++y)
        {
            QRect tile = tileRect(x, y, tileSize, ox, oy);
            int kind = lv.map[x][y];

            if (kind == Level::FLOOR)
            {
                int shade = 38 + (x+y)%2*6;
                p.setBrush(QColor(shade+10, shade-2, shade-8));
                p.setPen(QPen(QColor(25,18,14), 1));
                p.drawRect(tile);
            }
            else if (kind == Level::WALL)
            {
                p.setBrush(QColor(65, 55, 45));
                p.setPen(Qt::NoPen);
                p.drawRect(tile);
                p.setBrush(QColor(52, 44, 36));
                p.drawRect(tile.adjusted(2,2,-2,-tileSize/2));
            }
            else if (kind == Level::FIRE)
            {
                // Animated fire pit
                p.setBrush(QColor(40, 20, 10));
                p.setPen(Qt::NoPen);
                p.drawRect(tile);
                for (int f = 0; f < 3; ++f)
                {
                    float phase = animFrame * 0.3f + f * 2.1f;
                    int fw = 8 + (int)(4*sin(phase));
                    int fh = 10 + (int)(5*sin(phase+1));
                    int fx = tile.center().x() - fw/2 + (int)(4*sin(phase+f));
                    int fy = tile.center().y() - fh/2;
                    QRadialGradient fg(fx+fw/2, fy+fh, fw);
                    fg.setColorAt(0, QColor(255, 200, 50, 220));
                    fg.setColorAt(0.5, QColor(255, 80, 0, 160));
                    fg.setColorAt(1, QColor(255, 40, 0, 0));
                    p.setBrush(fg);
                    p.drawEllipse(fx, fy, fw, fh);
                }
            }
            else if (kind == Level::LOCKED)
            {
                p.setBrush(QColor(60, 30, 15));
                p.setPen(QPen(QColor(160, 100, 30), 2));
                p.drawRect(tile);
                p.setBrush(QColor(160, 110, 40));
                p.setPen(Qt::NoPen);
                p.drawRoundedRect(tile.center().x()-6, tile.center().y()-2, 12, 10, 3, 3);
                p.setBrush(Qt::NoBrush);
                p.setPen(QPen(QColor(160, 110, 40), 2));
                p.drawArc(tile.center().x()-5, tile.center().y()-9, 10, 10, 0, 180*16);
                p.setPen(Qt::NoPen);
            }
            else if (kind == Level::DOOR)
            {
                p.setBrush(QColor(80, 55, 30));
                p.setPen(QPen(QColor(120, 90, 45), 1));
                p.drawRect(tile);
            }
            else if (kind == Level::KEY_TILE)
            {
                p.setBrush(QColor(38, 30, 22));
                p.setPen(Qt::NoPen);
                p.drawRect(tile);
                int kx = tile.center().x() - 2, ky = tile.center().y();
                // Subtle glow
                p.setBrush(QColor(255, 200, 40, (int)(40 + 25*sin(animFrame*0.18))));
                p.drawEllipse(kx - 14, ky - 10, 30, 22);
                // Key shape
                p.setPen(QPen(QColor(255, 200, 50), 2));
                p.setBrush(QColor(255, 200, 50, 60));
                p.drawEllipse(kx - 11, ky - 5, 10, 10); // bow
                p.setBrush(Qt::NoBrush);
                p.drawLine(kx - 6, ky, kx + 11, ky);    // shaft
                p.drawLine(kx + 3, ky, kx + 3, ky + 5); // tooth 1
                p.drawLine(kx + 6, ky, kx + 6, ky + 4); // tooth 2
                p.drawLine(kx + 9, ky, kx + 9, ky + 3); // tooth 3
            }
            else if (kind == Level::SWORD_TILE)
            {
                int shade = 38 + (x+y)%2*6;
                p.setBrush(QColor(shade+10, shade-2, shade-8));
                p.setPen(QPen(QColor(25, 18, 14), 1));
                p.drawRect(tile);
                int cx2 = tile.center().x(), cy2 = tile.center().y();
                // Glow beneath sword
                p.setPen(Qt::NoPen);
                float sg = 5 + 3*sin(animFrame*0.15f);
                p.setBrush(QColor(255, 215, 0, (int)(40 + 30*sin(animFrame*0.15))));
                p.drawEllipse(cx2-(int)(sg*2.5f), cy2-(int)(sg*2.5f), (int)(sg*5), (int)(sg*5));
                // Sword drawn with rotation
                p.save();
                p.translate(cx2, cy2);
                p.rotate(-45);
                p.setPen(Qt::NoPen);
                // Blade body
                QPoint blade[] = { QPoint(-2,-4), QPoint(2,-4), QPoint(2,-16), QPoint(-2,-16) };
                p.setBrush(QColor(210, 220, 200));
                p.drawPolygon(blade, 4);
                // Blade tip
                QPoint tip[] = { QPoint(-2,-16), QPoint(2,-16), QPoint(0,-21) };
                p.drawPolygon(tip, 3);
                // Crossguard
                p.setBrush(QColor(180, 130, 40));
                p.drawRoundedRect(-8, -4, 16, 3, 1, 1);
                // Handle
                p.setBrush(QColor(110, 65, 25));
                p.drawRoundedRect(-2, -1, 4, 9, 1, 1);
                // Pommel
                p.setBrush(QColor(200, 160, 40));
                p.drawEllipse(-4, 7, 8, 8);
                p.restore();
            }
            else if (kind == Level::CELL)
            {
                // Prison cell bars
                p.setBrush(QColor(35, 28, 20));
                p.setPen(Qt::NoPen);
                p.drawRect(tile);
                p.setPen(QPen(QColor(90, 80, 60), 3));
                for (int b = 1; b <= 3; ++b)
                    p.drawLine(tile.left()+b*tileSize/4, tile.top()+2,
                               tile.left()+b*tileSize/4, tile.bottom()-2);
                p.setPen(Qt::NoPen);
                // Friend silhouette
                if (x == 12 && y == 6)  // friend shown once, in centre cell tile
                {
                    p.setBrush(QColor(140, 110, 80, 180));
                    p.drawEllipse(tile.center().x()-8, tile.top()+4, 14, 14);
                    p.drawRoundedRect(tile.center().x()-8, tile.top()+18, 14, 18, 4, 4);
                }
            }
        }

    // Wall torches — left wall only (x=1 column)
    for (int ty_idx = 2; ty_idx <= 10; ty_idx += 4)
    {
        int tx = ox + 1 * tileSize + tileSize / 2;
        int ty = oy + ty_idx * tileSize + tileSize / 3;
        p.setPen(QPen(QColor(70, 50, 25), 2));
        p.drawLine(tx, ty, tx, ty + 12);
        int fl = 5 + (int)(2 * sin(animFrame * 0.22 + ty_idx));
        QRadialGradient flame(tx, ty, fl + 4);
        flame.setColorAt(0, QColor(255, 180, 50, 230));
        flame.setColorAt(1, QColor(255, 50, 0, 0));
        p.setBrush(flame);
        p.setPen(Qt::NoPen);
        p.drawEllipse(tx - fl, ty - fl, fl * 2, fl * 2);
    }

    // Dragon defeated message overlay
    if (game.isCellUnlocked())
    {
        p.setBrush(QColor(200, 240, 150, 60));
        p.setPen(Qt::NoPen);
        p.drawRect(0, 52, width(), height()-52);
    }

    // Megafire warning overlay (2-turn telegraph)
    if (game.isMegaFireWarning())
    {
        QPoint center = game.getMegaFireWarningCenter();
        int pulse = 80 + (int)(70 * sin(animFrame * 0.5));
        p.setPen(QPen(QColor(255, 80, 0, 200), 2));
        for (int ddx = -1; ddx <= 1; ddx++)
            for (int ddy = -1; ddy <= 1; ddy++)
            {
                p.setBrush(QColor(255, 50, 0, pulse));
                p.drawRect(tileRect(center.x()+ddx, center.y()+ddy, tileSize, ox, oy));
            }
    }
}

// -------------------------------------------------------
// Main paint event
// -------------------------------------------------------

void GameView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const int tileSize = 44;
    Game::Phase phase = game.getPhase();

    // Calculate map offset to center it
    int mapW = game.getLevel().gridSize * tileSize;
    int mapH = game.getLevel().gridSize * tileSize;
    int ox = (width() - mapW) / 2;
    int oy = 60 + (height() - 60 - mapH) / 2;

    // Draw level-specific background and tiles
    switch (phase)
    {
    case Game::Phase::LEVEL1_EXPLORE:
        drawLevel1(p, tileSize, ox, oy);
        break;
    case Game::Phase::LEVEL2_RIDDLE:
    case Game::Phase::LEVEL2_COMBAT:
        drawLevel2(p, tileSize, ox, oy);
        break;
    case Game::Phase::LEVEL3_INFILTRATE:
        drawLevel3(p, tileSize, ox, oy);
        break;
    case Game::Phase::LEVEL4_NAVIGATE:
        drawLevel4(p, tileSize, ox, oy);
        break;
    case Game::Phase::LEVEL5_DRAGON:
        drawLevel5(p, tileSize, ox, oy);
        break;
    default:
        p.fillRect(rect(), QColor(10, 8, 20));
    }

    // Draw enemies
    for (const Enemy *e : game.getEnemies())
        drawEnemy(p, e, ox, oy, tileSize);

    // Draw projectiles
    drawProjectiles(p, ox, oy, tileSize);

    // Draw flash events
    drawFlashEvents(p, ox, oy, tileSize);

    // Draw player
    const int px = ox + game.getPlayer().getX() * tileSize;
    const int py = oy + game.getPlayer().getY() * tileSize;
    drawPlayer(p, px, py, tileSize);

    // Intro dialog (level 1)
    if (showIntroDialog && phase == Game::Phase::LEVEL1_EXPLORE && introTimer > 0)
    {
        const Enemy *shadow = nullptr;
        if (!game.getEnemies().isEmpty()) shadow = game.getEnemies().first();

        p.setBrush(QColor(255, 248, 230, 220));
        p.setPen(QPen(QColor(70, 55, 45), 1));
        p.drawRoundedRect(QRect(px - 100, py - 68, 200, 48), 8, 8);
        p.setFont(QFont("Georgia", 9));
        p.setPen(QColor(40, 30, 20));
        p.drawText(QRect(px - 90, py - 64, 180, 40), Qt::TextWordWrap,
                   playerName + ": I must reach the cottage!");

        if (shadow)
        {
            int ex = ox + shadow->getX() * tileSize;
            int ey = oy + shadow->getY() * tileSize;
            p.setBrush(QColor(40, 20, 48, 220));
            p.setPen(QPen(QColor(176, 120, 176), 1));
            p.drawRoundedRect(QRect(ex - 40, ey - 62, 190, 44), 8, 8);
            p.setPen(QColor(240, 215, 255));
            p.drawText(QRect(ex - 30, ey - 58, 170, 36), Qt::TextWordWrap,
                       "Shadow: You'll never escape me...");
        }
    }

    // Riddle dialog (fullscreen overlay)
    if (phase == Game::Phase::LEVEL2_RIDDLE)
        drawRiddleDialog(p);

    // Story banner
    drawStoryBanner(p);

    // HUD on top
    drawHUD(p);
}

// -------------------------------------------------------
// Key input
// -------------------------------------------------------

void GameView::keyPressEvent(QKeyEvent *event)
{
    Game::Phase phase = game.getPhase();

    // Riddle navigation
    if (phase == Game::Phase::LEVEL2_RIDDLE)
    {
        int n = game.getRiddleChoices().size();
        if (event->key() == Qt::Key_Up)   riddleSelected = (riddleSelected - 1 + n) % n;
        if (event->key() == Qt::Key_Down) riddleSelected = (riddleSelected + 1) % n;
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Space)
            game.answerRiddle(riddleSelected);
        update();
        return;
    }

    int dx = 0, dy = 0;
    if (event->key() == Qt::Key_Up)    dy = -1;
    if (event->key() == Qt::Key_Down)  dy =  1;
    if (event->key() == Qt::Key_Left)  dx = -1;
    if (event->key() == Qt::Key_Right) dx =  1;

    // Attack
    if (event->key() == Qt::Key_Space)
    {
        game.playerAttack();
        game.updateEnemies();
        update();
        checkStateTransitions();
        return;
    }

    // Potion
    if (event->key() == Qt::Key_P)
    {
        game.usePotion();
        update();
        return;
    }

    if (dx != 0 || dy != 0)
    {
        showIntroDialog = false;
        game.movePlayer(dx, dy);
        // // Level 4: start trap-hide timer once on first move
        // if (game.getCurrentLevel() == 4 && !trapHideTimer)
        // {
        //     trapHideTimer = new QTimer(this);
        //     trapHideTimer->setSingleShot(true);
        //     connect(trapHideTimer, &QTimer::timeout, this, [this]() {
        //         for (Enemy *e : game.getEnemies())
        //             if (e->getType() == EnemyType::TRAP)
        //                 static_cast<TrapEnemy*>(e)->hide();
        //         trapHideTimer = nullptr;
        //         update();
        //     });
        //     trapHideTimer->start(10000); // 10 seconds
        // }

        if ((game.getCurrentLevel() == 4 || game.getCurrentLevel() == 5) && !trapHideTimer)
        {
            int hideDelay = (game.getCurrentLevel() == 5) ? 5000 : 10000;
            trapHideTimer = new QTimer(this);
            trapHideTimer->setSingleShot(true);
            connect(trapHideTimer, &QTimer::timeout, this, [this]() {
                for (Enemy *e : game.getEnemies())
                    if (e->getType() == EnemyType::TRAP)
                        static_cast<TrapEnemy*>(e)->hide();
                trapHideTimer = nullptr;
                update();
            });
            trapHideTimer->start(hideDelay);
        }

        // Level 4: start 5-second alarm countdown the moment alarm fires
        if (game.isAlarmActive() && !l4AlarmTimerStarted)
        {
            l4AlarmTimerStarted = true;
            alarmSecondsLeft = 5;
            alarmCountdownTimer = new QTimer(this);
            connect(alarmCountdownTimer, &QTimer::timeout, this, [this]() {
                alarmSecondsLeft--;
                if (alarmSecondsLeft <= 0) {
                    alarmCountdownTimer->stop();
                    alarmCountdownTimer->deleteLater();
                    alarmCountdownTimer = nullptr;
                    l4AlarmTimerStarted = false;
                    emit gameLost();
                }
                update();
            });
            alarmCountdownTimer->start(1000); // fires every second
        }
        game.updateEnemies();

        checkStateTransitions();
        update();
    }
}

void GameView::checkStateTransitions()
{
    if (game.checkLose())
    {
        emit gameLost();
        return;
    }
    if (game.checkWin())
    {
        if (game.getCurrentLevel() >= 5)
            emit gameWon();
        else
        {
            emit levelComplete(game.getCurrentLevel());
            game.advanceToNextLevel();
            riddleSelected = 0;
            showIntroDialog = true;
            introTimer = 200;
        }
    }
}
