#include "mainwindow.h"
#include <QApplication>
#include <QFont>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), lastCompletedLevel(0), autoSaveTimer(nullptr)
{
    setWindowTitle("Dungeon Realms: The Whispering Forest");
    stack = new QStackedWidget(this);

    menuScreen       = buildMenuScreen();
    transitionScreen = buildTransitionScreen();
    endScreen        = buildEndScreen();
    gameScreen       = new GameView(this);

    stack->addWidget(menuScreen);        // 0
    stack->addWidget(gameScreen);        // 1
    stack->addWidget(transitionScreen);  // 2
    stack->addWidget(endScreen);         // 3

    setCentralWidget(stack);
    stack->setCurrentIndex(0);

    setFixedSize(900, 680);
    setFocusPolicy(Qt::StrongFocus);

    // Autosave every 5 seconds while playing
    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setInterval(5000);
    connect(autoSaveTimer, &QTimer::timeout, this, [this]() {
        if (stack->currentIndex() == 1)
            saveCurrentGame();
    });

    connect(gameScreen, &GameView::skipLevelRequested, this, [this]() {
        Game &g = gameScreen->getGame();
        if (g.getCurrentLevel() < 5) {
            int completedLevel = g.getCurrentLevel();
            g.advanceToNextLevel();
            saveCurrentGame();
            showLevelTransition(completedLevel);
        }
    });

    connect(gameScreen, &GameView::levelComplete, this, [this](int level) {
        gameScreen->clearLevel4Timers();
        saveCurrentGame();
        showLevelTransition(level);
    });

    connect(gameScreen, &GameView::gameLost, this, [this]() {
        int level = gameScreen->getGame().getCurrentLevel();
        int delay = 2000;
        QString msg;
        if (level == 1)
        {
            msg = "The shadow caught you!\n\nReturning to the forest...";
            delay = 1500;
        }
        else
        {
            msg = "You have been defeated!\n\nLevel " + QString::number(level) + " restarting...";
        }
        transitionLabel->setText(msg);
        stack->setCurrentIndex(2);
        QTimer::singleShot(delay, this, [this]() {
            if (stack->currentIndex() == 2)
            {
                gameScreen->clearLevel4Timers();
                gameScreen->getGame().restartCurrentLevel();
                gameScreen->getGame().clearFlashEvents();
                stack->setCurrentIndex(1);
                gameScreen->setFocus();
            }
        });
    });

    connect(gameScreen, &GameView::gameWon, this, [this]() {
        SaveManager::deleteSave();
        showEndScreen(true);
    });
}

// -------------------------------------------------------
// Screen builders
// -------------------------------------------------------

QWidget *MainWindow::buildMenuScreen()
{
    QWidget *w = new QWidget(this);
    w->setStyleSheet("background-color: #0e0b1a; color: #e8d5a8;");

    QLabel *titleLabel = new QLabel("DUNGEON REALMS");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "color: #f0c060; font-family: Georgia; font-size: 34px; font-weight: bold;"
        "letter-spacing: 4px; margin-bottom: 4px;");

    QLabel *subLabel = new QLabel("The Whispering Forest");
    subLabel->setAlignment(Qt::AlignCenter);
    subLabel->setStyleSheet(
        "color: #b89060; font-family: Georgia; font-size: 14px; font-style: italic;"
        "margin-bottom: 20px;");

    QLabel *loreLabel = new QLabel(
        "Your friend Aldric ventured into the forbidden castle to steal the dragon's treasure.\n"
        "He was captured. You must brave five perilous levels to bring him home.");
    loreLabel->setAlignment(Qt::AlignCenter);
    loreLabel->setWordWrap(true);
    loreLabel->setStyleSheet("color: #c8b080; font-family: Georgia; font-size: 11px; margin: 0 40px;");

    QLabel *nameLabel = new QLabel("Adventurer Name:");
    nameLabel->setStyleSheet("color: #d4b878; font-family: Georgia; font-size: 12px;");

    nameInput = new QLineEdit();
    nameInput->setPlaceholderText("Enter your name...");
    nameInput->setStyleSheet(
        "background: #1e1830; color: #f0e0b0; border: 1px solid #6040a0;"
        "font-family: Georgia; font-size: 12px; padding: 6px; border-radius: 4px;");

    QLabel *roleLabel = new QLabel("Choose Your Class:");
    roleLabel->setStyleSheet("color: #d4b878; font-family: Georgia; font-size: 12px;");

    roleBox = new QComboBox();
    roleBox->addItem("Wizard  — 3-tile spell range, 5 spell charges");
    roleBox->addItem("Fighter — Shield bonus, high melee damage");
    roleBox->addItem("Rogue   — Stealth, chance to double-strike");
    roleBox->addItem("Cleric  — Healing bonus, 4 starting potions");
    roleBox->setStyleSheet(
        "background: #1e1830; color: #f0e0b0; border: 1px solid #6040a0;"
        "font-family: Georgia; font-size: 11px; padding: 4px; border-radius: 4px;");

    QLabel *rulesLabel = new QLabel(
        "Arrow Keys: Move your hero\n"
        "Space: Attack (melee or ranged depending on class)\n"
        "P: Use a health potion\n"
        "Walk over keys to collect them\n"
        "S: Save game at any time\n\n"
        "5 levels of increasing danger await. Good luck, hero.");
    rulesLabel->setWordWrap(true);
    rulesLabel->setAlignment(Qt::AlignCenter);
    rulesLabel->setStyleSheet(
        "color: #a09060; font-family: Georgia; font-size: 10px;"
        "background: #120f1e; border: 1px solid #3a2a50;"
        "padding: 10px; border-radius: 6px; margin: 0 20px;");

    QPushButton *startBtn = new QPushButton("BEGIN ADVENTURE");
    startBtn->setStyleSheet(
        "QPushButton { background: #4a2070; color: #f0d890; font-family: Georgia; font-size: 14px;"
        "  font-weight: bold; border: 2px solid #8040c0; border-radius: 8px; padding: 10px 30px; }"
        "QPushButton:hover { background: #6030a0; border-color: #c080ff; }"
        "QPushButton:pressed { background: #3a1860; }");

    continueBtn = new QPushButton("CONTINUE SAVED GAME");
    continueBtn->setStyleSheet(
        "QPushButton { background: #203040; color: #80c0e0; font-family: Georgia; font-size: 12px;"
        "  border: 2px solid #406080; border-radius: 8px; padding: 8px 20px; }"
        "QPushButton:hover { background: #304858; border-color: #60a0c0; }"
        "QPushButton:disabled { background: #181820; color: #404040; border-color: #303030; }");
    continueBtn->setEnabled(SaveManager::hasSaveFile());

    connect(startBtn,    &QPushButton::clicked, this, [this]() { startGame(false); });
    connect(continueBtn, &QPushButton::clicked, this, [this]() { startGame(true);  });

    QVBoxLayout *layout = new QVBoxLayout(w);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(10);
    layout->addStretch(1);
    layout->addWidget(titleLabel);
    layout->addWidget(subLabel);
    layout->addWidget(loreLabel);
    layout->addSpacing(14);
    layout->addWidget(nameLabel);
    layout->addWidget(nameInput);
    layout->addSpacing(6);
    layout->addWidget(roleLabel);
    layout->addWidget(roleBox);
    layout->addSpacing(10);
    layout->addWidget(rulesLabel);
    layout->addSpacing(14);
    layout->addWidget(startBtn,   0, Qt::AlignCenter);
    layout->addWidget(continueBtn, 0, Qt::AlignCenter);
    layout->addStretch(1);

    return w;
}

QWidget *MainWindow::buildTransitionScreen()
{
    QWidget *w = new QWidget(this);
    w->setStyleSheet("background-color: #060410;");

    transitionLabel = new QLabel("Level Complete!");
    transitionLabel->setAlignment(Qt::AlignCenter);
    transitionLabel->setWordWrap(true);
    transitionLabel->setStyleSheet(
        "color: #f0d060; font-family: Georgia; font-size: 20px; font-weight: bold; margin: 20px;");

    QLabel *hint = new QLabel("Game saved.  Press Enter to continue or wait...");
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: #806050; font-family: Georgia; font-size: 11px;");

    QVBoxLayout *layout = new QVBoxLayout(w);
    layout->setAlignment(Qt::AlignCenter);
    layout->addWidget(transitionLabel);
    layout->addWidget(hint);

    return w;
}

QWidget *MainWindow::buildEndScreen()
{
    QWidget *w = new QWidget(this);
    w->setStyleSheet("background-color: #080510;");

    endLabel = new QLabel("...");
    endLabel->setAlignment(Qt::AlignCenter);
    endLabel->setWordWrap(true);
    endLabel->setStyleSheet(
        "color: #f0e0a0; font-family: Georgia; font-size: 18px; margin: 30px;");

    QLabel *hint = new QLabel("Press R to restart from the beginning");
    hint->setAlignment(Qt::AlignCenter);
    hint->setStyleSheet("color: #706050; font-family: Georgia; font-size: 11px;");

    QVBoxLayout *layout = new QVBoxLayout(w);
    layout->setAlignment(Qt::AlignCenter);
    layout->addWidget(endLabel);
    layout->addWidget(hint);

    return w;
}

// -------------------------------------------------------
// Game start
// -------------------------------------------------------

void MainWindow::startGame(bool fromSave)
{
    playerName = nameInput->text().trimmed();
    if (playerName.isEmpty()) playerName = "Adventurer";

    selectedRole = roleBox->currentText().split("  ").first().trimmed();

    if (fromSave && SaveManager::hasSaveFile())
    {
        Game::SaveState saved;
        if (SaveManager::loadGame(saved))
        {
            gameScreen->resetGame();
            gameScreen->getGame().loadSaveState(saved);
            gameScreen->getGame().getPlayer().setName(playerName);
            stack->setCurrentIndex(1);
            gameScreen->setFocus();
            autoSaveTimer->start();
            return;
        }
    }

    gameScreen->resetGame();
    gameScreen->setPlayerProfile(playerName, selectedRole);
    stack->setCurrentIndex(1);
    gameScreen->setFocus();
    autoSaveTimer->start();
}

// -------------------------------------------------------
// Level transition — messages updated for the 1→4 skip
// -------------------------------------------------------

void MainWindow::showLevelTransition(int completedLevel)
{
    QString msg;
    switch (completedLevel)
    {
    case 1:
        msg = "Level 1 Complete!\n"
              "You escaped the Whispering Forest!\n\n"
              "You learn that your friend Aldric is imprisoned\n"
              "in the dungeon beneath the great castle.\n\n"
              "The Witch's Corridor lies ahead...";
        break;
    case 2:
        msg = "Level 2 Complete!\n"
              "You escaped the witch's curse!\n\n"
              "The castle gates loom before you.\n"
              "Ancient gargoyles stir in the shadows...\n\n"
              "The Dragon's Castle awaits.";
        break;
    case 3:
        msg = "Level 3 Complete!\n"
              "You navigated the cursed corridor!\n\n"
              "Beyond the gate lies the castle interior.\n"
              "Guards patrol every hallway...\n\n"
              "Stay hidden. Find the dungeon entrance.";
        break;
    case 4:
        msg = "Level 4 Complete!\n"
              "You found the dungeon entrance!\n\n"
              "The air reeks of sulfur.\n"
              "A massive shape stirs in the darkness...\n\n"
              "Drakoroth the Dragon awaits.";
        break;
    default:
        msg = "Level " + QString::number(completedLevel) + " Complete!\nPress Enter to continue.";
        break;
    }

    transitionLabel->setText(msg);
    stack->setCurrentIndex(2);

    QTimer::singleShot(3500, this, [this]() {
        if (stack->currentIndex() == 2)
        {
            stack->setCurrentIndex(1);
            gameScreen->setFocus();
        }
    });
}

void MainWindow::showEndScreen(bool won)
{
    autoSaveTimer->stop();
    if (won)
    {
        int score = gameScreen->getGame().getPlayer().getScore();
        endLabel->setText(
            "VICTORY!\n\n"
            "You defeated Drakoroth the Dragon\nand freed Aldric from his cell!\n\n"
            "The treasure is yours - and so is the glory.\n\n"
            "Final Score: " + QString::number(score) + "\n\n"
                                       "Press R to play again");
        endLabel->setStyleSheet(
            "color: #f8e060; font-family: Georgia; font-size: 18px; "
            "font-weight: bold; margin: 30px;");
        endScreen->setStyleSheet("background-color: #0a0c06;");
    }
    else
    {
        int level = gameScreen->getGame().getCurrentLevel();
        endLabel->setText(
            "DEFEATED\n\n"
            "You fell on Level " + QString::number(level) + ".\n"
                                       "Aldric remains imprisoned...\n\n"
                                       "Press R to try again from the beginning");
        endLabel->setStyleSheet(
            "color: #e06040; font-family: Georgia; font-size: 18px; "
            "font-weight: bold; margin: 30px;");
        endScreen->setStyleSheet("background-color: #0c0608;");
    }
    stack->setCurrentIndex(3);
}

void MainWindow::saveCurrentGame()
{
    Game::SaveState state = gameScreen->getGame().getSaveState();
    SaveManager::saveGame(state);
}

// -------------------------------------------------------
// Key events
// -------------------------------------------------------

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (stack->currentIndex() == 3 && event->key() == Qt::Key_R)
    {
        SaveManager::deleteSave();
        continueBtn->setEnabled(false);
        stack->setCurrentIndex(0);
        return;
    }

    if (stack->currentIndex() == 2 && event->key() == Qt::Key_Return)
    {
        stack->setCurrentIndex(1);
        gameScreen->setFocus();
        return;
    }

    if (stack->currentIndex() == 0 && event->key() == Qt::Key_Return)
    {
        startGame(false);
        return;
    }

    if (stack->currentIndex() == 1)
    {
        if (event->key() == Qt::Key_S)
        {
            saveCurrentGame();
            continueBtn->setEnabled(true);
        }
        QMainWindow::keyPressEvent(event);
    }
}

MainWindow::~MainWindow() {}
