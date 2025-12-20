#pragma once
#include "EnemyBase.h"

class EnemyMinotaur : public EnemyBase
{
public:
    static EnemyMinotaur* create();
    virtual bool init() override;
    virtual void update(float dt) override;

protected:
    virtual void doAttack() override;
};