#include "mainwindow.h"
#include "gameview.h"
#include "level2view.h"

#include <QComboBox>
#include <QFont>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    stack = new QStackedWidget(this);

    // ------------------------------------------------------------------ Menu
    menuScreen = new QWidget(this);
    menuScreen->setStyleSheet(
        "background-color: #f2e6c8;"
        "color: #3a2c20;"
        );

    QLabel *titleLabel = new QLabel("Dungeon Realms: The Whispering Forest");
    QLabel *nameLabel  = new QLabel("Adventurer name:");
    nameInput = new QLineEdit(this);
    nameInput->setPlaceholderText("Type your adventurer name");
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
        "- Trees and river water both block movement\n"
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

    // --------------------------------------------------------------- Screens
    gameScreen   = new GameView(this);
    level2Screen = new Level2View(this);

    // End screen
    endLabel  = new QLabel("Game Over / Win");
    endScreen = new QWidget(this);
    endScreen->setStyleSheet("background-color: #0d0a06; color: #f2e6c8;");
    endLabel->setAlignment(Qt::AlignCenter);
    endLabel->setFont(QFont("Georgia", 16, QFont::Bold));
    QVBoxLayout *endLayout = new QVBoxLayout(endScreen);
    endLayout->addWidget(endLabel);

    // Stack indices:  0=menu  1=level1  2=level2  3=end
    stack->addWidget(menuScreen);    // 0
    stack->addWidget(gameScreen);    // 1
    stack->addWidget(level2Screen);  // 2
    stack->addWidget(endScreen);     // 3

    setCentralWidget(stack);
    stack->setCurrentIndex(0);
    setFocusPolicy(Qt::StrongFocus);

    // Level 1 won -> transition to Level 2
    connect(gameScreen, &GameView::gameWon, this, [=]() {
        level2Screen->startLevel(playerName, selectedRole);
        stack->setCurrentIndex(2);
        level2Screen->setFocus();
    });

    // Level 1 lost -> end screen
    connect(gameScreen, &GameView::gameLost, this, [=]() {
        endLabel->setText(
            "DEFEAT!\nThe shadow beast caught you.\n\nPress R to try again.");
        stack->setCurrentIndex(3);
    });

    // Level 2 complete -> victory screen
    connect(level2Screen, &Level2View::levelComplete, this, [=]() {
        endLabel->setText(
            "VICTORY!\nYou escaped the Witch's Cottage.\n\nPress R to play again.");
        stack->setCurrentIndex(3);
    });
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // Start game from menu
    if (stack->currentIndex() == 0 && event->key() == Qt::Key_Return)
    {
        playerName   = nameInput->text();
        selectedRole = roleBox->currentText();
        if (playerName.isEmpty()) playerName = "Adventurer";

        gameScreen->setPlayerProfile(playerName, selectedRole);
        stack->setCurrentIndex(1);
        gameScreen->setFocus();
    }

    // Replay from end screen
    if (stack->currentIndex() == 3 && event->key() == Qt::Key_R)
    {
        gameScreen->resetGame(10);
        gameScreen->setPlayerProfile(playerName, selectedRole);
        stack->setCurrentIndex(1);
        gameScreen->setFocus();
    }
}

MainWindow::~MainWindow() {}
