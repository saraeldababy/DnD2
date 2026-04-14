#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameWidget(QWidget *parent = nullptr);
    void resetGame(int size =10);
signals:
    void gameWon();
    void gameLost();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
private:
    int playerX;
    int playerY;
    int gridSize;
    int tileSize;

    // 0 = empty, 1 = wall, 2 = exit, 3 = enemy
    int map[10][10];

    void setupMap();

};


#endif
