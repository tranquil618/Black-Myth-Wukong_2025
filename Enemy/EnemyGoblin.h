#pragma once
#include "EnemyBase.h"

class EnemyGoblin : public EnemyBase
{
public:
    static EnemyGoblin* create();
    virtual bool init() override;
    virtual void update(float dt) override;

    // 被主角攻击时调用
    virtual void takeDamage(int damage) override;

protected:
    virtual void doAttack() override;

private:
    bool _isBlocking = false;
};