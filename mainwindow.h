#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include "gameview.h"
#include "savemanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QStackedWidget *stack;

    // Screen 0: Main menu
    QWidget    *menuScreen;
    QLineEdit  *nameInput;
    QComboBox  *roleBox;
    QPushButton *continueBtn;

    // Screen 1: Game
    GameView *gameScreen;

    // Screen 2: Level transition
    QWidget *transitionScreen;
    QLabel  *transitionLabel;

    // Screen 3: End screen (win/lose)
    QWidget *endScreen;
    QLabel  *endLabel;

    QString playerName;
    QString selectedRole;
    int     lastCompletedLevel;

    QWidget *buildMenuScreen();
    QWidget *buildTransitionScreen();
    QWidget *buildEndScreen();

    void startGame(bool fromSave = false);
    void showLevelTransition(int completedLevel);
    void showEndScreen(bool won);
    void saveCurrentGame();
};

#endif
