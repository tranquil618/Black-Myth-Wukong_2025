#include "EnemyKnight.h"
#include "Player.h"
#include <cstdlib>

USING_NS_CC;

EnemyKnight* EnemyKnight::create()
{
    EnemyKnight* ret = new (std::nothrow) EnemyKnight();
    if (ret && ret->init())
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool EnemyKnight::init()
{
    if (!EnemyBase::init())
        return false;

    // ===== 基础属性（中等型防御敌人）=====
    _maxHp = 120;
    _hp = _maxHp;
    _attack = 18;
    _speed = 45.0f;
    _attackRange = 35.0f;
    _attackCooldown = 1.8f;

    // ===== 模型 =====
    _model = Sprite3D::create("model/knight.c3b");
    _model->setScale(1.0f);
    addChild(_model);

    // ===== 动画 =====
    _idleAction = Animate3D::create(
        Animation3D::create("model/knight_idle.c3b"));
    _idleAction->retain();

    _runAction = Animate3D::create(
        Animation3D::create("model/knight_walk.c3b"));
    _runAction->retain();

    _attackAction = Animate3D::create(
        Animation3D::create("model/knight_attack.c3b"));
    _attackAction->retain();

    _hitAction = Animate3D::create(
        Animation3D::create("model/knight_hit.c3b"));
    _hitAction->retain();

    _deadAction = Animate3D::create(
        Animation3D::create("model/knight_dead.c3b"));
    _deadAction->retain();

    _blockAction = Animate3D::create(
        Animation3D::create("model/knight_block.c3b"));
    _blockAction->retain();

    changeState(EnemyState::IDLE);
    return true;
}

void EnemyKnight::update(float dt)
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

            runAction(Sequence::create(
                DelayTime::create(0.4f),
                CallFunc::create(
                    CC_CALLBACK_0(EnemyKnight::doAttack, this)),
                nullptr));
        }
    }
    else
    {
        changeState(EnemyState::RUN);
        moveTowardsTarget(dt);
    }
}

void EnemyKnight::doAttack()
{
    auto player = dynamic_cast<Player*>(_target);
    if (player)
    {
        player->takeDamage(_attack);
    }
}

void EnemyKnight::takeDamage(int damage)
{
    if (_state == EnemyState::DEAD)
        return;

    // ===== 75% 概率盾牌格挡 =====
    float r = static_cast<float>(rand()) / RAND_MAX;
    if (r < _blockChance)
    {
        // 格挡成功：不扣血
        _model->stopAllActions();
        if (_blockAction)
            _model->runAction(_blockAction);
        return;
    }

    // ===== 未格挡，正常受击 =====
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