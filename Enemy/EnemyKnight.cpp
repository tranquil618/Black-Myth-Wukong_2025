#include "EnemyKnight.h"
#include "Player/Player.h"
#include <cstdlib>  // 用于随机数

USING_NS_CC;

// ================= 模型与动画路径 =================
static const std::string KNIGHT_MODEL = "model/knight/knight.c3b";  // 模型路径
static const std::string ANIM_IDLE = "Armature|knight_idle";     // idle动画
static const std::string ANIM_RUN = "Armature|knight_walk";     // 行走动画
static const std::string ANIM_ATTACK = "Armature|knight_attack";   // 攻击动画
static const std::string ANIM_HIT = "Armature|knight_hit";      // 受击动画
static const std::string ANIM_BLOCK = "Armature|knight_block";    // 格挡动画
static const std::string ANIM_DEAD = "Armature|knight_death";    // 死亡动画

// ================= 行为参数 =================
static const float KNIGHT_DETECTION_RANGE = 250.0f;  // 检测范围（比地精小）

// 创建骑士实例
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

// 初始化
bool EnemyKnight::init()
{
    if (!EnemyBase::init())
        return false;

    // 初始化属性
    _maxHp = 150;
    _hp = _maxHp;
    _attack = 20;
    _speed = 45.0f;
    _attackRange = 35.0f;
    _attackCooldown = 5.0f;

    // 加载模型
    _model = Sprite3D::create(KNIGHT_MODEL);
    if (_model)
    {
        _model->setScale(0.15f);

        // ================= 加载纹理 & 设置Shader =================
        auto texDiffuse = Director::getInstance()->getTextureCache()->addImage("model/knight/knight_diffuse.png");  // 漫反射纹理
        auto texNormal = Director::getInstance()->getTextureCache()->addImage("model/knight/knight_normal.png");   // 法线纹理

        if (texDiffuse && texNormal)
        {
            // 获取法线贴图Shader
            auto shader = GLProgramCache::getInstance()->getGLProgram("Shader3DPositionNormalTexBumped");

            // 应用Shader
            if (shader)
            {
                auto glState = GLProgramState::create(shader);
                glState->setUniformTexture("u_normalMap", texNormal);  // 设置法线纹理
                _model->setGLProgramState(glState);
            }

            // 设置漫反射纹理
            _model->setTexture(texDiffuse);
        }
        else
        {
            // 加载失败时仅设置漫反射纹理
            _model->setTexture("model/knight/knight_diffuse.png");
        }
        // ==========================================================

        this->addChild(_model);

        // 预加载动画
        _idleAction = Animate3D::create(Animation3D::create(KNIGHT_MODEL, ANIM_IDLE));
        _idleAction->retain();

        _runAction = Animate3D::create(Animation3D::create(KNIGHT_MODEL, ANIM_RUN));
        _runAction->retain();

        _attackAction = Animate3D::create(Animation3D::create(KNIGHT_MODEL, ANIM_ATTACK));
        _attackAction->retain();

        _hitAction = Animate3D::create(Animation3D::create(KNIGHT_MODEL, ANIM_HIT));
        _hitAction->retain();

        _blockAction = Animate3D::create(Animation3D::create(KNIGHT_MODEL, ANIM_BLOCK));
        _blockAction->retain();

        _deadAction = Animate3D::create(Animation3D::create(KNIGHT_MODEL, ANIM_DEAD));
        _deadAction->retain();
    }

    // 初始状态设为Idle
    changeState(EnemyState::IDLE);
    return true;
}

// 帧更新
void EnemyKnight::update(float dt)
{
    // 死亡状态或无目标时退出
    if (_state == EnemyState::DEAD || !_target)
        return;

    // 调用父类更新（如攻击计时器）
    EnemyBase::update(dt);
    // 受击/格挡状态时不执行移动逻辑
    if (_state == EnemyState::HIT || _state == EnemyState::BLOCK)
        return;

    // 计算与目标的距离
    float distance = getPosition3D().distance(_target->getPosition3D());

    // 超出检测范围：回到Idle
    if (distance > KNIGHT_DETECTION_RANGE)
    {
        changeState(EnemyState::IDLE);
        return;
    }

    // ===== 战斗逻辑 =====
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
                DelayTime::create(0.45f),  // 前摇时间（根据动画调整）
                CallFunc::create(CC_CALLBACK_0(EnemyKnight::doAttack, this)),  // 执行攻击判定
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
void EnemyKnight::doAttack()
{
    auto player = dynamic_cast<Player*>(_target);
    if (player)
    {
        // 扩展攻击范围容错：避免模型偏移导致判定失效
        if (getPosition3D().distance(player->getPosition3D()) < _attackRange + 20.0f)
        {
            player->takeDamage(_attack);
        }
    }
}

// 受击处理（含格挡逻辑）
void EnemyKnight::takeDamage(int damage)
{
    if (_state == EnemyState::DEAD)
        return;

    // ===== 格挡判定 =====
    float r = static_cast<float>(rand()) / RAND_MAX;  // 生成0-1随机数
    if (r < _blockChance)  // 触发格挡
    {
        // 仅在非格挡状态时切换状态
        if (_state != EnemyState::BLOCK)
        {
            changeState(EnemyState::BLOCK);
            // 播放格挡动画
            if (_blockAction && _model)
            {
                _model->stopAllActions();
                _model->runAction(_blockAction);
            }

            // 格挡结束后回到Idle
            runAction(Sequence::create(
                DelayTime::create(0.5f),  // 格挡持续时间
                CallFunc::create([this]()
                    {
                        if (_state != EnemyState::DEAD)
                            changeState(EnemyState::IDLE);
                    }),
                nullptr
            ));
        }
        // 格挡成功：不受伤害（可改为减伤，如_hp -= damage * 0.1f）
        return;
    }

    // ===== 格挡失败：扣血 =====
    _hp -= damage;

    // 血量归0：切换死亡状态
    if (_hp <= 0)
    {
        _hp = 0;
        changeState(EnemyState::DEAD);
    }
    else  // 仍存活：受击硬直
    {
        changeState(EnemyState::HIT);
        // 受击后回到Idle
        runAction(Sequence::create(
            DelayTime::create(0.4f),  // 受击硬直时间
            CallFunc::create([this]()
                {
                    if (_state != EnemyState::DEAD)
                        changeState(EnemyState::IDLE);
                }),
            nullptr
        ));
    }
}