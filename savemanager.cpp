#include "savemanager.h"
#include <QFile>
#include <QDataStream>
#include <QDir>

static const quint32 MAGIC = 0xD8140001;

bool SaveManager::saveGame(const Game::SaveState &s, const QString &filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) return false;

    QDataStream ds(&file);
    ds.setVersion(QDataStream::Qt_6_0);

    ds << MAGIC;
    ds << (qint32)s.currentLevel;
    ds << (qint32)s.phaseInt;
    ds << (qint32)s.playerX << (qint32)s.playerY;
    ds << (qint32)s.playerHealth;
    ds << (qint32)s.playerScore;
    ds << (qint32)s.playerPotions;
    ds << (qint32)s.playerKeys;
    ds << s.playerName;
    ds << s.playerRole;

    ds << (qint32)s.enemyHP.size();
    for (int hp : s.enemyHP) ds << (qint32)hp;

    for (int i = 0; i < 4; ++i) ds << s.doorsOpened[i];
    ds << s.cellUnlocked;

    file.close();
    return true;
}

bool SaveManager::loadGame(Game::SaveState &s, const QString &filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QDataStream ds(&file);
    ds.setVersion(QDataStream::Qt_6_0);

    quint32 magic;
    ds >> magic;
    if (magic != MAGIC) { file.close(); return false; }

    qint32 tmp;
    ds >> tmp; s.currentLevel = tmp;
    ds >> tmp; s.phaseInt = tmp;
    ds >> tmp; s.playerX = tmp;
    ds >> tmp; s.playerY = tmp;
    ds >> tmp; s.playerHealth = tmp;
    ds >> tmp; s.playerScore = tmp;
    ds >> tmp; s.playerPotions = tmp;
    ds >> tmp; s.playerKeys = tmp;
    ds >> s.playerName;
    ds >> s.playerRole;

    qint32 numEnemies;
    ds >> numEnemies;
    s.enemyHP.clear();
    for (int i = 0; i < numEnemies; ++i)
    {
        qint32 hp; ds >> hp;
        s.enemyHP.append(hp);
    }

    for (int i = 0; i < 4; ++i) ds >> s.doorsOpened[i];
    ds >> s.cellUnlocked;

    file.close();
    return true;
}

bool SaveManager::hasSaveFile(const QString &filepath)
{
    return QFile::exists(filepath);
}

void SaveManager::deleteSave(const QString &filepath)
{
    QFile::remove(filepath);
}
