#include "Boss.h"

USING_NS_CC;

static const std::string ANIM_FILE = "maw.c3b";
static const std::string ANIM_IDLE = "Armature|maw_idle";//待机
static const std::string ANIM_WALK = "Armature|maw_walk";//走
static const std::string ANIM_RUN = "Armature|maw_run";//跑
static const std::string ANIM_ATTACK_0 = "Armature|maw_jumpAttack_2"; //一阶段
static const std::string ANIM_ATTACK_SWIPE = "Armature|maw_swipe"; //二阶段
static const std::string ANIM_ATTACK_JUMP = "Armature|maw_jumpAttack";//二阶段
static const std::string ANIM_ATTACK_PUNCH = "Armature|maw_punch";//二阶段
static const std::string ANIM_ROAR = "Armature|maw_roar"; // 怒气爆发
static const std::string ANIM_DEAD = "Armature|mae_dead";//死亡
static const std::string ANIM_DODGE_RIGHT = "Armature|dodge_right";//右闪避

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

void Boss::SetTarget(cocos2d::Node* player)
{
    _player = player;
}

bool Boss::IsDead() const
{
    return _state == State::DEAD;
}

bool Boss::init(const std::string& modelPath)
{
    //初始化3D模型
    if (!Sprite3D::initWithFile(modelPath)) 
    {
        return false;
    }
    _modelPath = modelPath; // 保存路径

    //基础属性初始化
    //血量
    max_blood = 1000;
    current_blood = max_blood;
    //怒气状态
    is_rage = false;
    _state = State::IDLE;
    //攻击
    attackTimer = 0.0f;
    attack_cooldown = 2.0f; // 初始攻击间隔
    _player = nullptr;

    //启动自带的待机动画
    EnterIdle();

    scheduleUpdate();
    return true;
}

//检查 Boss 是否处于可行动状态。如果是，则驱动handleAI进行距离判定
//只有Idle状态才会攻击,Hit/Attack时不会打断
void Boss::update(float dt)
{
    if (_state == State::DEAD|| _state == State::ATTACK|| _state == State::HIT)
        return;
    attackTimer += dt;
    HandleAI(dt);
}

//根据与悟空的距离和阶段触发行动
void Boss::HandleAI(float dt)
{
    if (!_player)
        return;
    //距离足够近
    if (Distance_BossPlayer() <= attack_range )
    {
        if (attackTimer >= attack_cooldown)//冷却结束则攻击
        {
            attackTimer = 0.0f;
            if (!is_rage)// 第一阶段：固定执行一种攻击动作
                PerformAttack(0);
            else// 第二阶段：随机执行三种攻击动作
            {                
                int randomSkill = cocos2d::random(1, 3);
                PerformAttack(randomSkill);
            }
        }        
    }
    else if (Distance_BossPlayer() <= track_range)//在追踪范围内，如果进入怒气阶段就跑，否则走
        MoveToPlayer(dt, is_rage);
    else//距离远就停下待机
    {
        if (_state != State::IDLE)
        {
            _state = State::IDLE;
            EnterIdle();
        }
    }
}

//计算boss与悟空距离
float Boss::Distance_BossPlayer()
{
    if (!_player) // 如果悟空指针为空，判定失败
        return 9999999; 
    return getPosition3D().distance(_player->getPosition3D());
}

//被攻击到
void Boss::TakeDamage(int damage)
{
    // 如果已经在死亡/闪避状态，直接返回
    if (_state == State::DEAD|| _state == State::DODGE)
        return;

    current_blood -= damage;
    if (current_blood <= 0)
    {
        Die();
        return;
    }
    //判断是否进入怒气状态（血量小于一半时）
    if (!is_rage && current_blood <= max_blood / 2)
    {
        EnterRage();
        return;
    }
       
    //停止当前动作
    stopActionByTag(TAG_HIT);
    _state = State::HIT;
    stopActionByTag(TAG_ANIM);

    // --- 计算后退方向 ---
    Vec3 backVec;
    if (_player) 
    {
        // 计算单位向量：(Boss位置 - 悟空位置)
        Vec3 playerPos = _player->getPosition3D();
        Vec3 bossPos = this->getPosition3D();

        // 方向向量 = 终点 - 起点
        Vec3 direction = bossPos - playerPos;
        direction.y = 0; // 锁定高度，防止 Boss 被打飞到天上
        direction.normalize(); // 归一化，只取方向

        backVec = direction * 20.0f; //后退距离
    }
    else// 万一没获取到悟空，默认后退
        backVec = Vec3(0, 0, 20);
    
    // 变红
    auto tintRed = cocos2d::TintTo::create(0.1f, 255, 100, 100);
    auto tintNormal = cocos2d::TintTo::create(0.1f, 255, 255, 255);
    // 后退一点
    auto moveBack = cocos2d::MoveBy::create(0.1f, backVec);

    // 受击动作序列
    auto hitseq = cocos2d::Sequence::create(
        cocos2d::Spawn::create(tintRed, moveBack, nullptr), //一边退一边红
        tintNormal,
        cocos2d::CallFunc::create([this]() {
            // 受击结束
            this->OnAttackComplete();
            }),
        nullptr
    );
    hitseq->setTag(TAG_HIT);
    runAction(hitseq);
}

//将状态设为 ATTACK 防止被 AI 重复触发 。在一阶段固定出招，二阶段通过 random 实现随机出招。
void Boss::PerformAttack(int id) //id对应Mixamo的不同动作片段
{
    _state = State::ATTACK;
    this->stopActionByTag(TAG_ANIM);
    
    std::string anim;
    switch (id)
    {
        case 0:
            anim = ANIM_ATTACK_0;
            break;
        case 1:
            anim = ANIM_ATTACK_PUNCH;
            break;
        case 2:
            anim = ANIM_ATTACK_JUMP;
            break;
        case 3:
            anim = ANIM_ATTACK_SWIPE;
            break;
    }
    PlayAnim(anim, false);

    float duration = 1.0f; // 默认保底时间
    auto animData = cocos2d::Animation3D::create(_modelPath, anim);
    if (animData)
        duration = animData->getDuration();

    runAction(Sequence::create(
        DelayTime::create(duration),//z动态等待动画播完
        CallFunc::create(CC_CALLBACK_0(Boss::OnAttackComplete, this)),
        nullptr
    ));
}

//动画
void Boss::PlayAnim(const std::string& animName, bool loop)
{
    stopActionByTag(TAG_ANIM);

    auto anim = Animation3D::create(_modelPath, animName);
    if (!anim)
        return;

    auto animate = Animate3D::create(anim);
    animate->setTag(TAG_ANIM);

    if (loop)
        runAction(RepeatForever::create(animate));
    else
        runAction(animate);
}

//攻击动画结束后的回调。清除攻击标记，将状态切回IDLE，并恢复受怒气影响的呼吸效果。
void Boss::OnAttackComplete()
{
    _state = State::IDLE;
    attackTimer = 0.0f;
    if (is_rage)
    {
        if (persistentRed && !this->getActionByTag(TAG_RAGE_COLOR))
        {
            persistentRed->setTag(TAG_RAGE_COLOR);
            this->runAction(persistentRed);
        }
        if (persistentBreath && !this->getActionByTag(TAG_RAGE_BREATH))
        {
            persistentBreath->setTag(TAG_RAGE_BREATH);
            this->runAction(persistentBreath);
        }
    }
    EnterIdle();
}

void Boss::EnterIdle()
{
    _state = State::IDLE;
    PlayAnim(ANIM_IDLE, true);
}

//死亡消失
void Boss::Die()
{
    if (_state == State::DEAD)
        return;
    _state = State::DEAD;
    stopAllActions();
    unscheduleUpdate(); 
    PlayAnim(ANIM_DEAD, false);
    auto anim3D = Animation3D::create(_modelPath, ANIM_DEAD);
    /*if (!anim3D)
    {
        // 如果没找到死亡动画，直接走原本的消失逻辑
        this->runAction(Sequence::create(
            Spawn::create(FadeOut::create(1.0f), ScaleTo::create(1.0f, 0.0f), nullptr),
            RemoveSelf::create(),
            nullptr));
        return;
    }*/
    auto animate = Animate3D::create(anim3D);
    auto animDuration = animate->getDuration(); // 获取动画实际长度（秒）
    auto waitAnimFinish = DelayTime::create(animDuration); // 等待倒地动作播完
    //组合死亡序列
    //播动画 -> 停顿一下 -> 淡出缩小 -> 彻底从场景移除
    auto deathSeq = Sequence::create(
        waitAnimFinish,                       // 首先播放 3D 倒地动作
        DelayTime::create(0.5f),       // 倒地后在地上躺 0.5 秒，增加打击感
        Spawn::create(                 // 同时执行淡出和缩小
            FadeOut::create(1.0f),
            ScaleTo::create(1.0f, 0.0f),
            nullptr
        ),
        RemoveSelf::create(),          // 彻底删除对象
        nullptr
    );
    this->runAction(deathSeq);
}

//怒气状态
void Boss::EnterRage()
{
    if (is_rage) 
        return; 
    is_rage = true;
    _state = State::RAGE;
    this->stopActionByTag(TAG_ANIM);
    PlayAnim(ANIM_ROAR, false);
    // 获取咆哮动画的时长
    float roarDuration = 1.0f; // 默认值
    auto animData = Animation3D::create(_modelPath, ANIM_ROAR);
    if (animData)
        roarDuration = animData->getDuration();

    //攻击频率提升
    attack_cooldown *= 0.5f;   //攻击速度翻倍
    stopActionByTag(TAG_ANIM);
    //怒气瞬间：红色闪烁+震动
    auto tint_red = TintTo::create(0.1f, 255, 50, 50);
    auto tint_white = TintTo::create(0.1f, 255, 255, 255);
    auto shake_l = MoveBy::create(0.05f, Vec2(-15, 0));
    auto shake_r = MoveBy::create(0.05f, Vec2(15, 0));
    auto burst = cocos2d::Sequence::create(tint_red,shake_l,shake_r,shake_l,shake_r,tint_white,nullptr);

    //怒气状态持续
    //淡红色
    auto red = TintTo::create(1.0f, 255, 120, 120);
    auto normal = TintTo::create(1.0f, 255, 255, 255);
    if (persistentRed)  // 清理旧的（如果有）
        persistentRed->release();
    persistentRed = RepeatForever::create(Sequence::create(red, normal, nullptr));
    persistentRed->retain(); //手动增加引用计数
    //呼吸加重
    auto scaleUp = ScaleTo::create(1.0f, 1.05f);
    auto scaleDown = ScaleTo::create(1.0f, 1.0f);
    if (persistentBreath)
        persistentBreath->release();
    persistentBreath = RepeatForever::create(Sequence::create(scaleUp, scaleDown, nullptr));
    persistentBreath->retain();
    //怒气状态持续执行动作序列
    auto startPersistentEffects = CallFunc::create([this]() {
        // 恢复到 IDLE 状态并开启持续动画
        this->OnAttackComplete();
        });

    //先瞬间爆发，然后开启持续效果
    this->runAction(Sequence::create(
        burst,
        DelayTime::create(roarDuration > 0.5f ? roarDuration - 0.5f : 0.1f),
        startPersistentEffects,
        nullptr
    ));    
}

//向悟空方向移动
void Boss::MoveToPlayer(float dt, bool is_run) 
{
    if (!_player || _state == State::ATTACK || _state == State::HIT || _state == State::DEAD)
        return;

    //获取位置并计算方向
    Vec3 bossPos = this->getPosition3D();
    Vec3 playerPos = _player->getPosition3D();
    Vec3 direction = playerPos - bossPos;
    direction.y = 0; // 锁定高度，防止Boss往天上飞
    direction.normalize(); // 归一化，只取方向

    float distance = Distance_BossPlayer();
    // 如果已经在攻击距离内，就不移动了
    if (distance < attack_range) 
        return;
    //计算位移
    float speed = is_run ? run_speed : walk_speed; 
    this->setPosition3D(bossPos + direction * speed * dt);

    //自动转向，让Boss面对悟空
    float angle = CC_RADIANS_TO_DEGREES(atan2(direction.x, direction.z));
    this->setRotation3D(Vec3(0, angle, 0));

    //状态管理与动画切换
    State targetState = is_run ? State::RUN : State::WALK;
    if (_state != targetState)
    {
        _state = targetState;
        PlayAnim(is_run ? ANIM_RUN : ANIM_WALK, true);
    }
}
/*
void Boss::EnterWalkAnimation(bool is_run) 
{
    // 停止之前的待机或行走动作
    this->stopActionByTag(ANIM_TAG_IDLE);

    float start = is_run ? 10.0f : 8.0f;    // 假设 Mixamo 中 8s 是走，10s 是跑
    float duration = is_run ? 1.0f : 1.5f; // 动作长度

    auto animate = Animate3D::create(_bossAnimation, start, duration);
    auto repeat = RepeatForever::create(animate);
    repeat->setTag(ANIM_TAG_IDLE); // 复用IDLE的Tag方便清理
    this->runAction(repeat);
}*/

//闪避
void Boss::Dodge() 
{
    if (_state == State::ATTACK || _state == State::DODGE || _state == State::DEAD) 
        return;
    _state = State::DODGE;
    stopActionByTag(TAG_ANIM);

    //计算闪避方向（取boss的右侧向量，实现侧闪）
    Vec3 bossPos = this->getPosition3D();
    Vec3 playerPos = _player->getPosition3D();
    Vec3 toPlayer = playerPos - bossPos;
    toPlayer.y = 0;
    toPlayer.normalize();

    // 侧方向量：将指向玩家的向量旋转90度
    Vec3 sideDir = Vec3(-toPlayer.z, 0, toPlayer.x);
    float dodge_distance = 200.0f; //闪避距离
    Vec3 targetPos = bossPos + sideDir * dodge_distance;

    PlayAnim(ANIM_DODGE_RIGHT, false);

    auto move = EaseSineOut::create(MoveTo::create(0.5f, targetPos));
    auto done = CallFunc::create([this]() {
        _state = State::IDLE;
        this->EnterIdle();
        });
    // 执行
    this->runAction(Sequence::create(move, done, nullptr));
}

Boss::~Boss() 
{
    CC_SAFE_RELEASE(persistentRed);
    CC_SAFE_RELEASE(persistentBreath);
}
