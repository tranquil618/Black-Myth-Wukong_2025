#include "EnemyKnight.h"
#include "Player/Player.h"
#include <cstdlib>

USING_NS_CC;

// ===== 模型与动画名 =====
static const std::string KNIGHT_MODEL = "model/knight/knight.c3b";

static const std::string ANIM_IDLE = "Armature|knight_idle";
static const std::string ANIM_RUN = "Armature|knight_run";
static const std::string ANIM_ATTACK = "Armature|knight_attack";
static const std::string ANIM_HIT = "Armature|knight_hit";
static const std::string ANIM_BLOCK = "Armature|knight_block";
static const std::string ANIM_DEAD = "Armature|knight_dead";

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

    // ===== 属性：中等型防御敌人 =====
    _maxHp = 120;
    _hp = _maxHp;
    _attack = 18;
    _speed = 45.0f;
    _attackRange = 35.0f;
    _attackCooldown = 1.8f;

    // ===== 模型 =====
    _model = Sprite3D::create(KNIGHT_MODEL);
    _model->setScale(1.0f);
    addChild(_model);

    // ===== 动画加载（单 c3b + 动画名）=====
    _idleAction = Animate3D::create(
        Animation3D::create(KNIGHT_MODEL, ANIM_IDLE));
    _idleAction->retain();

    _runAction = Animate3D::create(
        Animation3D::create(KNIGHT_MODEL, ANIM_RUN));
    _runAction->retain();

    _attackAction = Animate3D::create(
        Animation3D::create(KNIGHT_MODEL, ANIM_ATTACK));
    _attackAction->retain();

    _hitAction = Animate3D::create(
        Animation3D::create(KNIGHT_MODEL, ANIM_HIT));
    _hitAction->retain();

    _blockAction = Animate3D::create(
        Animation3D::create(KNIGHT_MODEL, ANIM_BLOCK));
    _blockAction->retain();

    _deadAction = Animate3D::create(
        Animation3D::create(KNIGHT_MODEL, ANIM_DEAD));
    _deadAction->retain();

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
            _attackTimer = 0.0f;
            changeState(EnemyState::ATTACK);

            runAction(Sequence::create(
                DelayTime::create(0.45f),
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

    // ===== 75% 概率格挡 =====
    float r = static_cast<float>(rand()) / RAND_MAX;
    if (r < _blockChance)
    {
        changeState(EnemyState::BLOCK);
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