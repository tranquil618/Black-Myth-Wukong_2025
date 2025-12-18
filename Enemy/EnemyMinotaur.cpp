#include "EnemyMinotaur.h"
#include "Player/Player.h"

USING_NS_CC;

// ===== 模型与动画名 =====
static const std::string MINOTAUR_MODEL = "model/minotaur/minotaur.c3b";

static const std::string ANIM_IDLE = "Armature|minotaur_idle";
static const std::string ANIM_WALK = "Armature|minotaur_walk";
static const std::string ANIM_ATTACK = "Armature|minotaur_attack";
static const std::string ANIM_HIT = "Armature|minotaur_hit";
static const std::string ANIM_DEAD = "Armature|minotaur_dead";

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

    // ===== 属性：高血高攻慢速 =====
    _maxHp = 200;
    _hp = _maxHp;
    _attack = 28;
    _speed = 35.0f;
    _attackRange = 40.0f;
    _attackCooldown = 2.2f;

    // ===== 模型 =====
    _model = Sprite3D::create(MINOTAUR_MODEL);
    _model->setScale(1.2f);
    addChild(_model);

    // ===== 动画加载 =====
    _idleAction = Animate3D::create(
        Animation3D::create(MINOTAUR_MODEL, ANIM_IDLE));
    _idleAction->retain();

    _runAction = Animate3D::create(
        Animation3D::create(MINOTAUR_MODEL, ANIM_WALK));
    _runAction->retain();

    _attackAction = Animate3D::create(
        Animation3D::create(MINOTAUR_MODEL, ANIM_ATTACK));
    _attackAction->retain();

    _hitAction = Animate3D::create(
        Animation3D::create(MINOTAUR_MODEL, ANIM_HIT));
    _hitAction->retain();

    _deadAction = Animate3D::create(
        Animation3D::create(MINOTAUR_MODEL, ANIM_DEAD));
    _deadAction->retain();

    _blockAction = nullptr;

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
            _attackTimer = 0.0f;
            changeState(EnemyState::ATTACK);

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