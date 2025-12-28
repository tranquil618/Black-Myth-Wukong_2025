#pragma once

#include "cocos2d.h" 
#include "EnemyType.h"

class EnemyBase;

class EnemyFactory
{
public:
    static EnemyBase* createEnemy(
        EnemyType type,
        const cocos2d::Vec3& position
    );
};