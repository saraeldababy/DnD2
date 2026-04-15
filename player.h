#ifndef PLAYER_H
#define PLAYER_H

#pragma once
#include "character.h"

class Player : public Character
{
public:
    Player(int x = 0, int y = 0);

    void move(int dx, int dy);
};
#endif // PLAYER_H
