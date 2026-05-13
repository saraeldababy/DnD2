#ifndef GAME_H
#define GAME_H

#pragma once

#include <QString>
#include <QVector>
#include <QPoint>
#include "player.h"
#include "enemy.h"
#include "level.h"

// Projectile (arrows, fireballs)
struct Projectile {
    float x, y;
    float dx, dy;
    int damage;
    QString type; // "arrow" or "fireball"
    bool active;
};

// Animation flash event (for hits, defeats)
struct FlashEvent {
    int x, y;
    int framesLeft;
    QString type; // "hit", "defeat", "treasure"
};

class Game
{
public:
    enum class Phase {
        LEVEL1_EXPLORE,       // Level 1: escape shadow
        LEVEL2_RIDDLE,        // Level 2: riddle dialog
        LEVEL2_COMBAT,        // Level 2: after riddle, fight or pass
        LEVEL3_INFILTRATE,    // Level 3: castle exterior, shoot archers
        LEVEL4_NAVIGATE,      // Level 4: castle interior, traps & keys
        LEVEL5_DRAGON,        // Level 5: dragon boss fight
        VICTORY,
        DEAD
    };
    void triggerAlarm();
    void activateGuardChase();   // called by GameView when 5-second timer fires
    bool isAlarmActive() const;
    bool areGuardsChasing() const;

private:
    Player player;
    QVector<Enemy*> enemies;
    Level level;
    Phase phase;
    int currentLevel;
    int storyState;
    int turns;

    // Level 2 riddle system
    int riddleIndex;
    bool riddleAnswered;
    bool riddleCorrect;
    int riddleAttempts;

    // Level 3
    QVector<Projectile> projectiles;

    // Level 4
    QVector<QPoint> trapPositions;
    QVector<QPoint> keyPositions;
    QVector<QPoint> lockedDoorPositions;
    QVector<QPoint> chestPositions;
    bool doorsOpened[4]; // 4 locked doors in level 4
    bool alarmActive;
    bool guardsChasing;

    // Level 5
    bool cellUnlocked;
    bool dragonDefeated;
    bool hasSword;
    bool megaFireWarningActive;

    // Visuals
    QVector<FlashEvent> flashEvents;

    // Combat turn tracking
    int damageCooldown; // prevent instant-death spam

    // Story messages queue
    QVector<QString> storyMessages;
    int storyMsgTimer;

    void buildLevel1();
    void buildLevel2();
    void buildLevel3();
    void buildLevel4();
    void buildLevel5();

    void clearEnemies();

    bool levelWalkable(int x, int y) const;

public:
    Game(int size = 10);
    ~Game();

    // Core loop
    void movePlayer(int dx, int dy);
    void updateEnemies();
    void updateProjectiles();
    bool checkWin();
    bool checkLose();

    // Player attack (Space bar)
    void playerAttack();

    // Riddle system
    QString getRiddleQuestion() const;
    QVector<QString> getRiddleChoices() const;
    void answerRiddle(int choiceIndex);
    bool isRiddleActive() const;
    bool getRiddleCorrect() const;
    int getRiddleAttempts() const;

    // Potion
    bool usePotion();

    // Reset / restart
    void reinit();               // full reset, safe to call on existing object
    void restartCurrentLevel();  // rebuild current level only, restore health

    // Getters
    Phase getPhase() const;
    int getCurrentLevel() const;
    Player &getPlayer();
    const QVector<Enemy*> &getEnemies() const;
    Level &getLevel();
    const QVector<Projectile> &getProjectiles() const;
    const QVector<FlashEvent> &getFlashEvents() const;
    void clearFlashEvents();

    bool isDoorOpen(int i) const;
    bool isCellUnlocked() const;
    bool playerHasSword() const;
    bool isMegaFireWarning() const;
    QPoint getMegaFireWarningCenter() const;

    // Messaging
    QString storyHint() const;
    QString getLatestStoryMessage() const;
    void tickStoryMessage();
    bool hasStoryMessage() const;

    // Level transition
    void advanceToNextLevel();

    // Save/Load
    struct SaveState {
        int currentLevel;
        int phaseInt;
        int playerX, playerY, playerHealth;
        int playerScore, playerPotions, playerKeys;
        QString playerName, playerRole;
        QVector<int> enemyHP;
        bool doorsOpened[4];
        bool cellUnlocked;
    };
    SaveState getSaveState() const;
    void loadSaveState(const SaveState &s);
};

#endif
