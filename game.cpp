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
    witchScene(nullptr),
    leverProgress(0), leverStrikes(0), leverGateOpen(false),
    cellUnlocked(false), dragonDefeated(false),
    hasSword(false), megaFireWarningActive(false),
    alarmActive(false), guardsChasing(false),
    damageCooldown(0), storyMsgTimer(0)
{
    for (int i = 0; i < 4; ++i) doorsOpened[i] = false;
    for (int i = 0; i < 4; ++i) leverLocked[i] = false;
    buildLevel1();
}

Game::~Game() { clearEnemies(); delete witchScene; }

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
    clearEnemies();
    projectiles.clear();
    flashEvents.clear();
    damageCooldown = 0;

    // 10×10 grid — corridor uses rows 0-4, rest are walls
    level = Level(10);
    g_levelPtr = &level;

    for (int x = 0; x < 10; ++x)
        for (int y = 0; y < 10; ++y)
            level.map[x][y] = Level::WALL;

    // Corridor interior: rows 1-3
    for (int x = 1; x <= 8; ++x)
        for (int y = 1; y <= 3; ++y)
            level.map[x][y] = Level::FLOOR;

    // Outer corridor walls
    for (int x = 0; x < 10; ++x) {
        level.map[x][0] = Level::WALL;
        level.map[x][4] = Level::WALL;
    }
    for (int y = 1; y <= 3; ++y) {
        level.map[0][y] = Level::WALL;
        level.map[9][y] = Level::WALL;
    }

    // Door to witch room (right wall, centre)
    level.map[9][2] = Level::DOOR;

    // Chests (contain potions)
    level.map[3][1] = Level::CHEST;
    level.map[4][3] = Level::CHEST;

    // Spike traps
    enemies.append(new TrapEnemy(5, 2, 0, 20));
    enemies.append(new TrapEnemy(7, 3, 1, 20));

    delete witchScene;
    witchScene = new WitchScene(player.getRole());

    phase = Phase::LEVEL2_CORRIDOR;
    player.setX(1);
    player.setY(2);

    storyMessages.clear();
    storyMessages.append("The witch's cottage... navigate the corridor and find the door.");
    storyMessages.append("Chests hold potions. Watch for floor traps!");
    storyMsgTimer = 200;
}

void Game::buildLevel3()
{
    clearEnemies();
    projectiles.clear();
    flashEvents.clear();
    damageCooldown = 0;
    gargoyles.clear();

    // 14×14 gargoyle arena
    level = Level(14);
    g_levelPtr = &level;

    for (int x = 0; x < 14; ++x)
        for (int y = 0; y < 14; ++y)
            level.map[x][y] = Level::FLOOR;

    // Outer walls
    for (int i = 0; i < 14; ++i) {
        level.map[i][0]  = Level::WALL;
        level.map[i][13] = Level::WALL;
        level.map[0][i]  = Level::WALL;
        level.map[13][i] = Level::WALL;
    }

    // Pillars for cover
    level.map[4][4] = Level::WALL;
    level.map[4][9] = Level::WALL;
    level.map[7][4] = Level::WALL;
    level.map[7][9] = Level::WALL;

    // Three gargoyles on right side
    gargoyles.append({11, 2,  3, true});
    gargoyles.append({11, 7,  3, true});
    gargoyles.append({11, 11, 3, true});

    phase = Phase::LEVEL3_SPLASH;
    player.setX(2);
    player.setY(7);
    player.resetForLevel(2, 7);

    storyMessages.clear();
    storyMsgTimer = 0;
}

static QVector<int> correctLeverSequence(const QString &role)
{
    if (role == "Wizard")  return {1, 3, 0, 2};
    if (role == "Fighter") return {2, 0, 3, 1};
    if (role == "Rogue")   return {3, 2, 1, 0};
    return {0, 1, 2, 3}; // Cleric
}

void Game::buildLevel3Corridor()
{
    clearEnemies();

    // Rebuild the 14×14 map as a 7-row corridor
    for (int x = 0; x < 14; ++x)
        for (int y = 0; y < 14; ++y)
            level.map[x][y] = Level::WALL;

    // Walkable corridor rows 1-5
    for (int x = 1; x <= 12; ++x)
        for (int y = 1; y <= 5; ++y)
            level.map[x][y] = Level::FLOOR;

    // Gate (closed) at (12,3)
    level.map[12][3] = Level::LOCKED;

    // Spike traps as TrapEnemy on FLOOR tiles
    const QPoint spikes[] = {{4,1},{8,1},{3,3},{9,5},{5,5},{3,5},{10,3}};
    for (int i = 0; i < 7; ++i)
        enemies.append(new TrapEnemy(spikes[i].x(), spikes[i].y(), i, 20));

    // Levers — index matches correct-sequence logic
    level.map[1][1]  = Level::LEVER;   // index 0
    level.map[1][5]  = Level::LEVER;   // index 1
    level.map[11][5] = Level::LEVER;   // index 2
    level.map[11][1] = Level::LEVER;   // index 3

    // Reset lever state
    leverProgress = 0;
    leverStrikes  = 0;
    for (int i = 0; i < 4; ++i) leverLocked[i] = false;
    leverGateOpen = false;

    phase = Phase::LEVEL3_CORRIDOR;
    player.setX(1);
    player.setY(3);

    storyMessages.clear();
    storyMessages.append("Four levers — pull them in the order from the poem!");
    storyMessages.append("Three wrong pulls and the curse claims you.");
    storyMsgTimer = 200;
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
    if (phase == Phase::LEVEL2_WITCH_ROOM) return;
    if (phase == Phase::LEVEL3_SPLASH || phase == Phase::LEVEL3_BRIEFING || phase == Phase::LEVEL3_POEM) return;
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

    // Level 2: entering witch room via door
    if (phase == Phase::LEVEL2_CORRIDOR && tile == Level::DOOR)
    {
        phase = Phase::LEVEL2_WITCH_ROOM;
        if (witchScene) witchScene->advanceDialogue();
        storyMessages.clear();
        storyMsgTimer = 0;
    }

    // Level 3 corridor: lever interaction
    if (phase == Phase::LEVEL3_CORRIDOR && tile == Level::LEVER)
    {
        int lx = player.getX(), ly = player.getY();
        int leverIdx = -1;
        if (lx == 1  && ly == 1) leverIdx = 0;
        else if (lx == 1  && ly == 5) leverIdx = 1;
        else if (lx == 11 && ly == 5) leverIdx = 2;
        else if (lx == 11 && ly == 1) leverIdx = 3;
        if (leverIdx >= 0 && !leverLocked[leverIdx])
            pullLever(leverIdx);
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

    if (phase == Phase::LEVEL4_NAVIGATE || phase == Phase::LEVEL5_DRAGON || phase == Phase::LEVEL3_CORRIDOR)
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
    if (phase == Phase::LEVEL2_WITCH_ROOM) return;
    if (phase == Phase::LEVEL3_SPLASH || phase == Phase::LEVEL3_BRIEFING || phase == Phase::LEVEL3_POEM) return;

    // Level 3 gargoyle arena: SPACE = instant attack on nearest gargoyle
    if (phase == Phase::LEVEL3_GARGOYLE)
    {
        const int px = player.getX(), py = player.getY();
        int bestDist = 999999, bestIdx = -1;
        for (int i = 0; i < gargoyles.size(); ++i)
        {
            if (!gargoyles[i].alive) continue;
            int d = abs(gargoyles[i].x - px) + abs(gargoyles[i].y - py);
            if (d < bestDist) { bestDist = d; bestIdx = i; }
        }
        if (bestIdx >= 0)
        {
            gargoyles[bestIdx].hp--;
            flashEvents.append({gargoyles[bestIdx].x, gargoyles[bestIdx].y, 20, "hit"});
            player.addScore(10);
            if (gargoyles[bestIdx].hp <= 0)
            {
                gargoyles[bestIdx].alive = false;
                flashEvents.append({gargoyles[bestIdx].x, gargoyles[bestIdx].y, 30, "defeat"});
                player.addScore(100);
                storyMessages.prepend("Gargoyle destroyed!");
                storyMsgTimer = 100;
                bool allDead = true;
                for (auto &g : gargoyles) if (g.alive) { allDead = false; break; }
                if (allDead)
                {
                    phase = Phase::LEVEL3_POEM;
                    storyMessages.clear();
                    storyMessages.append("All gargoyles defeated! Study the poem and press Space...");
                    storyMsgTimer = 200;
                }
            }
        }
        return;
    }

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
            if (e->getX() == px && e->getY() == py)
            {
                // Instant kill — Level 1 is one-try, no HP
                player.takeDamage(player.getHealth());
                flashEvents.append({px, py, 25, "hit"});
                storyMessages.prepend("The shadow caught you!");
                storyMsgTimer = 100;
            }
            break;
        }
        case EnemyType::WITCH:
            break; // WitchEnemy not used in new Level 2
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
            e->moveToward(px, py, staticWalkable);
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

    // Gargoyle movement (Level 3 arena, turn-based — one step per player move)
    if (phase == Phase::LEVEL3_GARGOYLE)
    {
        for (auto &g : gargoyles)
        {
            if (!g.alive) continue;
            int dx = 0, dy = 0;
            if (g.x < px) dx = 1; else if (g.x > px) dx = -1;
            if (g.y < py) dy = 1; else if (g.y > py) dy = -1;
            // Prefer horizontal, then vertical
            if (dx != 0 && level.isWalkable(g.x + dx, g.y))
                g.x += dx;
            else if (dy != 0 && level.isWalkable(g.x, g.y + dy))
                g.y += dy;
            // Gargoyle reaches player
            if (g.x == px && g.y == py && damageCooldown == 0)
            {
                player.takeDamage(25);
                damageCooldown = 20;
                flashEvents.append({px, py, 25, "hit"});
                storyMessages.prepend("A gargoyle claws you for 25 damage!");
                storyMsgTimer = 120;
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

    case Phase::LEVEL2_CORRIDOR:
        return false; // transition to witch room happens in movePlayer

    case Phase::LEVEL2_WITCH_ROOM:
        return witchScene && witchScene->escaped();

    case Phase::LEVEL3_SPLASH:
    case Phase::LEVEL3_BRIEFING:
    case Phase::LEVEL3_GARGOYLE:
    case Phase::LEVEL3_POEM:
        return false;

    case Phase::LEVEL3_CORRIDOR:
        return leverGateOpen && level.map[player.getX()][player.getY()] == Level::DOOR;

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
// Level 2 witch / Level 3 lever — new systems
// -------------------------------------------------------

void Game::submitWitchAnswer(const QString &answer)
{
    if (!witchScene) return;
    witchScene->submitAnswer(answer);
    if (witchScene->escaped())
    {
        player.addScore(200);
        storyMessages.prepend("You solved the riddle! The witch releases you.");
        storyMsgTimer = 180;
    }
    else
    {
        flashEvents.append({player.getX(), player.getY(), 25, "hit"});
        if (witchScene->wrongAttempts() >= 3)
        {
            player.takeDamage(player.getHealth());
            storyMessages.prepend("Three wrong answers! The witch's curse claims you!");
            storyMsgTimer = 200;
        }
        else
        {
            player.takeDamage(25);
            storyMessages.prepend(QString("Wrong! (%1/3) — think harder...").arg(witchScene->wrongAttempts()));
            storyMsgTimer = 150;
        }
    }
}

void Game::advanceL3Phase()
{
    if      (phase == Phase::LEVEL3_SPLASH)   phase = Phase::LEVEL3_BRIEFING;
    else if (phase == Phase::LEVEL3_BRIEFING) phase = Phase::LEVEL3_GARGOYLE;
    else if (phase == Phase::LEVEL3_POEM)     buildLevel3Corridor();
}

void Game::pullLever(int leverIndex)
{
    const QVector<int> seq = correctLeverSequence(player.getRole());
    if (leverIndex == seq[leverProgress])
    {
        leverLocked[leverIndex] = true;
        leverProgress++;
        if (leverProgress == 4)
        {
            leverGateOpen = true;
            level.map[12][3] = Level::DOOR;
            player.addScore(300);
            storyMessages.prepend("★ All levers pulled! The gate opens — reach it! ★");
            storyMsgTimer = 200;
        }
        else
        {
            storyMessages.prepend(QString("Correct! Step %1/4 — keep going!").arg(leverProgress));
            storyMsgTimer = 120;
        }
    }
    else
    {
        leverStrikes++;
        leverProgress = 0;
        for (int i = 0; i < 4; ++i) leverLocked[i] = false;
        storyMessages.prepend(QString("WRONG LEVER! Strike %1/3!").arg(leverStrikes));
        storyMsgTimer = 150;
        if (leverStrikes >= 3)
        {
            player.takeDamage(player.getHealth());
            storyMessages.prepend("Three strikes! The curse destroys you!");
            storyMsgTimer = 200;
        }
    }
}

WitchScene*                Game::getWitchScene()    const { return witchScene; }
const QVector<Gargoyle>&   Game::getGargoyles()     const { return gargoyles; }
bool                       Game::isLeverLocked(int i) const { return (i>=0&&i<4)?leverLocked[i]:false; }
bool                       Game::isLeverGateOpen()  const { return leverGateOpen; }
int                        Game::getLeverProgress() const { return leverProgress; }
int                        Game::getLeverStrikes()  const { return leverStrikes; }

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
    delete witchScene; witchScene = nullptr;
    gargoyles.clear();
    leverProgress = 0;
    leverStrikes  = 0;
    for (int i = 0; i < 4; ++i) leverLocked[i] = false;
    leverGateOpen = false;
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
    case 2: buildLevel2(); break;
    case 3: buildLevel3(); break;
    case 4: buildLevel4(); break;
    case 5: buildLevel5(); break;
    default: buildLevel1(); break;
    }
    player.heal(player.getMaxHealth());
}
// -------------------------------------------------------
// Level transition
// -------------------------------------------------------

void Game::advanceToNextLevel()
{
    player.addScore(500 * currentLevel);
    currentLevel++;

    switch (currentLevel)
    {
    case 2: buildLevel2(); break;
    case 3: buildLevel3(); break;
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
    case Phase::LEVEL2_CORRIDOR:
        return "Navigate the corridor, avoid traps, reach the door on the right.";
    case Phase::LEVEL2_WITCH_ROOM:
        return "Answer the witch's riddle. Type your answer and press Enter.";
    case Phase::LEVEL3_SPLASH:
    case Phase::LEVEL3_BRIEFING:
    case Phase::LEVEL3_POEM:
        return "Press Space to continue...";
    case Phase::LEVEL3_GARGOYLE:
        return "Defeat all gargoyles! SPACE = attack nearest. Move to dodge!";
    case Phase::LEVEL3_CORRIDOR:
        return leverGateOpen ? "Gate is open! Reach it to escape!" : "Pull the 4 levers in the correct order from the poem.";
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
    case 2: buildLevel2(); break;
    case 3: buildLevel3(); break;
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
