#include "mainwindow.h"
#include "gameview.h"
#include "level2view.h"
#include "level3view.h"
#include <QComboBox>
#include <QFont>
#include <QLineEdit>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    stack = new QStackedWidget(this);
    menuScreen = new QWidget(this);
    menuScreen->setStyleSheet("background-color:#f2e6c8; color:#3a2c20;");
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
        "Goal: Reach the cottage, outsmart the witch, slay the dragon.\n\n"
        "How to play:\n"
        "- Arrow keys move your hero\n"
        "- Trees and river water block movement\n"
        "- Follow the lantern trail in Level 1\n"
        "- Avoid traps, collect treasure for hints\n\n"
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
    gameScreen   = new GameView(this);
    level2Screen = new Level2View(this);
    level3Screen = new Level3View(this);
    endLabel  = new QLabel();
    endScreen = new QWidget(this);
    endScreen->setStyleSheet("background-color:#0d0a06; color:#f2e6c8;");
    endLabel->setAlignment(Qt::AlignCenter);
    endLabel->setFont(QFont("Georgia", 16, QFont::Bold));
    endLabel->setStyleSheet("color:#f2e6c8;");
    QVBoxLayout *endLayout = new QVBoxLayout(endScreen);
    endLayout->addWidget(endLabel);
    stack->addWidget(menuScreen);    
    stack->addWidget(gameScreen);    
    stack->addWidget(level2Screen);  
    stack->addWidget(level3Screen);  
    stack->addWidget(endScreen);     
    setCentralWidget(stack);
    stack->setCurrentIndex(0);
    setFocusPolicy(Qt::StrongFocus);
    connect(gameScreen, &GameView::gameWon, this, [=]() {
        level2Screen->startLevel(playerName, selectedRole);
        stack->setCurrentIndex(2);
        level2Screen->setFocus();
    });
    connect(gameScreen, &GameView::gameLost, this, [=]() {
        endLabel->setText("DEFEAT!\nThe shadow beast caught you.\n\nPress R to try again.");
        stack->setCurrentIndex(4);
    });
    connect(level2Screen, &Level2View::levelComplete, this, [=]() {
        level3Screen->startLevel(playerName, selectedRole);
        stack->setCurrentIndex(3);
        level3Screen->setFocus();
    });
    connect(level3Screen, &Level3View::levelComplete, this, [=]() {
        level3Screen->setFocus();
    });
}
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (stack->currentIndex() == 0 && event->key() == Qt::Key_Return) {
        playerName   = nameInput->text();
        selectedRole = roleBox->currentText();
        if (playerName.isEmpty()) playerName = "Adventurer";
        gameScreen->setPlayerProfile(playerName, selectedRole);
        stack->setCurrentIndex(1);
        gameScreen->setFocus();
    }
    if (stack->currentIndex() == 4 && event->key() == Qt::Key_R) {
        gameScreen->resetGame(10);
        gameScreen->setPlayerProfile(playerName, selectedRole);
        stack->setCurrentIndex(1);
        gameScreen->setFocus();
    }
}
MainWindow::~MainWindow() {}
