#include "EnemyGoblin.h"
#include "Player/Player.h"

USING_NS_CC;

// ===== 动画名称常量 =====
static const std::string GOBLIN_MODEL = "model/goblin/goblin.c3b";

static const std::string ANIM_IDLE = "Armature|goblin_idle";
static const std::string ANIM_RUN = "Armature|goblin_run";
static const std::string ANIM_ATTACK = "Armature|goblin_attack";
static const std::string ANIM_HIT = "Armature|goblin_hit";
static const std::string ANIM_DEAD = "Armature|goblin_dead";
static const std::string ANIM_BLOCK = "Armature|goblin_block";

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

    // ===== 基础属性 =====
    _maxHp = 60;
    _hp = _maxHp;
    _attack = 10;
    _speed = 80.0f;
    _attackRange = 30.0f;
    _attackCooldown = 1.2f;

    // ===== 创建模型 =====
    _model = Sprite3D::create(GOBLIN_MODEL);
    _model->setScale(0.8f);
    addChild(_model);

    // ===== 加载动画（关键修改点）=====
    _idleAction = Animate3D::create(
        Animation3D::create(GOBLIN_MODEL, ANIM_IDLE));
    _idleAction->retain();

    _runAction = Animate3D::create(
        Animation3D::create(GOBLIN_MODEL, ANIM_RUN));
    _runAction->retain();

    _attackAction = Animate3D::create(
        Animation3D::create(GOBLIN_MODEL, ANIM_ATTACK));
    _attackAction->retain();

    _hitAction = Animate3D::create(
        Animation3D::create(GOBLIN_MODEL, ANIM_HIT));
    _hitAction->retain();

    _blockAction = Animate3D::create(
        Animation3D::create(GOBLIN_MODEL, ANIM_BLOCK));
    _blockAction->retain();

    _deadAction = Animate3D::create(
        Animation3D::create(GOBLIN_MODEL, ANIM_DEAD));
    _deadAction->retain();

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
            _attackTimer = 0.0f;
            changeState(EnemyState::ATTACK);

            runAction(Sequence::create(
                DelayTime::create(0.35f),
                CallFunc::create(
                    CC_CALLBACK_0(EnemyGoblin::doAttack, this)),
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