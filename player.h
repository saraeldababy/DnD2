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
