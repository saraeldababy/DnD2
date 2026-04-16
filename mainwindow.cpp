#include "mainwindow.h"
#include "gameview.h"

#include <QComboBox>
#include <QFont>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    stack = new QStackedWidget(this);

    menuScreen = new QWidget(this);
    menuScreen->setStyleSheet(
        "background-color: #1f1a1a;"
        "color: #f2e6c8;"
    );

    QLabel *titleLabel = new QLabel("Dungeon Realms: The Whispering Forest");
    QLabel *nameLabel = new QLabel("Adventurer name:");
    nameInput = new QLineEdit(this);
    QLabel *roleLabel = new QLabel("Choose your class:");

    roleBox = new QComboBox(this);
    roleBox->addItem("Wizard");
    roleBox->addItem("Fighter");
    roleBox->addItem("Rogue");
    roleBox->addItem("Cleric");

    QLabel *rulesLabel = new QLabel(
        "\nStory: A shadow beast haunts the old forest road.\n"
        "Goal: Reach the cottage safely.\n\n"
        "How to play:\n"
        "- Arrow keys move your hero through the forest\n"
        "- River water blocks movement, trees are passable\n"
        "- Follow the lantern trail across the bridge\n"
        "- The shadow chases you immediately, so keep moving\n\n"
        "Press ENTER to begin"
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

    gameScreen = new GameView(this);

    endLabel = new QLabel("Game Over / Win");
    endScreen = new QWidget(this);
    QVBoxLayout *endLayout = new QVBoxLayout(endScreen);
    endLayout->addWidget(endLabel);

    stack->addWidget(menuScreen); // 0
    stack->addWidget(gameScreen); // 1
    stack->addWidget(endScreen);  // 2

    setCentralWidget(stack);
    stack->setCurrentIndex(0);

    setFocusPolicy(Qt::StrongFocus);

    connect(gameScreen, &GameView::gameWon, this, [=]() {
        endLabel->setText("VICTORY!\nYou guided the party to safety.\nPress R to replay.");
        stack->setCurrentIndex(2);
    });

    connect(gameScreen, &GameView::gameLost, this, [=]() {
        endLabel->setText("DEFEAT!\nThe shadow beast caught you.\nPress R to try again.");
        stack->setCurrentIndex(2);
    });
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (stack->currentIndex() == 0 && event->key() == Qt::Key_Return)
    {
        playerName = nameInput->text();
        selectedRole = roleBox->currentText();

        if (playerName.isEmpty())
            playerName = "Adventurer";

        stack->setCurrentIndex(1);
    }

    if (stack->currentIndex() == 2 && event->key() == Qt::Key_R)
    {
        gameScreen->resetGame(10);
        stack->setCurrentIndex(1);
    }
}

MainWindow::~MainWindow() {}
