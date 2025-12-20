#include "Maria.h"
#include "Enemy/EnemyBase.h"
#include "base/CCDirector.h"
#include "renderer/CCMaterial.h" 
#include "2d/CCActionInterval.h" // 包含 DelayTime
#include "2d/CCActionInstant.h" // 包含 Sequence, CallFunc

// =========================================================================
// 静态常量定义
// =========================================================================
const std::string Maria::ANIM_MODEL_PATH = "Maria.c3b";

const std::string Maria::ANIM_IDLE = "Armature|idle";
const std::string Maria::ANIM_WALK = "Armature|walk";
const std::string Maria::ANIM_RUN = "Armature|run_forward";

const std::string Maria::ANIM_P_ATTACK1 = "Armature|slash_1";
const std::string Maria::ANIM_P_ATTACK2 = "Armature|slash_4";
const std::string Maria::ANIM_P_ATTACK3 = "Armature|slash_2";

const std::string Maria::ANIM_SKILL_START = "Armature|pose";
const std::string Maria::ANIM_GHOST_1 = "Armature|slash_2";
const std::string Maria::ANIM_GHOST_2 = "Armature|slide_attack";
const std::string Maria::ANIM_GHOST_3 = "Armature|right_kick";
const std::string Maria::ANIM_GHOST_4 = "Armature|highSpinAttack";
const std::string Maria::ANIM_GHOST_5 = "Armature|left_kick";

// 其他动作
const std::string Maria::ANIM_JUMP = "Armature|jump"; // 跳跃动画
const std::string Maria::ANIM_START_CROUCH = "Armature|crouch";
const std::string Maria::ANIM_CROUCH_IDLE = "Armature|crouching";
const std::string Maria::ANIM_DE_CROUCH = "Armature|decrouch";

const std::string Maria::ANIM_START_BLOCK = "Armature|block";
const std::string Maria::ANIM_BLOCK_IDLE = "Armature|block_hold";
const std::string Maria::ANIM_DE_BLOCK = "Armature|deblock";

const std::string Maria::ANIM_DODGE_BACK = "Armature|dodge_backword";
const std::string Maria::ANIM_DODGE_FRONT = "Armature|dodge_forward";
const std::string Maria::ANIM_DODGE_LEFT = "Armature|dodge_right";
const std::string Maria::ANIM_DODGE_RIGHT = "Armature|dodge_left";

const std::string Maria::ANIM_HURT = "Armature|impact_small";  
const std::string Maria::ANIM_DEAD = "Armature|dead"; 
// =========================================================================
// 初始化和构造相关函数
// =========================================================================

Maria* Maria::create(const std::string& modelPath)
{
    Maria* maria = new (std::nothrow) Maria();
    if (maria && maria->init(modelPath))
    {
        maria->autorelease();
        return maria;
    }
    CC_SAFE_DELETE(maria);
    return nullptr;
}

bool Maria::init(const std::string& modelPath)
{
    if (!Sprite3D::init()) {
        return false;
    }

    if (!Sprite3D::initWithFile(modelPath)) {
        CCLOGERROR("Failed to load 3D model: %s", modelPath.c_str());
        return false;
    }
    _moveBasePos = getPosition3D();
    setState(MariaState::IDLE);
    this->scheduleUpdate();

    return true;
}

// 播放动画 (使用 Animate3D 实现)
void Maria::playAnimation(const std::string& animName, bool loop)
{
    this->stopAllActions();

    auto anim = Animation3D::create(Maria::ANIM_MODEL_PATH, animName);
    if (!anim) {
        CCLOGERROR("Animation not found: %s", animName.c_str());
        setState(MariaState::IDLE);
        return;
    }

    auto animate = Animate3D::create(anim); // 正确关联 Animation3D*

    if (loop) {
        this->runAction(RepeatForever::create(animate));
    }
    else {
        this->runAction(animate);
    }
}

// 获取角色前向向量（考虑Y轴旋转）
Vec3 Maria::getForwardVector() const
{
    float yaw = CC_DEGREES_TO_RADIANS(getRotation3D().y);
    return Vec3(sinf(yaw), 0, cosf(yaw)).getNormalized(); // 前向向量(XZ平面)
}

// 动画播放完成的回调 (处理状态切换逻辑)
void Maria::onAnimationFinished(const std::string& animName)
{
    // 动画结束时，清理连招窗口动作
    if (_comboWindowAction) {
        this->stopAction(_comboWindowAction);
        _comboWindowAction = nullptr;
    }

    // 注意：攻击连招的状态切换逻辑实现在 runAttackCombo() 的 Sequence::create 中
    // 因此，这里不需要处理 ATTACKING 状态，只需要处理 IDLE 相关的默认逻辑

    // 排除特殊状态，其他状态默认回到IDLE
    if (_currentState != MariaState::ATTACKING &&
        _currentState != MariaState::CROUCH_IDLE &&
        _currentState != MariaState::BLOCK_IDLE)
    {
        setState(MariaState::IDLE);
    }
}


// =========================================================================
// 状态和行为切换函数 (setState)
// =========================================================================

void Maria::setState(MariaState newState)
{
    if (_currentState == newState)
        return;

    // --- 强制清理连招相关动作 ---
    if (_comboWindowAction) {
        this->stopAction(_comboWindowAction);
        _comboWindowAction = nullptr;

        // 从攻击状态切换时，更新基准位置
        if (_currentState == MariaState::ATTACKING) {
            _moveBasePos = getPosition3D();
        }
    }
    // ----------------------------------------------------

    MariaState oldState = _currentState; // 记录旧状态
    _currentState = newState;

    // 停止循环动作 (Tag 100 用于蹲伏/格挡/跳跃的结束动画)
    this->stopActionByTag(100);

    switch (newState)
    {
        case MariaState::IDLE:
            playAnimation(ANIM_IDLE, true);
            _moveSpeed = 0.0f;

            // 从攻击或跳跃状态切换到空闲时更新基准位置
            if (oldState == MariaState::ATTACKING || oldState == MariaState::JUMPING) {
                _moveBasePos = getPosition3D();
            }
            break;

        case MariaState::WALK:
            playAnimation(ANIM_WALK, true);
            _moveSpeed = _walkSpeed;
            break;

        case MariaState::RUN:
            playAnimation(ANIM_RUN, true);
            _moveSpeed = _runSpeed;
            break;

        case MariaState::BLOCK_IDLE:
            playAnimation(ANIM_BLOCK_IDLE, true);
            _moveSpeed = 0.0f;
            break;

        case MariaState::CROUCH_IDLE:
            playAnimation(ANIM_CROUCH_IDLE, true);
            _moveSpeed = 0.0f;
            break;

        case MariaState::JUMPING:
            // JUMPING状态在runJump()中处理动画逻辑
            break;

        case MariaState::DODGING:
            _moveSpeed = 0.0f; // 速度在update的attack逻辑中处理
            break;

        default:
            break;
    }
}


// =========================================================================
// 移动和跳跃
// =========================================================================
void Maria::runMove(const Vec3& direction, bool isRunning)
{
    // 攻击状态处理
    if (_isAttacking) {
        // 攻击中只允许旋转，不处理移动
        if (_isRotationLocked) {
            // 锁定状态下，面向锁定方向
            float targetAngle = CC_RADIANS_TO_DEGREES(atan2f(_lockedDirection.x, _lockedDirection.z));
            setRotation3D(Vec3(0, targetAngle, 0));
        }
        else {
            // 非锁定状态下，面向移动方向
            Vec3 dir(direction.x, 0, direction.z);
            if (dir.lengthSquared() > 0.01f) {
                dir.normalize();
                float angle = CC_RADIANS_TO_DEGREES(atan2f(dir.x, dir.z));
                setRotation3D(getRotation3D().lerp(Vec3(0, angle, 0), 0.2f));
            }
        }
        return;
    }

    // 闪避或攻击状态下不能移动
    if (_currentState == MariaState::DODGING || _isAttacking) {
        return;
    }

    // 1. 处理输入方向 (direction 为 相对于相机 的方向: X/Z)
    Vec3 moveInput(direction.x, 0, direction.z);
    if (moveInput.lengthSquared() < 0.0001f) {
        // 无输入时停止移动
        if (_currentState == MariaState::WALK || _currentState == MariaState::RUN) {
            setState(MariaState::IDLE);
        }
        return;
    }
    moveInput.normalize();

    // 2. 获取相机Y轴旋转角度 (转换为弧度)
    float cameraYaw = CC_DEGREES_TO_RADIANS(this->_cameraYawAngle);

    // 3. 计算相机坐标系的前向和右向向量 (XZ平面)
    Vec3 camForward = Vec3(sinf(cameraYaw), 0.0f, cosf(cameraYaw));
    Vec3 camRight = Vec3(camForward.z, 0.0f, -camForward.x);

    // 4. 将输入方向转换为世界坐标系方向
    Vec3 moveDir = camForward * moveInput.z - camRight * moveInput.x;
    moveDir.normalize();


    // 角色旋转处理
    if (!_isRotationLocked) {
        // 非锁定状态下，面向移动方向
        float targetAngle = CC_RADIANS_TO_DEGREES(atan2f(moveDir.x, moveDir.z));
        float currentAngle = getRotation3D().y;
        float newAngle = currentAngle + (targetAngle - currentAngle) * 0.2f;
        setRotation3D(Vec3(0, newAngle, 0));
    }
    else {
        // 锁定状态下，面向锁定方向
        float targetAngle = CC_RADIANS_TO_DEGREES(atan2f(_lockedDirection.x, _lockedDirection.z));
        setRotation3D(Vec3(0, targetAngle, 0));
    }
    // -----------------------------------------------------------------

    // 设置移动状态
    setState(isRunning ? MariaState::RUN : MariaState::WALK);
    _moveDirection = moveDir; // 保存方向供update()使用
}


void Maria::stopMove()
{
    // 只有在行走或奔跑状态时才停止
    if (_currentState == MariaState::WALK || _currentState == MariaState::RUN) {
        setState(MariaState::IDLE);
    }
}

void Maria::runDodge(const Vec3& direction)
{
    // 只有在 idle/walk/run 状态才能闪避
    if (_currentState != MariaState::IDLE && _currentState != MariaState::WALK && _currentState != MariaState::RUN) {
        return;
    }

    this->stopAllActions();

    // 1. 确定闪避动画和方向
    std::string dodgeAnim = ANIM_DODGE_BACK; // 默认后闪避
    Vec3 worldDodgeDir;

    // 计算相机坐标系
    float cameraYaw = CC_DEGREES_TO_RADIANS(this->_cameraYawAngle);
    Vec3 camForward = Vec3(sinf(cameraYaw), 0.0f, cosf(cameraYaw));
    Vec3 camRight = Vec3(camForward.z, 0.0f, -camForward.x);

    // 转换输入方向到世界坐标系
    Vec3 inputDir = camForward * direction.z - camRight * direction.x;

    if (direction.lengthSquared() < 0.01f) {
        // 无输入时默认后闪避
        dodgeAnim = ANIM_DODGE_BACK;
        worldDodgeDir = -getForwardVector();
    }
    else {
        worldDodgeDir = inputDir;
        worldDodgeDir.normalize();

        // 根据输入方向选择动画
        if (std::abs(direction.z) >= std::abs(direction.x)) {
            dodgeAnim = (direction.z > 0) ? ANIM_DODGE_FRONT : ANIM_DODGE_BACK;
        }
        else {
            dodgeAnim = (direction.x > 0) ? ANIM_DODGE_RIGHT : ANIM_DODGE_LEFT;
        }
    }

    // 2. 设置状态
    setState(MariaState::DODGING);
    _isAttacking = true;
    _attackStartPos = getPosition3D();
    _attackDirection = worldDodgeDir;
    _attackDistance = 15.0f;
    _attackDuration = 0.45f;
    _attackElapsed = 0.0f;

    // 3. 播放动画并设置回调
    auto anim3d = Animation3D::create(Maria::ANIM_MODEL_PATH, dodgeAnim);
    if (anim3d) {
        auto animate = Animate3D::create(anim3d);
        auto seq = Sequence::create(
            animate,
            CallFunc::create([=]() {
                _moveBasePos = getPosition3D();
                _isAttacking = false;
                setState(MariaState::IDLE);
                }),
            nullptr
        );
        this->stopAllActions();
        this->runAction(seq);
    }
}

// =========================================================================
// 其他功能
// =========================================================================
void Maria::toggleLock(bool isLocked, const cocos2d::Vec3& targetDir)
{
    _isRotationLocked = isLocked;

    if (isLocked) {
        // 锁定目标方向（XZ平面）
        _lockedDirection = Vec3(targetDir.x, 0, targetDir.z).getNormalized();

        // 面向目标方向
        float targetAngle = CC_RADIANS_TO_DEGREES(atan2f(_lockedDirection.x, _lockedDirection.z));
        setRotation3D(Vec3(0, targetAngle, 0));
    }
}

void Maria::runJump()
{
    if (_currentState == MariaState::IDLE || _currentState == MariaState::WALK || _currentState == MariaState::RUN) {
        setState(MariaState::JUMPING);
        playAnimation(ANIM_JUMP, false);

        // 跳跃动画完成后回到idle
        this->runAction(Sequence::create(
            DelayTime::create(0.8f),
            CallFunc::create([=]() { setState(MariaState::IDLE); }),
            nullptr
        ));
    }
}

// =========================================================================
// 格挡和蹲伏 (Q键切换，RMB格挡)
// =========================================================================
void Maria::toggleCrouch()
{
    if (_currentState == MariaState::IDLE || _currentState == MariaState::WALK) {
        // 从站立/行走到蹲伏
        playAnimation(ANIM_START_CROUCH, false);

        auto action = Sequence::create(
            DelayTime::create(0.5f),
            CallFunc::create([=]() { setState(MariaState::CROUCH_IDLE); }),
            nullptr
        );
        action->setTag(100);
        this->runAction(action);

    }
    else if (_currentState == MariaState::CROUCH_IDLE) {
        // 从蹲伏到站起
        playAnimation(ANIM_DE_CROUCH, false);

        auto action = Sequence::create(
            DelayTime::create(0.5f),
            CallFunc::create([=]() { setState(MariaState::IDLE); }),
            nullptr
        );
        action->setTag(100);
        this->runAction(action);
    }
}

void Maria::startBlock()
{
    if (_currentState == MariaState::IDLE || _currentState == MariaState::WALK) {
        playAnimation(ANIM_START_BLOCK, false);

        auto action = Sequence::create(
            DelayTime::create(0.3f),
            CallFunc::create([=]() { setState(MariaState::BLOCK_IDLE); }),
            nullptr
        );
        action->setTag(100);
        this->runAction(action);
    }
}

void Maria::stopBlock()
{
    if (_currentState == MariaState::BLOCK_IDLE) {
        playAnimation(ANIM_DE_BLOCK, false);

        auto action = Sequence::create(
            DelayTime::create(0.3f),
            CallFunc::create([=]() { setState(MariaState::IDLE); }),
            nullptr
        );
        action->setTag(100);
        this->runAction(action);
    }
}

// =========================================================================
// 帧更新 (处理位置更新)
// =========================================================================
void Maria::update(float dt)
{
    // 处理攻击位移
    if (_isAttacking)
    {
        _attackElapsed += dt;

        float t = _attackElapsed / _attackDuration;
        if (t > 1.0f) t = 1.0f;

        // 缓动效果
        float ease = 1.0f - (1.0f - t) * (1.0f - t);
        float currentDist = _attackDistance * ease;

        Vec3 targetPos = _attackStartPos + _attackDirection * currentDist;
        setPosition3D(targetPos);
    }

    // 处理行走/奔跑移动
    else if (_currentState == MariaState::WALK || _currentState == MariaState::RUN)
    {
        // 计算每帧位移
        Vec3 moveDelta = _moveDirection * _moveSpeed * dt;

        // 更新基准位置
        _moveBasePos += moveDelta;

        // 更新角色位置
        setPosition3D(_moveBasePos);
    }
}


// =========================================================================
// 连招攻击系统 (Combo Attack)
// =========================================================================
void Maria::runAttackCombo()
{
    // 1. 状态检查：特殊状态下不能攻击
    if (_currentState == MariaState::SKILLING ||
        _currentState == MariaState::HURT ||
        _currentState == MariaState::BLOCK_IDLE ||
        _currentState == MariaState::CROUCH_IDLE)
    {
        return;
    }

    // 2. 初始化攻击状态
    _attackStartPos = _moveBasePos;
    _currentState = MariaState::ATTACKING;

    // 停止之前的连招窗口动作
    if (_comboWindowAction) {
        this->stopAction(_comboWindowAction);
        _comboWindowAction = nullptr;
    }

    // 3. 更新连招计数（1-3循环）并选择动画
    _comboCount = (_comboCount % 3) + 1;

    std::string nextAnim;
    float currentMoveDist = 0.0f;
    float currentDuration = 0.0f;

    if (_comboCount == 1) {
        nextAnim = ANIM_P_ATTACK1;
        currentMoveDist = 1.8f;
        currentDuration = 0.35f;
    }
    else if (_comboCount == 2) {
        nextAnim = ANIM_P_ATTACK2;
        currentMoveDist = 3.0f;
        currentDuration = 0.4f;
    }
    else { // 第三段大招
        nextAnim = ANIM_P_ATTACK3;
        currentMoveDist = 10.0f;
        currentDuration = 0.5f;
    }

    // 设置位移参数供 update() 使用
    _isAttacking = true;
    _attackElapsed = 0.0f;
    _attackDirection = getForwardVector();
    _attackDistance = currentMoveDist;
    _attackDuration = currentDuration;

    // 4. 播放动画
    this->playAnimation(nextAnim);

    auto anim3d = Animation3D::create(Maria::ANIM_MODEL_PATH, nextAnim);
    if (!anim3d) return;

    auto animateAction = Animate3D::create(anim3d);

    // 5. 创建攻击序列动作，包含伤害判定
    auto attackSequence = Sequence::create(
        animateAction,
        CallFunc::create([=]() {
            // --- 新增：伤害判定逻辑 ---
            // 这里的判定逻辑：寻找场景中所有的敌人并检查距离
            // 假设你的敌人存放在父节点的 children 中，或者你有一个 EnemyManager
            auto scene = this->getParent();
            // 计算攻击判定中心（自己位置向前偏移一段距离）
            Vec3 attackCenter = this->getPosition3D() + getForwardVector() * 15.0f;

            for (auto node : scene->getChildren()) {
                auto enemy = dynamic_cast<EnemyBase*>(node);
                if (enemy && !enemy->isDead()) {
                    // 使用攻击中心点与怪物的距离判定
                    float dist = attackCenter.distance(enemy->getPosition3D());
                    if (dist < 25.0f) { // 适当调大判定范围
                        enemy->takeDamage(_attackPower);
                    }
                }
            }

            // --- 原有：位置和状态重置逻辑 ---
            Vec3 attackDisplacement = getForwardVector() * _attackDistance;
            Vec3 finalPos = _attackStartPos + attackDisplacement;

            this->setPosition3D(finalPos);
            this->_moveBasePos = finalPos;

            this->_comboCount = 0; // 连招结束重置
            this->_isAttacking = false;
            this->setState(MariaState::IDLE);
            }),
        nullptr
    );

    this->runAction(attackSequence);
}


// =========================================================================
// 影子技能系统 (Ghost Shadow Skill)
// =========================================================================
void Maria::runSkillShadow()
{
    // 只有在特定状态下才能释放技能
    if (_currentState != MariaState::IDLE && _currentState != MariaState::WALK && _currentState != MariaState::RUN) {
        return;
    }

    _currentState = MariaState::SKILLING;
    this->stopAllActions();

    // 1. 播放技能起始动画
    this->playAnimation(ANIM_SKILL_START, false);

    // 2. 创建技能序列（5个阶段）
    auto skillSequence = Sequence::create(
        // 等待起始动作
        DelayTime::create(0.4f),

        // --- 阶段I: 第一段影子 ---
        CallFunc::create([=]() {
            auto ghost1 = Maria::create(Maria::ANIM_MODEL_PATH);
            ghost1->setPosition3D(this->getPosition3D() + Vec3(5, 0, 10));
            ghost1->setRotation3D(this->getRotation3D());
            ghost1->setOpacity(180);
            ghost1->setScale(0.1f);
            this->getParent()->addChild(ghost1);

            ghost1->runAction(Sequence::create(
                Animate3D::create(Animation3D::create(Maria::ANIM_MODEL_PATH, ANIM_GHOST_1)),
                CallFunc::create([ghost1]() { ghost1->removeFromParent(); }),
                nullptr
            ));
            }),
        DelayTime::create(0.6f),

        // --- 阶段II: 第二段影子 ---
        CallFunc::create([=]() {
            auto ghost2 = Maria::create(Maria::ANIM_MODEL_PATH);
            ghost2->setPosition3D(this->getPosition3D() + Vec3(0, 0, 0));
            ghost2->setRotation3D(this->getRotation3D());
            ghost2->setOpacity(180);
            ghost2->setScale(0.1f);
            this->getParent()->addChild(ghost2);

            ghost2->runAction(Sequence::create(
                Animate3D::create(Animation3D::create(Maria::ANIM_MODEL_PATH, ANIM_GHOST_2)),
                CallFunc::create([ghost2]() { ghost2->removeFromParent(); }),
                nullptr
            ));
            }),
        DelayTime::create(0.5f),

        // --- 阶段III: 第三段影子 ---
        CallFunc::create([=]() {
            auto ghost3 = Maria::create(Maria::ANIM_MODEL_PATH);
            ghost3->setPosition3D(this->getPosition3D() + Vec3(0, 0, 8));
            ghost3->setRotation3D(this->getRotation3D());
            ghost3->setOpacity(180);
            ghost3->setScale(0.1f);
            this->getParent()->addChild(ghost3);

            ghost3->runAction(Sequence::create(
                Animate3D::create(Animation3D::create(Maria::ANIM_MODEL_PATH, ANIM_GHOST_3)),
                CallFunc::create([ghost3]() { ghost3->removeFromParent(); }),
                nullptr
            ));
            }),
        DelayTime::create(0.4f),

        // --- 阶段IV: 第四段影子 ---
        CallFunc::create([=]() {
            auto ghost4 = Maria::create(Maria::ANIM_MODEL_PATH);
            ghost4->setPosition3D(this->getPosition3D() + Vec3(0, 0, 15));
            ghost4->setRotation3D(this->getRotation3D());
            ghost4->setOpacity(180);
            ghost4->setScale(0.1f);
            this->getParent()->addChild(ghost4);

            ghost4->runAction(Sequence::create(
                Animate3D::create(Animation3D::create(Maria::ANIM_MODEL_PATH, ANIM_GHOST_4)),
                CallFunc::create([ghost4]() { ghost4->removeFromParent(); }),
                nullptr
            ));
            }),
        DelayTime::create(0.5f),

        // --- 阶段V: 第五段影子 ---
        CallFunc::create([=]() {
            auto ghost5 = Maria::create(Maria::ANIM_MODEL_PATH);
            ghost5->setPosition3D(this->getPosition3D() + Vec3(-10, 0, 5));
            ghost5->setRotation3D(this->getRotation3D());
            ghost5->setOpacity(180);
            ghost5->setScale(0.1f);
            this->getParent()->addChild(ghost5);

            ghost5->runAction(Sequence::create(
                Animate3D::create(Animation3D::create(Maria::ANIM_MODEL_PATH, ANIM_GHOST_5)),
                CallFunc::create([ghost5]() { ghost5->removeFromParent(); }),
                nullptr
            ));
            }),
        DelayTime::create(0.6f),

        // 技能结束，回到空闲状态
        CallFunc::create([=]() {
            this->onAnimationFinished(ANIM_SKILL_START);
            }),
        nullptr
    );

    this->runAction(skillSequence);
}


// =========================================================================
// 对player接口的函数实现
// =========================================================================
void Maria::takeDamage(int damage) {
    // 1. 安全检查：死亡或闪避中无敌
    if (_currentState == MariaState::DEAD || _currentState == MariaState::DODGING) {
        return;
    }

    // 2. 计算实际伤害
    int finalDamage = damage;
    if (_currentState == MariaState::BLOCK_IDLE) {
        finalDamage = std::max(1, (int)(damage * 0.2f)); // 确保至少受到1点伤害
    }

    _hp -= finalDamage;
    CCLOG("Maria took %d damage, remaining HP: %d", finalDamage, _hp);

    // 3. 停止当前所有动作，确保受击/死亡动画立即执行
    this->stopAllActions();

    if (_hp <= 0) {
        _hp = 0;
        setState(MariaState::DEAD);
        // 播放死亡动画，通常不循环
        playAnimation(ANIM_DEAD, false);
    }
    else {
        setState(MariaState::HURT);

        // 播放受击动画
        // 建议在 playAnimation 内部处理：动画播放完后通过回调自动回到 IDLE 状态
        playAnimation(ANIM_HURT, false);

        // 如果你的框架没有自动回到IDLE的逻辑，可以加一个延时回调（假设受击动画时长0.5秒）
        this->runAction(Sequence::create(
            DelayTime::create(0.5f),
            CallFunc::create([this]() {
                if (_currentState != MariaState::DEAD) {
                    this->setState(MariaState::IDLE);
                    this->playAnimation(ANIM_IDLE, true);
                }
                }),
            nullptr
        ));
    }
}

void Maria::attackEnemy(EnemyBase* enemy) {
    if (enemy && !enemy->isDead()) {
        enemy->takeDamage(_attackPower);
    }
}