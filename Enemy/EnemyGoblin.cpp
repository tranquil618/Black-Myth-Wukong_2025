#include "EnemyGoblin.h"
#include "Player.h"

USING_NS_CC;

EnemyGoblin* EnemyGoblin::create()
{
    EnemyGoblin* ret = new (std::nothrow) EnemyGoblin();
    if (ret && ret->init())
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool EnemyGoblin::init()
{
    if (!EnemyBase::init())
        return false;

    // ===== 基础属性（简单版）=====
    _maxHp = 40;
    _hp = _maxHp;
    _attack = 8;
    _speed = 70.0f;
    _attackRange = 25.0f;
    _attackCooldown = 1.5f;

    // ===== 模型 =====
    _model = Sprite3D::create("model/goblin.c3b");
    _model->setScale(1.0f);
    addChild(_model);

    // ===== 动画 =====
    _idleAction = Animate3D::create(
        Animation3D::create("model/goblin_idle.c3b"));
    _idleAction->retain();

    _runAction = Animate3D::create(
        Animation3D::create("model/goblin_run.c3b"));
    _runAction->retain();

    _attackAction = Animate3D::create(
        Animation3D::create("model/goblin_attack.c3b"));
    _attackAction->retain();

    _hitAction = Animate3D::create(
        Animation3D::create("model/goblin_hit.c3b"));
    _hitAction->retain();

    _deadAction = Animate3D::create(
        Animation3D::create("model/goblin_dead.c3b"));
    _deadAction->retain();

    // 格挡动画（即使只是表现，也单独留）
    _blockAction = Animate3D::create(
        Animation3D::create("model/goblin_block.c3b"));
    _blockAction->retain();

    changeState(EnemyState::IDLE);
    return true;
}

void EnemyGoblin::update(float dt)
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

            // 攻击动画中段命中
            runAction(Sequence::create(
                DelayTime::create(0.35f),
                CallFunc::create(CC_CALLBACK_0(EnemyGoblin::doAttack, this)),
                nullptr));
        }
    }
    else
    {
        changeState(EnemyState::RUN);
        moveTowardsTarget(dt);
    }
}

void EnemyGoblin::doAttack()
{
    auto player = dynamic_cast<Player*>(_target);
    if (player)
    {
        player->takeDamage(_attack);
    }
}

void EnemyGoblin::takeDamage(int damage)
{
    if (_state == EnemyState::DEAD)
        return;

    // 简单版：每次受击都先播放格挡动画
    _isBlocking = true;
    _model->stopAllActions();

    if (_blockAction)
    {
        _model->runAction(Sequence::create(
            _blockAction,
            CallFunc::create([this]() {
                _isBlocking = false;
                }),
            nullptr));
    }

    // 扣血（格挡只是表现，不免伤）
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