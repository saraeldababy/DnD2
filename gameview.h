#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include "game.h"

class GameView : public QWidget
{
    Q_OBJECT

private:
    Game game;
    QString playerName;
    bool showIntroDialog;
    int introTimer;

    // Riddle UI state
    int riddleSelected; // currently highlighted choice

    // Animation timer
    QTimer *animTimer;
    int animFrame;

    // add to private:
    QTimer *trapHideTimer;
    QTimer *alarmCountdownTimer;
    int alarmSecondsLeft;
    bool l4AlarmTimerStarted;


    void checkStateTransitions();

    // Projectile animation driven by timer
    void drawLevel1(QPainter &p, int tileSize, int ox, int oy);
    void drawLevel2(QPainter &p, int tileSize, int ox, int oy);
    void drawLevel3(QPainter &p, int tileSize, int ox, int oy);
    void drawLevel4(QPainter &p, int tileSize, int ox, int oy);
    void drawLevel5(QPainter &p, int tileSize, int ox, int oy);

    void drawPlayer(QPainter &p, int px, int py, int tileSize);
    void drawEnemy(QPainter &p, const Enemy *e, int ox, int oy, int tileSize);
    void drawProjectiles(QPainter &p, int ox, int oy, int tileSize);
    void drawHUD(QPainter &p);
    void drawFlashEvents(QPainter &p, int ox, int oy, int tileSize);
    void drawRiddleDialog(QPainter &p);
    void drawStoryBanner(QPainter &p);
    void drawHealthBar(QPainter &p, int x, int y, int w, int h, int hp, int maxhp, QColor fill);

    QRect tileRect(int x, int y, int tileSize, int ox, int oy) const;

public:
    explicit GameView(QWidget *parent = nullptr);

    void setPlayerProfile(const QString &name, const QString &role);
    void resetGame();
    void clearLevel4Timers();
    Game &getGame();

signals:
    void levelComplete(int level);
    void gameLost();
    void gameWon();

protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
};

#endif
