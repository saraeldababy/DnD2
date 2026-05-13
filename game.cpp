#include "game.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

static Level *g_levelPtr = nullptr;
static bool staticWalkable(int x, int y)
{
    if (!g_levelPtr) return false;
    return g_levelPtr->isWalkableForEnemy(x, y);
}

// -------------------------------------------------------
// Construction / destruction
// -------------------------------------------------------

Game::Game(int /*size*/)
    : phase(Phase::LEVEL1_EXPLORE), currentLevel(1),
    storyState(0), turns(0),
    riddleIndex(0), riddleAnswered(false), riddleCorrect(false), riddleAttempts(0),
    cellUnlocked(false), dragonDefeated(false),
    hasSword(false), megaFireWarningActive(false),
    alarmActive(false), guardsChasing(false),
    damageCooldown(0), storyMsgTimer(0)
{
    for (int i = 0; i < 4; ++i) doorsOpened[i] = false;
    buildLevel1();
}

Game::~Game() { clearEnemies(); }

void Game::clearEnemies()
{
    for (auto *e : enemies) delete e;
    enemies.clear();
}

bool Game::isAlarmActive() const { return alarmActive; }
bool Game::areGuardsChasing() const { return guardsChasing; }
// -------------------------------------------------------
// Level builders
// -------------------------------------------------------

void Game::buildLevel1()
{
    clearEnemies();
    projectiles.clear();
    flashEvents.clear();
    level = Level(10);
    g_levelPtr = &level;
    phase = Phase::LEVEL1_EXPLORE;
    storyState = 0;
    turns = 0;

    player.setX(1); player.setY(8);

    // FIX: shadow starts at (7,4) — a grass tile it can navigate freely from
    enemies.append(new ShadowEnemy(7, 4));

    storyMessages.clear();
    storyMessages.append("A shadow stirs in the darkness behind you...");
    storyMessages.append("Follow the lanterns! Reach the cottage!");
    storyMsgTimer = 180;
}

void Game::buildLevel2()
{
    // Implemented by partner

}

void Game::buildLevel3()
{
    // Implemented by partner
}

// void Game::buildLevel4()
// {
//     clearEnemies();
//     projectiles.clear();
//     flashEvents.clear();
//     for (int i = 0; i < 4; ++i) doorsOpened[i] = false;
//     trapPositions.clear();
//     keyPositions.clear();
//     lockedDoorPositions.clear();
//     chestPositions.clear();

//     // 14x14 so the map fits on screen (14*44=616px, playable height=620px)
//     level = Level(14);
//     for (int x = 0; x < 14; ++x)
//         for (int y = 0; y < 14; ++y)
//             level.map[x][y] = Level::FLOOR;

//     // Outer walls
//     for (int i = 0; i < 14; ++i)
//     {
//         level.map[i][0]  = Level::WALL;
//         level.map[i][13] = Level::WALL;
//         level.map[0][i]  = Level::WALL;
//         level.map[13][i] = Level::WALL;
//     }

//     // --- Room dividers (no overlapping assignments) ---
//     // Wall A: vertical at x=5 (y=1..12), locked door at y=6
//     for (int y = 1; y <= 12; ++y)
//         level.map[5][y] = (y == 6) ? Level::LOCKED : Level::WALL;
//     lockedDoorPositions.append({5, 6});

//     // Wall B: vertical at x=9 (y=1..12), locked door at y=6
//     for (int y = 1; y <= 12; ++y)
//         level.map[9][y] = (y == 6) ? Level::LOCKED : Level::WALL;
//     lockedDoorPositions.append({9, 6});

//     // --- Keys (2 in each of the first two sections) ---
//     level.map[3][3]  = Level::KEY_TILE; keyPositions.append({3,  3});
//     level.map[3][10] = Level::KEY_TILE; keyPositions.append({3, 10});
//     level.map[7][3]  = Level::KEY_TILE; keyPositions.append({7,  3});
//     level.map[7][9]  = Level::KEY_TILE; keyPositions.append({7,  9});

//     // --- Traps (none overlap keys/walls) ---
//     const int trapCoords[][2] = {{2,8},{4,2},{6,8},{8,2},{11,5},{11,9},{3,12},{6,12}};
//     for (auto& tc : trapCoords)
//     {
//         trapPositions.append({tc[0], tc[1]});
//         enemies.append(new TrapEnemy(tc[0], tc[1], (int)enemies.size()));
//     }

//     // --- Chest with health potion ---
//     level.map[12][11] = Level::CHEST;
//     chestPositions.append({12, 11});

//     // --- Dungeon stairs (goal) ---
//     level.map[12][2] = Level::STAIRS;

//     g_levelPtr = &level;
//     phase = Phase::LEVEL4_NAVIGATE;

//     player.setX(1); player.setY(6);
//     player.resetForLevel(1, 6);

//     storyMessages.clear();
//     storyMessages.append("Inside the castle. Find 4 keys and open 2 locked doors to reach the dungeon stairs!");
//     storyMessages.append("Hidden floor traps lurk everywhere. Use P for health potions!");
//     storyMsgTimer = 220;
// }

void Game::buildLevel4()
{
    clearEnemies();
    projectiles.clear();
    damageCooldown = 0;
    flashEvents.clear();
    for (int i = 0; i < 4; ++i) doorsOpened[i] = false;
    trapPositions.clear();
    keyPositions.clear();
    lockedDoorPositions.clear();
    chestPositions.clear();

    level = Level(14);

    // Fill everything as wall first
    for (int x = 0; x < 14; x++)
        for (int y = 0; y < 14; y++)
            level.map[x][y] = Level::WALL;

    // Upper-left room (cols 1-3, rows 1-5)
    for (int x = 1; x <= 3; x++)
        for (int y = 1; y <= 5; y++)
            level.map[x][y] = Level::FLOOR;

    // Open passage at (2,6) — only connection between upper/lower left rooms
    level.map[2][6] = Level::FLOOR;

    // Lower-left room (cols 1-3, rows 8-12)
    for (int x = 1; x <= 3; x++)
        for (int y = 8; y <= 12; y++)
            level.map[x][y] = Level::FLOOR;

    // Horizontal hallway at y=7 connecting all three rooms
    for (int x = 1; x <= 12; x++)
        level.map[x][7] = Level::FLOOR;

    // Center room (cols 5-9, rows 1-6 and 8-12)
    for (int x = 5; x <= 9; x++)
        for (int y = 1; y <= 6; y++)
            level.map[x][y] = Level::FLOOR;
    for (int x = 5; x <= 9; x++)
        for (int y = 8; y <= 12; y++)
            level.map[x][y] = Level::FLOOR;

    // Right room (cols 11-12, rows 1-6 and 8-12)
    for (int x = 11; x <= 12; x++)
        for (int y = 1; y <= 6; y++)
            level.map[x][y] = Level::FLOOR;
    for (int x = 11; x <= 12; x++)
        for (int y = 8; y <= 12; y++)
            level.map[x][y] = Level::FLOOR;

    // Key in center of center room
    level.map[7][3] = Level::KEY_TILE;
    keyPositions.append({7, 3});

    // Stairs at right end of hallway
    level.map[12][7] = Level::STAIRS;

    // 4 traps — visible at start, hidden after safeHintTimer expires
    const QPoint trapCoords[] = { {6,7}, {8,7}, {2,9}, {3,12} };
    for (int i = 0; i < 4; i++) {
        trapPositions.append(trapCoords[i]);
        enemies.append(new TrapEnemy(trapCoords[i].x(), trapCoords[i].y(), i));
    }

    // Guards
    enemies.append(new PatrolEnemy(1, 8,  {{1,8},{3,8},{3,12},{1,12}},  10));
    enemies.append(new PatrolEnemy(1, 1,  {{1,1},{3,1},{3,5},{1,5}},    11));
    enemies.append(new PatrolEnemy(5, 1,  {{5,1},{9,1},{9,6},{5,6}},    12));

    g_levelPtr = &level;
    phase = Phase::LEVEL4_NAVIGATE;

    player.setX(1); player.setY(12);
    player.resetForLevel(1, 12);

    alarmActive = false;
    guardsChasing = false;


    storyMessages.clear();
    storyMessages.append("Castle Lockdown! The key is in the center room.");
    storyMessages.append("Traps will vanish soon — memorize their positions!");
    storyMsgTimer = 220;
}

void Game::triggerAlarm()
{
    alarmActive = true;
    storyMessages.clear();
    storyMessages.append("ALARM! Guards alerted — brace yourself!");
    storyMsgTimer = 180;
    activateGuardChase();
}

void Game::activateGuardChase()
{
    guardsChasing = true;
    for (Enemy *e : enemies)
        e->onAlarm();
    storyMessages.clear();
    storyMessages.append("Guards are hunting you! Reach the stairs!");
    storyMsgTimer = 200;
}


void Game::buildLevel5()
{
    clearEnemies();
    projectiles.clear();
    damageCooldown = 0;
    flashEvents.clear();
    cellUnlocked  = false;
    dragonDefeated = false;
    hasSword = false;
    megaFireWarningActive = false;
    alarmActive    = false;
    guardsChasing  = false;
    trapPositions.clear();


    // 14x14 dungeon — fill the entire grid (not just 12 rows) to avoid leftover tiles
    level = Level(14);
    for (int x = 0; x < 14; ++x)
        for (int y = 0; y < 14; ++y)
            level.map[x][y] = Level::FLOOR;

    // Outer walls
    for (int i = 0; i < 14; ++i)
    {
        level.map[i][0]  = Level::WALL;
        level.map[i][13] = Level::WALL;
        level.map[0][i]  = Level::WALL;
        level.map[13][i] = Level::WALL;
    }

    // Pillars for cover
    level.map[3][3] = Level::WALL;
    level.map[3][9] = Level::WALL;
    level.map[6][5] = Level::WALL;
    level.map[6][8] = Level::WALL;

    // Fire pits (block player, not dragon)
    level.map[4][6] = Level::FIRE;
    level.map[5][5] = Level::FIRE;
    level.map[8][8] = Level::FIRE;

    // Prison cell dividing wall at x=11 (y=2..11), locked door at y=6
    for (int y = 2; y <= 11; ++y)
        level.map[11][y] = (y == 6) ? Level::LOCKED : Level::WALL;
    level.map[12][5] = Level::CELL;
    level.map[12][6] = Level::CELL;
    level.map[12][7] = Level::CELL;

    // Sword at center-bottom — guarded by traps
    level.map[7][12] = Level::SWORD_TILE;

    // 6 floor traps (attackPower=40), visible 5s then hidden
    const QPoint trapCoords5[] = { {4,10},{6,10},{8,10},{5,11},{7,11},{9,11} };
    for (int i = 0; i < 6; i++) {
        trapPositions.append(trapCoords5[i]);
        enemies.append(new TrapEnemy(trapCoords5[i].x(), trapCoords5[i].y(), i, 40));
    }

    // Key is dropped by the dragon when defeated (not pre-placed)

    g_levelPtr = &level;
    phase = Phase::LEVEL5_DRAGON;

    player.setX(1); player.setY(6);
    player.resetForLevel(1, 6);

    // Dragon starts in the centre of the dungeon
    enemies.append(new DragonEnemy(7, 6));

    storyMessages.clear();
    storyMessages.append("The dungeon reeks of sulfur. Drakoroth guards the cell!");
    storyMessages.append("Aldric calls out: 'Help! The dragon holds the key to my cell!'");
    storyMessages.append("Find the Dungeon Slayer Sword — only it can harm the dragon!");
    storyMsgTimer = 240;
}

// -------------------------------------------------------
// Walkable helper
// -------------------------------------------------------

bool Game::levelWalkable(int x, int y) const
{
    return level.isWalkable(x, y);
}

// -------------------------------------------------------
// Move player
// -------------------------------------------------------

void Game::movePlayer(int dx, int dy)
{
    if (dx == 0 && dy == 0) return;
    if (phase == Phase::LEVEL2_RIDDLE) return;
    if (damageCooldown > 0) damageCooldown--;

    const int nx = player.getX() + dx;
    const int ny = player.getY() + dy;
    if (!levelWalkable(nx, ny)) return;

    player.move(dx, dy);
    ++turns;

    const int tile = level.map[player.getX()][player.getY()];

    // Key pickup
    if (tile == Level::KEY_TILE)
    {
        player.addKey();
        level.map[player.getX()][player.getY()] = Level::FLOOR;
        flashEvents.append({player.getX(), player.getY(), 20, "treasure"});
        storyMessages.prepend("You found a key!");
        storyMsgTimer = 120;
    }

    // Sword pickup
    if (tile == Level::SWORD_TILE)
    {
        hasSword = true;
        player.setAttackPower(player.getAttackPower() + 20);
        level.map[player.getX()][player.getY()] = Level::FLOOR;
        flashEvents.append({player.getX(), player.getY(), 20, "treasure"});
        storyMessages.prepend("Dungeon Slayer Sword obtained! +20 Attack!");
        storyMsgTimer = 180;
    }

    // Chest pickup
    if (tile == Level::CHEST)
    {
        player.addPotion();
        level.map[player.getX()][player.getY()] = Level::FLOOR;
        flashEvents.append({player.getX(), player.getY(), 20, "treasure"});
        storyMessages.prepend("Found a health potion in the chest!");
        storyMsgTimer = 120;
    }

    // Level 4: locked door unlock + traps
    // if (phase == Phase::LEVEL4_NAVIGATE)
    // {
    //     for (int i = 0; i < lockedDoorPositions.size(); ++i)
    //     {
    //         const QPoint &dp = lockedDoorPositions[i];
    //         if (abs(dp.x() - player.getX()) + abs(dp.y() - player.getY()) == 1)
    //         {
    //             if (player.getKeys() > 0 && !doorsOpened[i])
    //             {
    //                 player.useKey();
    //                 doorsOpened[i] = true;
    //                 level.map[dp.x()][dp.y()] = Level::DOOR;
    //                 storyMessages.prepend("Door unlocked!");
    //                 storyMsgTimer = 120;
    //             }
    //         }
    //     }

    //     for (auto *e : enemies)
    //     {
    //         if (e->getType() == EnemyType::TRAP && !e->isDefeated())
    //         {
    //             TrapEnemy *trap = static_cast<TrapEnemy*>(e);
    //             if (!trap->isTriggered() &&
    //                 e->getX() == player.getX() && e->getY() == player.getY())
    //             {
    //                 trap->trigger();
    //                 if (damageCooldown == 0)
    //                 {
    //                     player.takeDamage(trap->getAttackPower());
    //                     damageCooldown = 30;
    //                     flashEvents.append({player.getX(), player.getY(), 25, "hit"});
    //                     storyMessages.prepend("TRAP! You take " +
    //                                           QString::number(trap->getAttackPower()) + " damage!");
    //                     storyMsgTimer = 150;
    //                 }
    //             }
    //         }
    //     }
    // }
    if (phase == Phase::LEVEL4_NAVIGATE && tile == Level::KEY_TILE && !alarmActive)
        triggerAlarm();

    if (phase == Phase::LEVEL4_NAVIGATE || phase == Phase::LEVEL5_DRAGON)
    {
        for (auto *e : enemies) {
            if (e->getType() == EnemyType::TRAP && !e->isDefeated()) {
                TrapEnemy *trap = static_cast<TrapEnemy*>(e);
                if (!trap->isTriggered() &&
                    e->getX() == player.getX() && e->getY() == player.getY())
                {
                    trap->trigger();
                    if (damageCooldown == 0) {
                        player.takeDamage(trap->getAttackPower());
                        damageCooldown = 30;
                        flashEvents.append({player.getX(), player.getY(), 25, "hit"});
                        storyMessages.prepend("TRAP! " + QString::number(trap->getAttackPower()) + " damage!");
                        storyMsgTimer = 150;
                    }
                }
            }
        }
    }

    // Level 5: unlock cell door when adjacent and holding dragon key
    if (phase == Phase::LEVEL5_DRAGON && dragonDefeated && player.getKeys() > 0)
    {
        // // Cell door is at (11, 6)
        // if (abs(11 - player.getX()) + abs(6 - player.getY()) <= 1)
        // {
        if (tile == Level::KEY_TILE || player.getKeys() > 0)
        {
            if (abs(11 - player.getX()) + abs(6 - player.getY()) <= 1 && player.getKeys() > 0)
            {
            player.useKey();
            cellUnlocked = true;
            level.map[11][6] = Level::DOOR;
            storyMessages.prepend("The cell is open! Aldric is free!");
            storyMsgTimer = 200;
            }
        }
    }

    // Level 1 story hints
    if (currentLevel == 1)
    {
        if (tile == Level::BRIDGE && storyState < 1) { storyState = 1; }
        else if (tile == Level::PATH && player.getX() >= 6 && storyState < 2) { storyState = 2; }
    }
}

// -------------------------------------------------------
// Player attack
// -------------------------------------------------------

void Game::playerAttack()
{
    if (phase == Phase::LEVEL2_RIDDLE) return;

    const int px   = player.getX();
    const int py   = player.getY();
    const QString role = player.getRole();

    for (auto *e : enemies)
    {
        if (e->isDefeated()) continue;

        int dist = abs(e->getX() - px) + abs(e->getY() - py);
        int attackRange = (role == "Wizard") ? 3 : 1;

        if (dist <= attackRange)
        {
            // Dragon immune without sword
            if (e->getType() == EnemyType::DRAGON && !hasSword)
            {
                storyMessages.prepend("Your weapon cannot harm the dragon! Find the Dungeon Slayer Sword!");
                storyMsgTimer = 160;
                break;
            }

            int dmg = player.getAttackPower();
            if (role == "Rogue" && rand() % 3 == 0) dmg *= 2;
            if (role == "Cleric") dmg += 5;

            e->takeDamage(dmg);
            flashEvents.append({e->getX(), e->getY(), 20, "hit"});
            player.addScore(dmg);

            if (!e->isAlive())
            {
                e->defeat();
                flashEvents.append({e->getX(), e->getY(), 30, "defeat"});
                player.addScore(100);

                if (e->getType() == EnemyType::DRAGON)
                {
                    dragonDefeated = true;
                    megaFireWarningActive = false;
                    projectiles.clear();
                    // Drop the cell key at the dragon's position
                    level.map[e->getX()][e->getY()] = Level::KEY_TILE;
                    storyMessages.prepend("Drakoroth falls! A key clatters to the ground!");
                    storyMsgTimer = 200;
                }
                else if (e->getType() == EnemyType::WITCH)
                {
                    storyMessages.prepend("The witch is defeated! Take the stairs to continue.");
                    storyMsgTimer = 200;
                }
                else if (e->getType() == EnemyType::ARCHER)
                {
                    storyMessages.prepend("Archer defeated!");
                    storyMsgTimer = 100;
                }
            }
            break;
        }
    }
}

// -------------------------------------------------------
// Update enemies
// -------------------------------------------------------

void Game::updateEnemies()
{
    g_levelPtr = &level;
    const int px = player.getX();
    const int py = player.getY();

    for (auto *e : enemies)
    {
        if (e->isDefeated()) continue;

        switch (e->getType())
        {
        case EnemyType::SHADOW:
        {
            e->moveToward(px, py, staticWalkable);
            if (e->getX() == px && e->getY() == py && damageCooldown == 0)
            {
                player.takeDamage(e->getAttackPower());
                damageCooldown = 40;
                flashEvents.append({px, py, 25, "hit"});
            }
            break;
        }
        case EnemyType::WITCH:
        {
            WitchEnemy *witch = static_cast<WitchEnemy*>(e);
            if (!witch->isRiddleSolved() && phase == Phase::LEVEL2_COMBAT)
            {
                int dist = abs(e->getX() - px) + abs(e->getY() - py);
                if (dist <= 3 && damageCooldown == 0)
                {
                    player.takeDamage(e->getAttackPower());
                    damageCooldown = 50;
                    flashEvents.append({px, py, 25, "hit"});
                }
            }
            break;
        }
        case EnemyType::ARCHER:
        {
            ArcherEnemy *archer = static_cast<ArcherEnemy*>(e);
            archer->tickCooldown();
            if (archer->canAttackFrom(px, py) && archer->readyToShoot())
            {
                Projectile proj;
                proj.x = (float)e->getX(); proj.y = (float)e->getY();
                float len = (float)std::max(1, abs(px-e->getX()) + abs(py-e->getY()));
                proj.dx = (px - e->getX()) / len;
                proj.dy = (py - e->getY()) / len;
                proj.damage = e->getAttackPower();
                proj.type   = "arrow";
                proj.active = true;
                projectiles.append(proj);
                archer->resetCooldown();
                storyMessages.prepend("An archer fires at you!");
                storyMsgTimer = 80;
            }
            break;
        }
        case EnemyType::TRAP:
            break; // handled in movePlayer
        case EnemyType::DRAGON:
        {
            // DragonEnemy *dragon = static_cast<DragonEnemy*>(e);
            // dragon->updatePhase();
            // //dragon->tickBreathCooldown();
            // e->moveToward(px, py, staticWalkable);

            // if (e->getX() == px && e->getY() == py && damageCooldown == 0)
            // {
            //     player.takeDamage(e->getAttackPower());
            //     damageCooldown = 35;
            //     flashEvents.append({px, py, 25, "hit"});
            // }

            // if (dragon->canBreatheFire(px, py) && dragon->readyToBreath())
            // {
            //     Projectile fire;
            //     fire.x = (float)e->getX(); fire.y = (float)e->getY();
            //     float len = std::max(1.0f, sqrtf(
            //                                    powf((float)(px - e->getX()), 2) + powf((float)(py - e->getY()), 2)));
            //     fire.dx     = (px - e->getX()) / len;
            //     fire.dy     = (py - e->getY()) / len;
            //     fire.damage = (dragon->getPhase() == 2) ? 35 : 20;
            //     fire.type   = "fireball";
            //     fire.active = true;
            //     projectiles.append(fire);
            //     dragon->resetBreathCooldown();
            // }
            // break;
            DragonEnemy *dragon = static_cast<DragonEnemy*>(e);
            dragon->updatePhase();
            e->moveToward(px, py, staticWalkable);

            if (dragon->getMegaFireCountdown() > 0)
            {
                dragon->decrementMegaFireCountdown();
                if (dragon->getMegaFireCountdown() == 0)
                {
                    // Megafire fires — flash all 9 tiles
                    QPoint center = dragon->getMegaFireCenter();
                    for (int ddx = -1; ddx <= 1; ddx++)
                        for (int ddy = -1; ddy <= 1; ddy++)
                            flashEvents.append({center.x()+ddx, center.y()+ddy, 40, "megafire"});
                    // Damage player if still in blast zone (bypasses damageCooldown)
                    if (abs(px - center.x()) <= 1 && abs(py - center.y()) <= 1)
                    {
                        player.takeDamage(50);
                        damageCooldown = 35;
                        flashEvents.append({px, py, 30, "hit"});
                        storyMessages.prepend("MEGAFIRE hits you for 50 damage!");
                        storyMsgTimer = 200;
                    }
                    else
                    {
                        storyMessages.prepend("You dodged Drakoroth's megafire!");
                        storyMsgTimer = 150;
                    }
                    dragon->resetHitCount();
                    megaFireWarningActive = false;
                }
                // Dragon is charging — no normal attacks this turn
            }
            else
            {
                // Normal proximity attack
                if (damageCooldown > 0) damageCooldown--;
                int ddx = abs(px - e->getX()), ddy = abs(py - e->getY());
                if (ddx <= 4 && ddy <= 4 && damageCooldown == 0)
                {
                    int dmg = (ddx == 0 && ddy == 0) ? 40 : 30;
                    player.takeDamage(dmg);
                    damageCooldown = 8;
                    flashEvents.append({px, py, 25, "hit"});
                    storyMessages.prepend((ddx == 0 && ddy == 0)
                                              ? "Dragon claws you for 40 damage!"
                                              : "Dragon's heat scorches you for 30 damage!");
                    storyMsgTimer = 120;
                    dragon->incrementHitCount();
                    if (dragon->getNormalHitCount() >= 3)
                    {
                        dragon->chargeMegaFire(px, py);
                        megaFireWarningActive = true;
                        storyMessages.prepend("Drakoroth charges a devastating fireball — MOVE!");
                        storyMsgTimer = 200;
                    }
                }
            }
            break;
        }
        case EnemyType::PATROL:
        {
            // e->moveToward(0, 0, staticWalkable); // PatrolEnemy ignores target
            // if (e->getX() == px && e->getY() == py && damageCooldown == 0) {
            //     player.takeDamage(e->getAttackPower());
            //     damageCooldown = 40;
            //     flashEvents.append({px, py, 25, "hit"});
            //     storyMessages.prepend("A guard strikes you!");
            //     storyMsgTimer = 100;
            // }
            // break;
            e->moveToward(px, py, staticWalkable); // px,py used when chasing
            if (e->getX() == px && e->getY() == py && damageCooldown == 0) {
                player.takeDamage(e->getAttackPower());
                damageCooldown = 40;
                flashEvents.append({px, py, 25, "hit"});
                storyMessages.prepend("A guard strikes you!");
                storyMsgTimer = 100;
            }
            break;
        }
        }
    }
}

// -------------------------------------------------------
// Projectiles
// -------------------------------------------------------

void Game::updateProjectiles()
{
    const int px = player.getX();
    const int py = player.getY();

    for (auto &proj : projectiles)
    {
        if (!proj.active) continue;
        proj.x += proj.dx * 0.4f;
        proj.y += proj.dy * 0.4f;

        int ix = (int)round(proj.x);
        int iy = (int)round(proj.y);

        if (!levelWalkable(ix, iy)) { proj.active = false; continue; }

        if (abs(ix - px) <= 1 && abs(iy - py) <= 1 && damageCooldown == 0)
        {
            player.takeDamage(proj.damage);
            damageCooldown = 30;
            flashEvents.append({px, py, 25, "hit"});
            proj.active = false;
        }

        if (ix < 0 || iy < 0 || ix >= level.gridSize || iy >= level.gridSize)
            proj.active = false;
    }

    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
                       [](const Projectile &p){ return !p.active; }),
        projectiles.end());
}

// -------------------------------------------------------
// Win / Lose
// -------------------------------------------------------

bool Game::checkWin()
{
    switch (phase)
    {
    case Phase::LEVEL1_EXPLORE:
        return level.map[player.getX()][player.getY()] == Level::COTTAGE;

    case Phase::LEVEL2_RIDDLE:
        return false;
    case Phase::LEVEL2_COMBAT:
    {
        bool witchDone = true;
        for (auto *e : enemies)
            if (e->getType() == EnemyType::WITCH && !e->isDefeated())
            { witchDone = false; break; }
        return witchDone && level.map[player.getX()][player.getY()] == Level::STAIRS;
    }
    case Phase::LEVEL3_INFILTRATE:
    {
        bool archersDone = true;
        for (auto *e : enemies)
            if (e->getType() == EnemyType::ARCHER && !e->isDefeated())
            { archersDone = false; break; }
        return archersDone && level.map[player.getX()][player.getY()] == Level::STAIRS;
    }
    // case Phase::LEVEL4_NAVIGATE:
    //     return level.map[player.getX()][player.getY()] == Level::STAIRS;
    case Phase::LEVEL4_NAVIGATE:
        return player.getKeys() > 0 &&
               level.map[player.getX()][player.getY()] == Level::STAIRS;

    case Phase::LEVEL5_DRAGON:
        return cellUnlocked && level.map[player.getX()][player.getY()] == Level::CELL;

    default:
        return false;
    }
}

bool Game::checkLose()
{
    return !player.isAlive();
}

// -------------------------------------------------------
// Riddle system (Level 2 — implemented by partner)
// -------------------------------------------------------

struct Riddle { QString question; QVector<QString> choices; int correct; QString roleLore[4]; };

static const Riddle RIDDLES[3] = {
    {
        "I speak without mouth, heard without ears,\nhave no body but come alive with wind.\nWhat am I?",
        {"A ghost", "An echo", "A shadow", "A dream"}, 1,
        {"Cast Echo Spell!", "Shield Bash — 'ECHO!'", "Shadow Step into silence", "Bless — 'Sacred Echo!'"}
    },
    {
        "The more you take, the more you leave behind.\nWhat am I?",
        {"Memories", "Footsteps", "Time", "Gold"}, 1,
        {"Arcane Footstep!", "Warrior Stride!", "Rogue Sprint!", "Holy March!"}
    },
    {
        "What has roots as nobody sees,\nis taller than trees, up up it goes\nand yet never grows?",
        {"A mountain", "A tower", "A river", "A storm"}, 0,
        {"Mountain Crush!", "Shield Wall!", "Vanish Strike!", "Earth Bless!"}
    }
};

QString          Game::getRiddleQuestion()  const { return RIDDLES[riddleIndex].question; }
QVector<QString> Game::getRiddleChoices()   const { return RIDDLES[riddleIndex].choices; }
bool             Game::isRiddleActive()     const { return phase == Phase::LEVEL2_RIDDLE; }
bool             Game::getRiddleCorrect()   const { return riddleCorrect; }
int              Game::getRiddleAttempts()  const { return riddleAttempts; }

void Game::answerRiddle(int choiceIndex)
{
    riddleAttempts++;
    if (choiceIndex == RIDDLES[riddleIndex].correct)
    {
        riddleCorrect  = true;
        riddleAnswered = true;

        int roleIdx = 0;
        const QString role = player.getRole();
        if (role == "Wizard")   roleIdx = 0;
        else if (role == "Fighter") roleIdx = 1;
        else if (role == "Rogue")   roleIdx = 2;
        else if (role == "Cleric")  roleIdx = 3;

        player.addItem("spell_" + QString::number(riddleIndex));
        player.setAttackPower(player.getAttackPower() + 15);

        storyMessages.clear();
        storyMessages.append("Correct! You use the " +
                             QString(RIDDLES[riddleIndex].roleLore[roleIdx]));
        storyMessages.append("The witch is weakened! Attack or reach the stairs!");
        storyMsgTimer = 220;

        for (auto *e : enemies)
            if (e->getType() == EnemyType::WITCH)
            {
                WitchEnemy *w = static_cast<WitchEnemy*>(e);
                w->solveRiddle();
                w->takeDamage(50);
                break;
            }
        phase = Phase::LEVEL2_COMBAT;
    }
    else
    {
        riddleAnswered = false;
        storyMessages.clear();
        storyMessages.append("Wrong! The witch blasts you!");
        storyMsgTimer = 140;
        player.takeDamage(20);
        flashEvents.append({player.getX(), player.getY(), 25, "hit"});
        if (riddleAttempts >= 3)
        {
            phase = Phase::LEVEL2_COMBAT;
            storyMessages.prepend("Out of chances! Face her wrath!");
        }
    }
}

// -------------------------------------------------------
// Potions
// -------------------------------------------------------

bool Game::usePotion() { return player.usePotion(); }


// -------------------------------------------------------
// Reinit (safe full reset — avoids the Game = Game() copy bug)
// -------------------------------------------------------

void Game::reinit()
{
    clearEnemies();
    projectiles.clear();
    flashEvents.clear();
    storyMessages.clear();
    for (int i = 0; i < 4; ++i) doorsOpened[i] = false;
    trapPositions.clear();
    keyPositions.clear();
    lockedDoorPositions.clear();
    chestPositions.clear();
    alarmActive = false;
    guardsChasing = false;
    cellUnlocked    = false;
    dragonDefeated  = false;
    hasSword = false;
    megaFireWarningActive = false;
    damageCooldown  = 0;
    storyMsgTimer   = 0;
    riddleIndex     = 0;
    riddleAnswered  = false;
    riddleCorrect   = false;
    riddleAttempts  = 0;
    currentLevel    = 1;
    storyState      = 0;
    turns           = 0;
    player = Player(); // Player has no owning pointers — copy is safe
    buildLevel1();
}

// -------------------------------------------------------
// Restart current level (loss mechanic)
// -------------------------------------------------------

void Game::restartCurrentLevel()
{
    switch (currentLevel)
    {
    case 1: buildLevel1(); break;
    case 4: buildLevel4(); break;
    case 5: buildLevel5(); break;
    default: buildLevel1(); break;
    }
    // Full health restore so the restart is always fair
    player.heal(player.getMaxHealth());
}
// -------------------------------------------------------
// Level transition
// -------------------------------------------------------

void Game::advanceToNextLevel()
{
    player.addScore(500 * currentLevel);

    // Skip levels 2 & 3 until partner implements them
    if (currentLevel == 1)
        currentLevel = 4;
    else
        currentLevel++;

    switch (currentLevel)
    {
    case 4: buildLevel4(); break;
    case 5: buildLevel5(); break;
    default:
        phase = Phase::VICTORY;
        break;
    }
}

// -------------------------------------------------------
// Getters
// -------------------------------------------------------

Game::Phase                    Game::getPhase()          const { return phase; }
int                            Game::getCurrentLevel()   const { return currentLevel; }
Player                        &Game::getPlayer()               { return player; }
const QVector<Enemy*>         &Game::getEnemies()        const { return enemies; }
Level                         &Game::getLevel()               { return level; }
const QVector<Projectile>     &Game::getProjectiles()    const { return projectiles; }
const QVector<FlashEvent>     &Game::getFlashEvents()    const { return flashEvents; }
bool                           Game::isDoorOpen(int i)   const { return (i>=0&&i<4)?doorsOpened[i]:false; }
bool                           Game::isCellUnlocked()    const { return cellUnlocked; }
bool   Game::playerHasSword()       const { return hasSword; }
bool   Game::isMegaFireWarning()    const { return megaFireWarningActive; }
QPoint Game::getMegaFireWarningCenter() const
{
    for (auto *e : enemies)
        if (e->getType() == EnemyType::DRAGON && !e->isDefeated())
            return static_cast<const DragonEnemy*>(e)->getMegaFireCenter();
    return QPoint(0, 0);
}

void Game::clearFlashEvents()
{
    for (auto &fe : flashEvents) if (fe.framesLeft > 0) fe.framesLeft--;
    flashEvents.erase(
        std::remove_if(flashEvents.begin(), flashEvents.end(),
                       [](const FlashEvent &f){ return f.framesLeft <= 0; }),
        flashEvents.end());
}

// -------------------------------------------------------
// Story messages
// -------------------------------------------------------

QString Game::storyHint() const
{
    switch (phase)
    {
    case Phase::LEVEL1_EXPLORE:
        if (storyState == 0) return "Find the bridge and follow the lantern path to the cottage.";
        if (storyState == 1) return "You crossed the old bridge. The cottage lights glow ahead...";
        return "Final stretch! Reach the cottage before the shadow catches you!";
    case Phase::LEVEL2_RIDDLE:
        return "Answer the riddle correctly to weaken the witch!";
    case Phase::LEVEL2_COMBAT:
        return "Witch weakened! Attack (Space) and reach the stairs to continue.";
    case Phase::LEVEL3_INFILTRATE:
        return "Defeat all archers (Space), then reach the castle gate.";
    // case Phase::LEVEL4_NAVIGATE:
    //     return "Find 4 keys, avoid traps, unlock 2 doors. Reach the dungeon stairs.";
    case Phase::LEVEL4_NAVIGATE:
        if (alarmActive)
            return "ALARM! Guards are hunting you — reach the stairs in time!";
        return "Reach the center room, grab the key, then race to the stairs.";
    case Phase::LEVEL5_DRAGON:
        return dragonDefeated
                   ? "Dragon slain! Take the key, open the cell to free Aldric!"
                   : "Fight Drakoroth! Space=Attack  P=Potion  Stay out of fire!";
    default: return "";
    }
}

QString Game::getLatestStoryMessage() const
{
    return storyMessages.isEmpty() ? "" : storyMessages.first();
}

void Game::tickStoryMessage()
{
    if (storyMsgTimer > 0) storyMsgTimer--;
    if (storyMsgTimer == 0 && !storyMessages.isEmpty())
    {
        storyMessages.removeFirst();
        if (!storyMessages.isEmpty()) storyMsgTimer = 160;
    }
}

bool Game::hasStoryMessage() const
{
    return !storyMessages.isEmpty() && storyMsgTimer > 0;
}

// -------------------------------------------------------
// Save / Load
// -------------------------------------------------------

Game::SaveState Game::getSaveState() const
{
    SaveState s;
    s.currentLevel  = currentLevel;
    s.phaseInt      = (int)phase;
    s.playerX       = player.getX();
    s.playerY       = player.getY();
    s.playerHealth  = player.getHealth();
    s.playerScore   = player.getScore();
    s.playerPotions = player.getPotions();
    s.playerKeys    = player.getKeys();
    s.playerName    = player.getName();
    s.playerRole    = player.getRole();
    for (auto *e : enemies)
        s.enemyHP.append(e->isDefeated() ? 0 : e->getHealth());
    for (int i = 0; i < 4; ++i) s.doorsOpened[i] = doorsOpened[i];
    s.cellUnlocked = cellUnlocked;
    return s;
}

void Game::loadSaveState(const SaveState &s)
{
    currentLevel = s.currentLevel;
    switch (currentLevel)
    {
    case 1: buildLevel1(); break;
    case 4: buildLevel4(); break;
    case 5: buildLevel5(); break;
    default: buildLevel1(); break;
    }

    player.setName(s.playerName);
    player.setRole(s.playerRole);
    player.setX(s.playerX);
    player.setY(s.playerY);
    player.takeDamage(player.getHealth());
    player.heal(s.playerHealth);

    phase = (Phase)s.phaseInt;
    for (int i = 0; i < 4; ++i) doorsOpened[i] = s.doorsOpened[i];
    cellUnlocked = s.cellUnlocked;

    for (int i = 0; i < std::min(s.enemyHP.size(), enemies.size()); ++i)
        if (s.enemyHP[i] == 0) enemies[i]->defeat();
}
