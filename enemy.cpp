#include "enemy.h"
#include <cstdlib>

Enemy::Enemy(int startX, int startY, int hp, int atk, EnemyType t, const QString &n, int id_)
    : Character(startX, startY, hp), type(t), attackPower(atk), alive(true), name(n), id(id_)
{}

EnemyType Enemy::getType() const { return type; }
QString   Enemy::getName() const { return name; }
int       Enemy::getAttackPower() const { return attackPower; }
int       Enemy::getId() const { return id; }
bool      Enemy::isDefeated() const { return !alive; }
void      Enemy::defeat() { alive = false; health = 0; }
void      Enemy::setX(int v) { x = v; }
void      Enemy::setY(int v) { y = v; }

void Enemy::moveToward(int tx, int ty, bool (*walkable)(int, int))
{
    const int stepX = (tx > x) ? 1 : ((tx < x) ? -1 : 0);
    const int stepY = (ty > y) ? 1 : ((ty < y) ? -1 : 0);
    if (stepX != 0 && walkable(x + stepX, y))      x += stepX;
    else if (stepY != 0 && walkable(x, y + stepY)) y += stepY;
}

bool Enemy::canAttackFrom(int /*px*/, int /*py*/) const { return false; }

// --- Shadow ---
ShadowEnemy::ShadowEnemy(int sx, int sy)
    : Enemy(sx, sy, 60, 15, EnemyType::SHADOW, "Shadow Beast")
{}

void ShadowEnemy::moveToward(int tx, int ty, bool (*walkable)(int, int))
{
    int dx = tx - x;
    int dy = ty - y;
    int stepX = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
    int stepY = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;

    if (abs(dx) >= abs(dy)) {
        if (stepX != 0 && walkable(x + stepX, y))
            x += stepX;
        else if (stepY != 0 && walkable(x, y + stepY))
            y += stepY;
    } else {
        if (stepY != 0 && walkable(x, y + stepY))
            y += stepY;
        else if (stepX != 0 && walkable(x + stepX, y))
            x += stepX;
    }
}

// --- Witch ---
WitchEnemy::WitchEnemy(int sx, int sy)
    : Enemy(sx, sy, 80, 20, EnemyType::WITCH, "Morgana the Witch"), riddleSolved(false)
{}
bool WitchEnemy::isRiddleSolved() const { return riddleSolved; }
void WitchEnemy::solveRiddle() { riddleSolved = true; }

// --- Archer ---
ArcherEnemy::ArcherEnemy(int sx, int sy, int id_)
    : Enemy(sx, sy, 50, 18, EnemyType::ARCHER, "Tower Archer", id_),
    range(4), cooldown(3), turnsUntilShot(1)
{}

bool ArcherEnemy::canAttackFrom(int px, int py) const
{
    if (isDefeated()) return false;
    int dx = abs(px - x), dy = abs(py - y);
    return (dx == 0 && dy <= range) || (dy == 0 && dx <= range);
}
bool ArcherEnemy::readyToShoot() const { return turnsUntilShot <= 0; }
void ArcherEnemy::resetCooldown() { turnsUntilShot = cooldown; }
void ArcherEnemy::tickCooldown()  { if (turnsUntilShot > 0) turnsUntilShot--; }
int  ArcherEnemy::getRange() const { return range; }

// --- Trap ---
TrapEnemy::TrapEnemy(int sx, int sy, int id_, int atk)
    : Enemy(sx, sy, 1, 25, EnemyType::TRAP, "Floor Trap", id_),
    triggered(false), visible(true)
{}
bool TrapEnemy::isTriggered() const { return triggered; }
bool TrapEnemy::isVisible()   const { return visible || triggered; }
void TrapEnemy::trigger() { triggered = true; visible = true; }
void TrapEnemy::reset()   { triggered = false; }
void TrapEnemy::hide() { if (!triggered) visible = false; }

// --- Patrol Guard ---
// PatrolEnemy::PatrolEnemy(int sx, int sy, int eax, int eay, int ebx, int eby, int id_)
//     : Enemy(sx, sy, 40, 15, EnemyType::PATROL, "Castle Guard", id_),
//     ax(eax), ay(eay), bx(ebx), by(eby),
//     goingToB(true), fast(false), turnCount(0)
// {}

// void PatrolEnemy::moveToward(int /*tx*/, int /*ty*/, bool (*walkable)(int, int))
// {
//     if (isDefeated()) return;
//     turnCount++;
//     if (!fast && turnCount % 2 != 0) return; // normal: every other turn

//     int targetX = goingToB ? bx : ax;
//     int targetY = goingToB ? by : ay;
//     int dx = targetX - x;
//     int dy = targetY - y;

//     if (dx == 0 && dy == 0) { goingToB = !goingToB; return; } // reached waypoint

//     int stepX = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
//     int stepY = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;

//     if (abs(dx) >= abs(dy)) {
//         if (stepX != 0 && walkable(x + stepX, y)) x += stepX;
//         else if (stepY != 0 && walkable(x, y + stepY)) y += stepY;
//     } else {
//         if (stepY != 0 && walkable(x, y + stepY)) y += stepY;
//         else if (stepX != 0 && walkable(x + stepX, y)) x += stepX;
//     }
// }

// void PatrolEnemy::onAlarm() { fast = true; }
// --- Patrol Guard ---
PatrolEnemy::PatrolEnemy(int sx, int sy, QVector<QPoint> pts, int id_)
    : Enemy(sx, sy, 40, 15, EnemyType::PATROL, "Castle Guard", id_),
    waypoints(pts), waypointIndex(0), chasing(false)
{}

void PatrolEnemy::moveToward(int tx, int ty, bool (*walkable)(int, int))
{
    if (isDefeated()) return;

    if (chasing) {
        int dx = tx - x, dy = ty - y;
        int stepX = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
        int stepY = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
        if (abs(dx) >= abs(dy)) {
            if (stepX != 0 && walkable(x + stepX, y)) x += stepX;
            else if (stepY != 0 && walkable(x, y + stepY)) y += stepY;
        } else {
            if (stepY != 0 && walkable(x, y + stepY)) y += stepY;
            else if (stepX != 0 && walkable(x + stepX, y)) x += stepX;
        }
        return;
    }

    if (waypoints.isEmpty()) return;
    QPoint target = waypoints[waypointIndex];
    int dx = target.x() - x, dy = target.y() - y;

    if (dx == 0 && dy == 0) {
        waypointIndex = (waypointIndex + 1) % waypoints.size();
        return;
    }

    int stepX = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
    int stepY = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;

    if (abs(dx) >= abs(dy)) {
        if (stepX != 0 && walkable(x + stepX, y)) x += stepX;
        else if (stepY != 0 && walkable(x, y + stepY)) y += stepY;
    } else {
        if (stepY != 0 && walkable(x, y + stepY)) y += stepY;
        else if (stepX != 0 && walkable(x + stepX, y)) x += stepX;
    }
}

void PatrolEnemy::onAlarm() { chasing = true; }


// --- Dragon ---
DragonEnemy::DragonEnemy(int sx, int sy)
    : Enemy(sx, sy, 200, 30, EnemyType::DRAGON, "Drakoroth the Dragon"),
    phase(1), breathCooldown(4), turnsUntilBreath(2),
    normalHitCount(0), megaFireCountdown(0), megaFireCenter(0, 0)
{}

int  DragonEnemy::getPhase() const { return phase; }
void DragonEnemy::updatePhase()
{
    if (health <= maxHealth / 2 && phase == 1)
    {
        phase = 2;
        attackPower = 45;
    }
}
bool DragonEnemy::canBreatheFire(int px, int py) const
{
    if (isDefeated()) return false;
    return abs(px - x) <= 3 && abs(py - y) <= 3;
}
bool DragonEnemy::readyToBreath() const { return turnsUntilBreath <= 0; }
void DragonEnemy::resetBreathCooldown()
{
    turnsUntilBreath = (phase == 2) ? 2 : breathCooldown;
}
void DragonEnemy::incrementHitCount() { normalHitCount++; }
void DragonEnemy::resetHitCount()     { normalHitCount = 0; }
int  DragonEnemy::getNormalHitCount() const { return normalHitCount; }

void DragonEnemy::chargeMegaFire(int px, int py)
{
    megaFireCountdown = 2;
    megaFireCenter    = QPoint(px, py);
}

void DragonEnemy::decrementMegaFireCountdown()
{
    if (megaFireCountdown > 0) megaFireCountdown--;
}

int    DragonEnemy::getMegaFireCountdown() const { return megaFireCountdown; }
QPoint DragonEnemy::getMegaFireCenter()    const { return megaFireCenter; }
void DragonEnemy::tickBreathCooldown()
{
    if (turnsUntilBreath > 0) turnsUntilBreath--;
}
