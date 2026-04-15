#ifndef PLAYER_H
#define PLAYER_H

#pragma once
#include "character.h"

class Player : public Character
{
//4PM
// private:
//     int health; //HEALTH

public:
    Player(int x = 0, int y = 0);

    void move(int dx, int dy);

    int getHealth() const;    //HEALTH
    void takeDamage(int dmg);//HEALTH
};
#endif // PLAYER_H
