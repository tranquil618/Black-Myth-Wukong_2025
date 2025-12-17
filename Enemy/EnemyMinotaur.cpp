#include "EnemyMinotaur.h"
#include "Player.h"

USING_NS_CC;

EnemyMinotaur* EnemyMinotaur::create()
{
    EnemyMinotaur* ret = new (std::nothrow) EnemyMinotaur();
    if (ret && ret->init())
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool EnemyMinotaur::init()
{
    if (!EnemyBase::init())
        return false;

    // ===== 基础属性（重型敌人）=====
    _maxHp = 200;
    _hp = _maxHp;
    _attack = 30;
    _speed = 30.0f;
    _attackRange = 40.0f;
    _attackCooldown = 2.5f;

    // ===== 模型 =====
    _model = Sprite3D::create("model/minotaur.c3b");
    _model->setScale(1.2f);
    addChild(_model);

    // ===== 动画 =====
    _idleAction = Animate3D::create(
        Animation3D::create("model/minotaur_idle.c3b"));
    _idleAction->retain();

    _runAction = Animate3D::create(
        Animation3D::create("model/minotaur_walk.c3b"));
    _runAction->retain();

    _attackAction = Animate3D::create(
        Animation3D::create("model/minotaur_attack.c3b"));
    _attackAction->retain();

    _hitAction = Animate3D::create(
        Animation3D::create("model/minotaur_hit.c3b"));
    _hitAction->retain();

    _deadAction = Animate3D::create(
        Animation3D::create("model/minotaur_dead.c3b"));
    _deadAction->retain();

    changeState(EnemyState::IDLE);
    return true;
}

void EnemyMinotaur::update(float dt)
{
    EnemyBase::update(dt);

    if (_state == EnemyState::DEAD || !_target)
        return;

    if (isInAttackRange())
    {
        rotateToTarget();

        if (_attackTimer >= _attackCooldown)
        {
            changeState(EnemyState::ATTACK);
            _attackTimer = 0.0f;

            // 重击：命中稍晚，显得“沉”
            runAction(Sequence::create(
                DelayTime::create(0.6f),
                CallFunc::create(
                    CC_CALLBACK_0(EnemyMinotaur::doAttack, this)),
                nullptr));
        }
    }
    else
    {
        changeState(EnemyState::RUN);
        moveTowardsTarget(dt);
    }
}

void EnemyMinotaur::doAttack()
{
    auto player = dynamic_cast<Player*>(_target);
    if (player)
    {
        player->takeDamage(_attack);
    }
}