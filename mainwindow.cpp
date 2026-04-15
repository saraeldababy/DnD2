#include "mainwindow.h"
#include "gameview.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    stack = new QStackedWidget(this);

    // MENU
    menuScreen = new QWidget(this);

    QLabel *menuLabel = new QLabel(
        "Welcome to Dungeon Realms!\n\n"
        "Mission 1: Use your strategic skils to escape the fighter enemy!\n"
        "Rules:\n"
        "- Use the arrow keys to move\n"
        "- Your character is blue\n"
        "- Avoid the red enemy\n"
        "- Goal: Reach green exit\n\n"
        "Click ENTER to play if you accept the mission"
        );

    QVBoxLayout *menuLayout = new QVBoxLayout(menuScreen);
    menuLayout->addWidget(menuLabel);

    // GAME
    gameScreen = new GameView(this);
    connect(gameScreen, &GameView::gameWon, this, [=]()
            {
                stack->setCurrentIndex(2); // end screen
            });

    connect(gameScreen, &GameView::gameLost, this, [=]()
            {
                stack->setCurrentIndex(2); // end screen
            });

    // END
    endLabel = new QLabel("Game Over / Win");
    endScreen = new QWidget(this);

    QVBoxLayout *endLayout = new QVBoxLayout(endScreen);
    endLayout->addWidget(endLabel);

    // STACK
    stack->addWidget(menuScreen);  // 0
    stack->addWidget(gameScreen);   // 1
    stack->addWidget(endScreen);    // 2

    setCentralWidget(stack);

    stack->setCurrentIndex(0);

    // SIGNALS
    setFocusPolicy(Qt::StrongFocus);
    connect(gameScreen, &GameView::gameWon, this, [=]()
            {
                endLabel->setText("YOU WIN!\nPress R to restart");
                stack->setCurrentIndex(2);
            });

    connect(gameScreen, &GameView::gameLost, this, [=]()
            {
                endLabel->setText("GAME OVER!\nPress R to restart");
                stack->setCurrentIndex(2);
            });
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // ENTER → start game from menu
    if (stack->currentIndex() == 0 &&
        event->key() == Qt::Key_Return)
    {
        stack->setCurrentIndex(1);
    }

    // R → restart from end screen
    if (stack->currentIndex() == 2 &&
        event->key() == Qt::Key_R)
    {
        gameScreen->resetGame(10);
        stack->setCurrentIndex(1);
    }
}

MainWindow::~MainWindow() {}
