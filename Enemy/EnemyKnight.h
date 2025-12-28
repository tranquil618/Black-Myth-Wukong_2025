#pragma once
#include "EnemyBase.h"

// 骑士敌人类
class EnemyKnight : public EnemyBase
{
public:
    // 创建骑士实例
    static EnemyKnight* create();
    // 初始化
    virtual bool init() override;
    // 帧更新
    virtual void update(float dt) override;
    // 受击处理（包含格挡逻辑）
    virtual void takeDamage(int damage) override;

protected:
    // 执行攻击
    virtual void doAttack() override;

private:
    float _blockChance = 0.75f;  // 格挡概率（75%）
};