#pragma once
#include "cocos2d.h"
#include "EnemyState.h"

class EnemyBase : public cocos2d::Node
{
public:
    EnemyBase();
    virtual ~EnemyBase();

    virtual bool init() override;
    virtual void update(float dt) override;

    // ===== 对外接口 =====
    void setTarget(cocos2d::Node* target);
    virtual void takeDamage(int damage);
    bool isDead() const;

protected:
    // ===== 子类必须实现 =====
    virtual void doAttack() = 0;

    // ===== 行为工具函数 =====
    bool isInAttackRange() const;
    void moveTowardsTarget(float dt);
    void rotateToTarget();
    void changeState(EnemyState state);

protected:
    // ===== 状态 =====
    EnemyState _state = EnemyState::IDLE;

    // ===== 基础属性 =====
    int _maxHp = 0;
    int _hp = 0;
    int _attack = 0;
    float _speed = 0.0f;
    float _attackRange = 0.0f;
    float _attackCooldown = 0.0f;
    float _attackTimer = 0.0f;

    // ===== 目标 =====
    cocos2d::Node* _target = nullptr;

    // ===== 模型 =====
    cocos2d::Sprite3D* _model = nullptr;

    // ===== 动画 =====
    cocos2d::Animate3D* _idleAction = nullptr;
    cocos2d::Animate3D* _runAction = nullptr;
    cocos2d::Animate3D* _attackAction = nullptr;
    cocos2d::Animate3D* _hitAction = nullptr;
    cocos2d::Animate3D* _blockAction = nullptr;
    cocos2d::Animate3D* _deadAction = nullptr;
};