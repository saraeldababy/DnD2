#ifndef ENEMY_H
#define ENEMY_H

#include "character.h"
#include <QString>
#include <QVector>
#include <QPoint>

enum class EnemyType {
    SHADOW,
    WITCH,
    ARCHER,
    TRAP,
    DRAGON,
    PATROL
};

class Enemy : public Character
{
protected:
    EnemyType type;
    int attackPower;
    bool alive;
    QString name;
    int id;

public:
    Enemy(int startX, int startY, int hp, int atk,
          EnemyType t, const QString &n, int id = 0);

    virtual ~Enemy() = default;

    EnemyType getType() const;
    QString getName() const;
    int getAttackPower() const;
    int getId() const;

    bool isDefeated() const;
    void defeat();

    virtual void moveToward(int tx, int ty, bool (*walkable)(int, int));
    virtual bool canAttackFrom(int px, int py) const;
    virtual void onAlarm(){}

    void setX(int v);
    void setY(int v);
};

// ---- Shadow ----
class ShadowEnemy : public Enemy
{
public:
    ShadowEnemy(int x, int y);
    void moveToward(int tx, int ty, bool (*walkable)(int, int)) override;
};

// ---- Witch ----
class WitchEnemy : public Enemy
{
private:
    bool riddleSolved;
public:
    WitchEnemy(int x, int y);
    bool isRiddleSolved() const;
    void solveRiddle();
};

// ---- Archer ----
class ArcherEnemy : public Enemy
{
private:
    int range;
    int cooldown;
    int turnsUntilShot;
public:
    ArcherEnemy(int x, int y, int id);
    bool canAttackFrom(int px, int py) const override;
    bool readyToShoot() const;
    void resetCooldown();
    void tickCooldown();
    int getRange() const;
};

// ---- Trap ----
class TrapEnemy : public Enemy
{
private:
    bool triggered;
    bool visible;
public:
    TrapEnemy(int x, int y, int id, int atk = 25);
    bool isTriggered() const;
    bool isVisible() const;
    void hide();
    void trigger();
    void reset();
};

// ---- Dragon ----
class DragonEnemy : public Enemy
{
private:
    int phase;
    int breathCooldown;
    int turnsUntilBreath;
    int normalHitCount;
    int megaFireCountdown;
    QPoint megaFireCenter;
public:
    DragonEnemy(int x, int y);
    int getPhase() const;
    void updatePhase();
    bool canBreatheFire(int px, int py) const;
    bool readyToBreath() const;
    void resetBreathCooldown();
    void tickBreathCooldown();
    // Megafire
    void incrementHitCount();
    void resetHitCount();
    int  getNormalHitCount() const;
    void chargeMegaFire(int px, int py);
    void decrementMegaFireCountdown();
    int  getMegaFireCountdown() const;
    QPoint getMegaFireCenter() const;

};

// ---- Patrol Guard ----
// class PatrolEnemy : public Enemy
// {
// private:
//     int ax, ay;    // waypoint A
//     int bx, by;    // waypoint B
//     bool goingToB;
//     bool fast;     // true after alarm fires
//     int turnCount;
// public:
//     PatrolEnemy(int sx, int sy, int eax, int eay, int ebx, int eby, int id = 0);
//     void moveToward(int tx, int ty, bool (*walkable)(int, int)) override;
//     void onAlarm() override;
// };
class PatrolEnemy : public Enemy
{
private:
    QVector<QPoint> waypoints;
    int waypointIndex;
    bool chasing;
public:
    PatrolEnemy(int sx, int sy, QVector<QPoint> pts, int id = 0);
    void moveToward(int tx, int ty, bool (*walkable)(int, int)) override;
    void onAlarm() override;
};

#endif
