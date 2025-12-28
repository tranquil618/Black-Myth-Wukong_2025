#pragma once
#include "cocos2d.h"

class Player 
{
public:
    virtual ~Player() {}
    virtual void takeDamage(int damage) = 0;
    virtual cocos2d::Vec3 getPosition3D() const = 0;
    virtual void attackEnemy(class EnemyBase* enemy) = 0;
};