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
private:
    float _detectionRange = 150.0f; // 警戒范围，超过这个距离不理人
};