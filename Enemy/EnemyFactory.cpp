#include "EnemyFactory.h"
#include "EnemyGoblin.h"
#include "EnemyMinotaur.h"
#include "EnemyKnight.h"

USING_NS_CC;

EnemyBase* EnemyFactory::createEnemy(
    EnemyType type,
    const Vec3& position)
{
    EnemyBase* enemy = nullptr;

    switch (type)
    {
    case EnemyType::GOBLIN:
        enemy = EnemyGoblin::create();
        break;

    case EnemyType::MINOTAUR:
        enemy = EnemyMinotaur::create();
        break;

    case EnemyType::KNIGHT:
        enemy = EnemyKnight::create();
        break;

    default:
        break;
    }

    if (enemy)
    {
        enemy->setPosition3D(position);
    }

    return enemy;
}