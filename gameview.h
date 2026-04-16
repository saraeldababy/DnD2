#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QWidget>
#include <QPainter>
#include <QKeyEvent>
#include "game.h"

class GameView : public QWidget
{
    Q_OBJECT

private:
    Game game;
    QString playerName = "Adventurer";
    bool showIntroDialog = true;

public:
    explicit GameView(QWidget *parent = nullptr);

    void resetGame(int size);
    void setPlayerProfile(const QString &name, const QString &role);

signals:
    void gameWon();
    void gameLost();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif
