#include "EnemyMinotaur.h"
#include "Player/Player.h"

USING_NS_CC;

// ================= 模型与动画路径 =================
static const std::string MINOTAUR_MODEL = "model/minotaur/minotaur.c3b";  // 模型路径
static const std::string ANIM_IDLE = "Armature|minotaur_idle";       // idle动画
static const std::string ANIM_WALK = "Armature|minotaur_walk";       // 行走动画
static const std::string ANIM_ATTACK = "Armature|minotaur_attack";     // 攻击动画
static const std::string ANIM_HIT = "Armature|minotaur_hit";        // 受击动画
static const std::string ANIM_DEAD = "Armature|minotaur_dead";       // 死亡动画

// ================= 行为参数 =================
static const float MINOTAUR_DETECTION_RANGE = 350.0f;  // 检测范围（较广）

// 创建牛头人实例
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

// 初始化
bool EnemyMinotaur::init()
{
    if (!EnemyBase::init())
        return false;

    // ===== 初始化属性 =====
    _maxHp = 200;    // 高血量
    _hp = _maxHp;
    _attack = 28;     // 高攻击力
    _speed = 35.0f;  // 移动较慢
    _attackRange = 40.0f;  // 攻击范围较大
    _attackCooldown = 3.0f; // 攻击冷却较短

    // 加载模型
    _model = Sprite3D::create(MINOTAUR_MODEL);
    if (_model)
    {
        // _model->setScale(1.2f);  // 如需放大模型可启用
        this->addChild(_model);

        // 预加载动画
        _idleAction = Animate3D::create(Animation3D::create(MINOTAUR_MODEL, ANIM_IDLE));
        _idleAction->retain();

        _runAction = Animate3D::create(Animation3D::create(MINOTAUR_MODEL, ANIM_WALK));
        _runAction->retain();

        _attackAction = Animate3D::create(Animation3D::create(MINOTAUR_MODEL, ANIM_ATTACK));
        _attackAction->retain();

        _hitAction = Animate3D::create(Animation3D::create(MINOTAUR_MODEL, ANIM_HIT));
        _hitAction->retain();

        _deadAction = Animate3D::create(Animation3D::create(MINOTAUR_MODEL, ANIM_DEAD));
        _deadAction->retain();
    }

    // 初始状态设为Idle（无格挡动作）
    _blockAction = nullptr;
    changeState(EnemyState::IDLE);
    return true;
}

// 帧更新
void EnemyMinotaur::update(float dt)
{
    // 1. 死亡状态或无目标时退出
    if (_state == EnemyState::DEAD || !_target)
        return;

    // 2. 调用父类更新（如攻击计时器）
    EnemyBase::update(dt);
    // 受击状态时不执行移动逻辑
    if (_state == EnemyState::HIT)
        return;

    // 3. 计算与目标的距离
    float distance = getPosition3D().distance(_target->getPosition3D());

    // 超出检测范围：回到Idle
    if (distance > MINOTAUR_DETECTION_RANGE)
    {
        changeState(EnemyState::IDLE);
        return;
    }

    // 4. 战斗逻辑
    if (isInAttackRange())  // 在攻击范围内
    {
        // 面向目标
        rotateToTarget();

        // 攻击冷却结束：执行攻击
        if (_attackTimer >= _attackCooldown)
        {
            _attackTimer = 0.0f;
            changeState(EnemyState::ATTACK);

            // 攻击序列：前摇等待 -> 执行攻击
            runAction(Sequence::create(
                DelayTime::create(0.6f),  // 前摇时间（牛头人动作较慢）
                CallFunc::create(CC_CALLBACK_0(EnemyMinotaur::doAttack, this)),  // 执行攻击判定
                nullptr
            ));
        }
        else
        {
            // 冷却中：保持Idle
            changeState(EnemyState::IDLE);
        }
    }
    else
    {
        // 不在攻击范围：移动到目标
        changeState(EnemyState::RUN);
        moveTowardsTarget(dt);
    }
}

// 执行攻击判定
void EnemyMinotaur::doAttack()
{
    auto player = dynamic_cast<Player*>(_target);
    if (player)
    {
        // 扩展攻击范围容错
        if (getPosition3D().distance(player->getPosition3D()) < _attackRange + 25.0f)
        {
            player->takeDamage(_attack);
        }
    }
}