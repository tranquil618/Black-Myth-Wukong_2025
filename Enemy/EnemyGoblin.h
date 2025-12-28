#pragma once
#include "EnemyBase.h" 

// 地精敌人类
class EnemyGoblin : public EnemyBase
{
public:
    // 创建地精实例
    static EnemyGoblin* create();
    // 初始化
    virtual bool init() override;
    // 帧更新
    virtual void update(float dt) override;
    // 受击处理
    virtual virtual void takeDamage(int damage) override;

protected:
    // 执行攻击
    void doAttack();
    // 从目标后退
    void retreatFromTarget();
};