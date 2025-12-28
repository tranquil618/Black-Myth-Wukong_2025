#pragma once
#include "EnemyBase.h"

// 牛头人敌人类
class EnemyMinotaur : public EnemyBase
{
public:
    // 创建牛头人实例
    static EnemyMinotaur* create();
    // 初始化
    virtual bool init() override;
    // 帧更新
    virtual void update(float dt) override;

protected:
    // 执行攻击
    virtual void doAttack() override;
};