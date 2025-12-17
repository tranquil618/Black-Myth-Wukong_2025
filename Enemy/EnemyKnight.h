#pragma once
#include "EnemyBase.h"

class EnemyKnight : public EnemyBase
{
public:
    static EnemyKnight* create();
    virtual bool init() override;
    virtual void update(float dt) override;

    // 覆盖：用于实现盾牌格挡
    virtual void takeDamage(int damage) override;

protected:
    virtual void doAttack() override;

private:
    float _blockChance = 0.75f;   // 75% 格挡概率
};