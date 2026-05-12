#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QKeyEvent>
#include "gameview.h"
#include "level2view.h"
#include <QLineEdit>
#include <QComboBox>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void keyPressEvent(QKeyEvent *event) override;
private:
    Ui::MainWindow *ui;
    QStackedWidget *stack;
    QWidget *menuScreen;
    GameView *gameScreen;
    QWidget *endScreen;
    QLabel *endLabel;
    Level2View *level2Screen;
QLineEdit *nameInput;
QComboBox *roleBox;

QString playerName;
QString selectedRole;
};
#endif // MAINWINDOW_H
