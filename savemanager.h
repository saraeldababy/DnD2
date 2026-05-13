#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include "game.h"
#include <QString>

class SaveManager
{
public:
    static bool saveGame(const Game::SaveState &state, const QString &filepath = "dungeon_save.dat");
    static bool loadGame(Game::SaveState &state, const QString &filepath = "dungeon_save.dat");
    static bool hasSaveFile(const QString &filepath = "dungeon_save.dat");
    static void deleteSave(const QString &filepath = "dungeon_save.dat");
};

#endif
