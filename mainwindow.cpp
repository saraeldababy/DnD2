#include "mainwindow.h"
#include "gameview.h"
#include <QLineEdit>
#include <QComboBox>
#include <QFont>
#include <QFontDatabase>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    stack = new QStackedWidget(this);

    // MENU
//    menuScreen = new QWidget(this);
//
  //  QLabel *menuLabel = new QLabel(
    //    "Welcome to Dungeon Realms!\n\n"
      //  "Mission 1: Use your strategic skils to escape the fighter enemy!\n"
        //"Rules:\n"
        //"- Use the arrow keys to move\n"
        //"- Your character is blue\n"
        //"- Avoid the red enemy\n"
        //"- Goal: Reach green exit\n\n"
        //"Click ENTER to play if you accept the mission"
        //);

    //QVBoxLayout *menuLayout = new QVBoxLayout(menuScreen);
    //menuLayout->addWidget(menuLabel);
menuScreen = new QWidget(this);
menuScreen->setStyleSheet(
    "background-color: #1e1b18;"
    "color: #e6d3a3;"
);
QLabel *titleLabel = new QLabel("Welcome to Dungeons and Dragons, Level 1");
QLabel *nameLabel = new QLabel("Enter your name:");
nameInput = new QLineEdit(this);
QLabel *roleLabel = new QLabel("Choose your role:");

roleBox = new QComboBox(this);
roleBox->addItem("Wizard");
roleBox->addItem("Fighter");
roleBox->addItem("Rogue");
roleBox->addItem("Cleric");

QLabel *rulesLabel = new QLabel(
    "\nMission 1: Escape the enemy!\n"
    "- Use arrow keys to move\n"
    "- Avoid the red enemy\n"
    "- Reach the green exit\n\n"
    "Press ENTER to start"
);
QFont titleFont("Georgia", 20, QFont::Bold);
QFont bodyFont("Georgia", 12);
titleLabel->setFont(titleFont);

nameLabel->setFont(bodyFont);
roleLabel->setFont(bodyFont);
rulesLabel->setFont(bodyFont);
QVBoxLayout *menuLayout = new QVBoxLayout(menuScreen);
menuLayout->setAlignment(Qt::AlignCenter);
menuLayout->addWidget(titleLabel);
menuLayout->addWidget(nameLabel);
menuLayout->addWidget(nameInput);
menuLayout->addWidget(roleLabel);
menuLayout->addWidget(roleBox);
menuLayout->addWidget(rulesLabel);
menuLayout->addSpacing(10);
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
    //if (stack->currentIndex() == 0 &&
      //  event->key() == Qt::Key_Return)
    //{
      //  stack->setCurrentIndex(1);
    //}

    // R → restart from end screen
if (stack->currentIndex() == 0 &&
    event->key() == Qt::Key_Return)
{
    playerName = nameInput->text();
    selectedRole = roleBox->currentText();

    if (playerName.isEmpty())
        playerName = "Adventurer";

    stack->setCurrentIndex(1);
}
    if (stack->currentIndex() == 2 &&
        event->key() == Qt::Key_R)
    {
        gameScreen->resetGame(10);
        stack->setCurrentIndex(1);
    }
}

MainWindow::~MainWindow() {}
