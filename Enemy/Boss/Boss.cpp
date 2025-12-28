#include "Boss.h"
#include "Player/Maria.h"  // 玩家类头文件

USING_NS_CC;

// 动画资源定义 (如果动画.c3b文件在项目中的路径改变需修改)
static const std::string ANIM_IDLE = "Armature|maw_idle";               // 闲置动画
static const std::string ANIM_WALK = "Armature|maw_walk";               // 行走动画
static const std::string ANIM_RUN = "Armature|maw_run";                 // 奔跑动画
static const std::string ANIM_DEAD = "Armature|maw_dead";               // 死亡动画
static const std::string ANIM_SHOW_MUSLE = "Armature|maw_showMusle";     // 展示肌肉动画
static const std::string ANIM_ROAR = "Armature|maw_roar";               // 咆哮动画
static const std::string ANIM_DODGE = "Armature|maw_dodge_right";        // 闪避动画
static const std::vector<std::string> ATTACK_ANIMS = {                   // 攻击动画列表
    "Armature|maw_punch",
    "Armature|maw_swipe",
    "Armature|maw_jumpAttack_2"
};

/**
 * 创建Boss实例
 * @param modelPath 模型文件路径
 * @return Boss实例指针
 */
Boss* Boss::createBoss(const std::string& modelPath)
{
    Boss* boss = new (std::nothrow) Boss();
    if (boss && boss->init(modelPath))
    {
        boss->autorelease();
        return boss;
    }
    CC_SAFE_DELETE(boss);
    return nullptr;
}

/**
 * 初始化Boss
 * @param modelPath 模型文件路径
 * @return 初始化是否成功
 */
bool Boss::init(const std::string& modelPath)
{
    if (!Sprite3D::initWithFile(modelPath))
        return false;

    _modelPath = modelPath;
    current_blood = max_blood;  // 初始血量设为最大血量
    _state = State::IDLE;       // 初始状态为闲置

    // 初始化动画
    CrossFadeAnim(ANIM_IDLE, true, 0.0f);

    this->scheduleUpdate();     // 开启帧更新
    return true;
}

/**
 * 帧更新函数
 * @param dt 帧间隔时间
 */
void Boss::update(float dt)
{
    if (_state == State::DEAD)
        return;

    attackTimer += dt;  // 更新攻击计时器
    HandleAI(dt);       // 处理AI逻辑
}

/**
 * 处理AI逻辑
 * @param dt 帧间隔时间
 */
void Boss::HandleAI(float dt)
{
    if (!_player)
        return;

    // 死亡、狂暴、闪避状态下不执行AI逻辑
    if (_state == State::DEAD || _state == State::RAGING || _state == State::DODGING)
        return;

    float dist = Distance_BossPlayer();  // 计算与玩家的距离

    // 在攻击范围内
    if (dist <= attack_range)
    {
        // 攻击冷却结束且当前不是攻击状态
        if (attackTimer >= attack_cooldown && _state != State::ATTACK)
        {
            attackTimer = 0.0f;
            // 狂暴状态随机选择攻击方式，否则使用默认攻击
            PerformAttack(is_rage ? cocos2d::random(0, (int)ATTACK_ANIMS.size() - 1) : 0);
        }
        // 非攻击和闲置状态时切换到闲置
        else if (_state != State::ATTACK && _state != State::IDLE)
        {
            _state = State::IDLE;
            CrossFadeAnim(ANIM_IDLE, true);
        }

        // 转向玩家
        Vec3 dir = _player->getPosition3D() - getPosition3D();
        setRotation3D(Vec3(0, CC_RADIANS_TO_DEGREES(atan2(dir.x, dir.z)), 0));
    }
    // 不在攻击范围内且非攻击状态时，向玩家移动
    else if (_state != State::ATTACK)
    {
        MoveToPlayer(dt);
    }
}

/**
 * 向玩家移动
 * @param dt 帧间隔时间
 */
void Boss::MoveToPlayer(float dt)
{
    if (!_player || _state == State::DEAD)
        return;

    Vec3 bossPos = getPosition3D();
    Vec3 playerPos = _player->getPosition3D();

    // 1. 计算方向向量并忽略Y轴（保持水平移动）
    Vec3 dir = playerPos - bossPos;
    dir.y = 0;  // 确保Boss在水平面上移动

    float distance = dir.length();  // 计算距离

    // 2. 接近攻击范围时停止移动
    if (distance <= attack_range * 0.95f)
    {
        if (_state != State::IDLE && _state != State::ATTACK)
        {
            _state = State::IDLE;
            CrossFadeAnim(ANIM_IDLE, true);
        }
        return;
    }

    // 3. 归一化方向向量
    dir.normalize();

    // 4. 执行位置更新（仅X和Z轴）
    float speed = is_rage ? run_speed : walk_speed;  // 狂暴状态下使用奔跑速度
    this->setPosition3D(Vec3(
        bossPos.x + dir.x * speed * dt,
        bossPos.y,  // 保持原Y坐标不变
        bossPos.z + dir.z * speed * dt
    ));

    // 5. 转向移动方向
    float targetAngle = CC_RADIANS_TO_DEGREES(atan2f(dir.x, dir.z));
    setRotation3D(Vec3(0, targetAngle, 0));

    // 6. 更新移动状态和动画
    State moveState = is_rage ? State::RUN : State::WALK;
    if (_state != moveState)
    {
        _state = moveState;
        CrossFadeAnim(is_rage ? ANIM_RUN : ANIM_WALK, true);
    }
}

/**
 * 动画切换（淡入淡出效果）
 * @param animName 动画名称
 * @param loop 是否循环播放
 * @param duration 过渡时间
 */
void Boss::CrossFadeAnim(const std::string& animName, bool loop, float duration)
{
    // 避免重复播放同一动画
    if (_currentAnimName == animName)
        return;

    _currentAnimName = animName;

    // 停止当前动画
    this->stopActionByTag(TAG_ANIM);

    // 创建动画实例
    auto animation = Animation3D::create(_modelPath, animName);
    if (!animation)
        return;

    auto animate = Animate3D::create(animation);
    animate->setTag(TAG_ANIM);

    // 根据是否循环设置播放方式
    if (loop)
    {
        auto repeat = RepeatForever::create(animate);
        repeat->setTag(TAG_ANIM);
        this->runAction(repeat);
    }
    else
    {
        this->runAction(animate);
    }
}

/**
 * 执行攻击动作
 * @param type 攻击类型（对应ATTACK_ANIMS的索引）
 */
void Boss::PerformAttack(int type)
{
    _state = State::ATTACK;
    std::string animName = ATTACK_ANIMS[type];
    CrossFadeAnim(animName, false);  // 播放攻击动画（非循环）

    // 获取动画时长
    auto anim = Animation3D::create(_modelPath, animName);
    float totalTime = anim->getDuration();

    // 攻击判定帧和动作结束回调
    this->runAction(Sequence::create(
        DelayTime::create(totalTime * 0.5f),  // 延迟到动画中途（判定帧）
        CallFunc::create([this]() {
            this->OnAttackFrameReached(is_rage ? 30 : 15);  // 狂暴状态伤害更高
            }),
        DelayTime::create(totalTime * 0.5f),  // 剩余动画时间
        CallFunc::create(CC_CALLBACK_0(Boss::OnActionFinished, this)),  // 动作结束
        nullptr
    ));
}

/**
 * 攻击判定帧处理（造成伤害）
 * @param damage 伤害值
 */
void Boss::OnAttackFrameReached(int damage)
{
    // 在攻击范围内则对玩家造成伤害
    if (_player && Distance_BossPlayer() < attack_range + 30.0f)
    {
        auto m = dynamic_cast<Maria*>(_player);
        if (m)
            m->takeDamage(damage);
    }
}

/**
 * 动作结束处理
 */
void Boss::OnActionFinished()
{
    if (_state == State::DEAD)
        return;

    _state = State::IDLE;  // 回到闲置状态
    _currentAnimName = "";
}

/**
 * 承受伤害处理
 * @param damage 伤害值
 */
void Boss::TakeDamage(int damage)
{
    if (_state == State::DEAD)
        return;

    current_blood -= damage;  // 扣除血量

    // 非狂暴状态且非攻击状态下，有50%概率闪避
    if (!is_rage && _state != State::ATTACK && CCRANDOM_0_1() < 0.5f)
    {
        performDodge();
        return;
    }

    // 受击闪烁效果（红色→白色）
    this->runAction(Sequence::create(
        TintTo::create(0.1f, 255, 100, 100),
        TintTo::create(0.1f, 255, 255, 255),
        nullptr
    ));

    // 血量低于一半时进入狂暴模式
    if (!is_rage && current_blood < max_blood / 2)
    {
        is_rage = true;
        enterRageMode();
    }

    CCLOG("Boss took %d damage, remaining HP: %d", damage, current_blood);

    // 血量为0时死亡
    if (current_blood <= 0)
        Die();
}

/**
 * 死亡处理
 */
void Boss::Die()
{
    _state = State::DEAD;
    this->stopAllActions();  // 停止所有动作
    CrossFadeAnim(ANIM_DEAD, false);  // 播放死亡动画

    // 死亡动画播放后淡出并移除
    this->runAction(Sequence::create(
        DelayTime::create(2.0f),
        FadeOut::create(1.0f),
        RemoveSelf::create(),
        nullptr
    ));
}

/**
 * 计算与玩家的距离
 * @return 距离值
 */
float Boss::Distance_BossPlayer()
{
    return _player ? getPosition3D().distance(_player->getPosition3D()) : 9999.0f;
}

/**
 * 进入狂暴模式
 */
void Boss::enterRageMode()
{
    is_rage = true;
    _state = State::RAGING;
    this->stopAllActions();  // 停止当前所有动作

    // 狂暴模式序列：咆哮动画→等待→恢复战斗状态（攻击冷却缩短）
    auto sequence = Sequence::create(
        CallFunc::create([this]() { CrossFadeAnim(ANIM_ROAR, false); }),
        DelayTime::create(3.0f),  // 咆哮动画持续时间
        CallFunc::create([this]() {
            attack_cooldown *= 0.6f;  // 攻击冷却缩短为60%
            _state = State::IDLE;
            }),
        nullptr
    );
    this->runAction(sequence);
}

/**
 * 执行闪避动作
 */
void Boss::performDodge()
{
    if (_state == State::DODGING || !_player)
        return;

    _state = State::DODGING;
    CrossFadeAnim(ANIM_DODGE, false);  // 播放闪避动画

    // 1. 记录当前Y坐标（保持高度不变）
    Vec3 currentPos = this->getPosition3D();
    float lockY = currentPos.y;

    // 计算远离玩家的方向
    Vec3 dir = currentPos - _player->getPosition3D();
    dir.y = 0;
    dir.normalize();

    // 2. 计算闪避目标位置
    float dodgeDist = 150.0f;
    Vec3 targetPos = Vec3(
        currentPos.x + dir.x * dodgeDist,
        lockY,
        currentPos.z + dir.z * dodgeDist
    );

    // 3. 执行闪避移动
    auto moveAction = MoveTo::create(0.4f, targetPos);

    // 闪避结束后回到闲置状态
    auto callback = CallFunc::create([this]() {
        _state = State::IDLE;
        });

    this->runAction(Sequence::create(moveAction, callback, nullptr));
}

/**
 * 设置目标玩家
 * @param player 玩家节点
 */
void Boss::setTarget(Node* player)
{
    _player = player;
}

/**
 * 判断是否死亡
 * @return 是否死亡
 */
bool Boss::IsDead() const
{
    return _state == State::DEAD;
}

/**
 * 析构函数
 */
Boss::~Boss()
{
}