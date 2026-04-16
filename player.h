// #ifndef PLAYER_H
// #define PLAYER_H

// #pragma once
// #include "character.h"

// class Player : public Character
// {
// //4PM
// // private:
// //     int health; //HEALTH

// public:
//     Player(int x = 0, int y = 0);

//     void move(int dx, int dy);

//     int getHealth() const;    //HEALTH
//     void takeDamage(int dmg);//HEALTH
// };
// #endif // PLAYER_H

#ifndef PLAYER_H
#define PLAYER_H

#include <QString>

class Player
{
private:
    int x, y;
    int health;
    QString role;

public:
    Player();

    int getX() const;
    int getY() const;

    void setX(int);
    void setY(int);
    void move(int dx, int dy);
    QString getRole() const;
    void setRole(const QString &r);
};

#endif
