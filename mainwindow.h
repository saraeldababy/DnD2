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
#include <QTimer>
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
    QWidget    *menuScreen;
    QLineEdit  *nameInput;
    QComboBox  *roleBox;
    QPushButton *continueBtn;
    GameView *gameScreen;
    QWidget *transitionScreen;
    QLabel  *transitionLabel;
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
    QTimer *autoSaveTimer;
};
#endif
