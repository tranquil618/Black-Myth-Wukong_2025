#include "EnemyBase.h"

USING_NS_CC;

EnemyBase::EnemyBase() {}

EnemyBase::~EnemyBase()
{
    CC_SAFE_RELEASE(_idleAction);
    CC_SAFE_RELEASE(_runAction);
    CC_SAFE_RELEASE(_attackAction);
    CC_SAFE_RELEASE(_hitAction);
    CC_SAFE_RELEASE(_blockAction);
    CC_SAFE_RELEASE(_deadAction);
}

bool EnemyBase::init()
{
    if (!Node::init())
        return false;

    _attackRange = 50.0f;  // Ä¬ÈÏ¹¥»÷¾àÀë
    _attackCooldown = 2.0f;   // Ä¬ÈÏ¹¥»÷ÀäÈ´
    _detectionRange = 250.0f;

    _attackTimer = 0.0f;
    _state = EnemyState::IDLE;

    scheduleUpdate();
    return true;
}

void EnemyBase::update(float dt)
{
    if (_state == EnemyState::DEAD)
        return;

    _attackTimer += dt;
}

void EnemyBase::setTarget(Node* target)
{
    _target = target;
}

bool EnemyBase::isDead() const
{
    return _state == EnemyState::DEAD;
}

bool EnemyBase::isInAttackRange() const
{
    if (!_target || !_target->getParent())
        return false;

    float dist = this->getPosition3D().distance(_target->getPosition3D());
    return dist <= _attackRange;
}

void EnemyBase::moveTowardsTarget(float dt)
{
    if (!_target || _state == EnemyState::ATTACK || _state == EnemyState::BLOCK)
        return;

    Vec3 dir = _target->getPosition3D() - this->getPosition3D();
    dir.normalize();

    this->setPosition3D(
        this->getPosition3D() + dir * _speed * dt);
}

void EnemyBase::rotateToTarget()
{
    if (!_target)
        return;

    Vec3 dir = _target->getPosition3D() - this->getPosition3D();
    float angle = CC_RADIANS_TO_DEGREES(atan2(dir.x, dir.z));
    this->setRotation3D(Vec3(0, angle, 0));
}

void EnemyBase::takeDamage(int damage)
{
    if (_state == EnemyState::DEAD)
        return;

    _hp -= damage;

    if (_hp <= 0)
    {
        _hp = 0;
        changeState(EnemyState::DEAD);
    }
    else
    {
        changeState(EnemyState::HIT);
    }
}

void EnemyBase::changeState(EnemyState state)
{
    // ËÀÍöºó²»ÔÊÐíÇÐ»»
    if (_state == EnemyState::DEAD)
        return;

    // ¸ñµ²ÖÐ£¬²»±»ÆäËû×´Ì¬´ò¶Ï
    if (_state == EnemyState::BLOCK && state != EnemyState::DEAD)
        return;

    if (_state == state)
        return;

    _state = state;

    if (!_model)
        return;

    _model->stopAllActions();

    switch (_state)
    {
    case EnemyState::IDLE:
        if (_idleAction)
            _model->runAction(
                RepeatForever::create(_idleAction));
        break;

    case EnemyState::RUN:
        if (_runAction)
            _model->runAction(
                RepeatForever::create(_runAction));
        break;

    case EnemyState::ATTACK:
        if (_attackAction)
            _model->runAction(_attackAction);
        break;

    case EnemyState::HIT:
        if (_hitAction)
            _model->runAction(_hitAction);
        break;

    case EnemyState::BLOCK:
        if (_blockAction)
            _model->runAction(_blockAction);
        break;

    case EnemyState::DEAD:
        if (_deadAction)
            _model->runAction(_deadAction);
        break;
    }
}