#include "EnemyGoblin.h"
#include "player/Player.h"

USING_NS_CC;

// ================= 资源定义 =================
static const std::string GOBLIN_MODEL = "model/goblin/goblin.c3b";  // 模型路径
static const std::string ANIM_IDLE = "Armature|goblin_idle";     //  idle动画
static const std::string ANIM_RUN = "Armature|goblin_run";      // 跑步动画
static const std::string ANIM_ATTACK = "Armature|goblin_attack";   // 攻击动画
static const std::string ANIM_HIT = "Armature|goblin_hit";      // 受击动画
static const std::string ANIM_DEAD = "Armature|goblin_dead";     // 死亡动画
static const std::string ANIM_BLOCK = "Armature|goblin_block";    // 格挡动画

// ================= 行为参数 =================
static const float GOBLIN_DETECTION_RANGE = 300.0f;   // 检测范围
static const float GOBLIN_ATTACK_START_DISTANCE = 70.0f; // 攻击启动距离
static const float GOBLIN_REAL_HIT_DISTANCE = 80.0f;   // 实际攻击判定距离
static const float GOBLIN_RETREAT_DISTANCE = 60.0f;   // 后退距离
static const float GOBLIN_RETREAT_TIME = 0.6f;    // 后退时间

// 创建地精实例
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

// 初始化
bool EnemyGoblin::init()
{
    if (!EnemyBase::init())
        return false;

    // 初始化属性
    _maxHp = 60;
    _hp = _maxHp;
    _attack = 12;
    _speed = 70.0f;  // 移动速度较快
    _attackRange = GOBLIN_ATTACK_START_DISTANCE;  // 攻击范围
    _attackCooldown = 4.0f;  // 攻击冷却时间

    // 加载模型
    _model = Sprite3D::create(GOBLIN_MODEL);
    if (_model)
    {
        _model->setScale(0.15f);  // 缩放模型
        this->addChild(_model);

        // 预加载动画（保留原始实现）
        _idleAction = Animate3D::create(Animation3D::create(GOBLIN_MODEL, ANIM_IDLE));
        _idleAction->retain();

        _runAction = Animate3D::create(Animation3D::create(GOBLIN_MODEL, ANIM_RUN));
        _runAction->retain();

        _attackAction = Animate3D::create(Animation3D::create(GOBLIN_MODEL, ANIM_ATTACK));
        _attackAction->retain();

        _hitAction = Animate3D::create(Animation3D::create(GOBLIN_MODEL, ANIM_HIT));
        _hitAction->retain();

        _deadAction = Animate3D::create(Animation3D::create(GOBLIN_MODEL, ANIM_DEAD));
        _deadAction->retain();
    }

    // 初始状态设为Idle
    changeState(EnemyState::IDLE);
    return true;
}

// 帧更新
void EnemyGoblin::update(float dt)
{
    // 攻击/受击状态时不执行移动逻辑
    if (_state == EnemyState::ATTACK || _state == EnemyState::HIT)
        return;

    EnemyBase::update(dt);
    // 死亡状态或无目标时退出
    if (_state == EnemyState::DEAD || !_target)
        return;

    // 计算与目标的距离
    float distance = getPosition3D().distance(_target->getPosition3D());

    // --- 状态逻辑 ---
    // 超出检测范围：回到Idle
    if (distance > GOBLIN_DETECTION_RANGE)
    {
        changeState(EnemyState::IDLE);
        return;
    }

    // 在攻击范围内
    if (distance <= _attackRange)
    {
        // 面向目标
        rotateToTarget();

        // 攻击冷却结束：执行攻击
        if (_attackTimer >= _attackCooldown)
        {
            _attackTimer = 0.0f;
            changeState(EnemyState::ATTACK);

            // 攻击序列：前摇等待 -> 执行攻击 -> 后摇等待 -> 后退
            float hitTiming = 1.5f;   // 前摇时间（攻击判定点）
            float backSwing = 1.0f;   // 后摇时间

            auto attackSequence = Sequence::create(
                DelayTime::create(hitTiming),                  // 前摇等待
                CallFunc::create(CC_CALLBACK_0(EnemyGoblin::doAttack, this)),  // 执行攻击判定
                DelayTime::create(backSwing),                  // 后摇等待
                CallFunc::create(CC_CALLBACK_0(EnemyGoblin::retreatFromTarget, this)),  // 后退
                nullptr
            );
            this->runAction(attackSequence);
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

// 受击处理
void EnemyGoblin::takeDamage(int damage)
{
    // 已死亡则忽略
    if (_state == EnemyState::DEAD)
        return;

    // 1. 扣血（使用父类逻辑或自定义）
    _hp -= damage;
    CCLOG("Goblin took %d damage, remaining HP: %d", damage, _hp);

    // 血量归0：切换死亡状态
    if (_hp <= 0)
    {
        _hp = 0;
        changeState(EnemyState::DEAD);
        return;
    }

    // 2. 打断当前所有动作
    this->stopAllActions();
    if (_model)
        _model->stopAllActions();

    // 3. 切换受击状态（不调用changeState，避免冲突）
    _state = EnemyState::HIT;

    // 4. 播放受击动画
    if (_hitAction && _model)
    {
        _model->runAction(_hitAction);
    }

    // 5. 受击后逻辑：等待0.5s -> 后退
    auto reactionSeq = Sequence::create(
        DelayTime::create(0.5f),  // 受击硬直
        CallFunc::create(CC_CALLBACK_0(EnemyGoblin::retreatFromTarget, this)),
        nullptr
    );
    this->runAction(reactionSeq);
}

// 执行攻击判定
void EnemyGoblin::doAttack()
{
    if (_state == EnemyState::DEAD)
        return;

    // 判定目标是否为玩家
    auto player = dynamic_cast<Player*>(_target);
    if (!player)
        return;

    // 距离在有效攻击范围内：造成伤害
    if (getPosition3D().distance(player->getPosition3D()) <= GOBLIN_REAL_HIT_DISTANCE)
    {
        player->takeDamage(_attack);
    }
}

// 从目标后退
void EnemyGoblin::retreatFromTarget()
{
    // 死亡或无目标：回到Idle
    if (_state == EnemyState::DEAD || !_target)
    {
        changeState(EnemyState::IDLE);
        return;
    }

    // 1. 计算后退方向（与目标相反）
    Vec3 selfPos = getPosition3D();
    Vec3 targetPos = _target->getPosition3D();
    Vec3 runDir = selfPos - targetPos;
    runDir.y = 0;  // 忽略Y轴

    // 避免除0：默认方向
    if (runDir.lengthSquared() < 0.0001f)
        runDir = Vec3(0, 0, 1);
    else
        runDir.normalize();

    // 2. 旋转面向后退方向
    float radians = atan2f(runDir.x, runDir.z);  // 计算弧度
    float degrees = CC_RADIANS_TO_DEGREES(radians);  // 转为角度
    this->setRotation3D(Vec3(0, degrees, 0));  // 应用旋转

    // 3. 播放跑步动画
    if (_model)
    {
        _model->stopAllActions();
        _model->runAction(RepeatForever::create(_runAction));
    }

    // 4. 执行后退移动
    Vec3 retreatVec = runDir * GOBLIN_RETREAT_DISTANCE;  // 后退距离
    auto moveAction = MoveBy::create(GOBLIN_RETREAT_TIME, retreatVec);  // 后退动作

    // 后退结束：回到Idle
    auto finishRetreat = CallFunc::create([this]()
        {
            if (_state != EnemyState::DEAD)
            {
                this->changeState(EnemyState::IDLE);
            }
        });

    this->runAction(Sequence::create(moveAction, finishRetreat, nullptr));
}