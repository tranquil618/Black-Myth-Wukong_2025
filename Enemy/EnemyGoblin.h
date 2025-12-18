#pragma once
#include "EnemyBase.h"

class EnemyGoblin : public EnemyBase
{
public:
    static EnemyGoblin* create();
    virtual bool init() override;
    virtual void update(float dt) override;

protected:
    virtual void doAttack() override;
};