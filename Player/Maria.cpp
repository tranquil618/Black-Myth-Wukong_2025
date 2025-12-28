#include "Maria.h"
#include "Enemy/EnemyBase.h"
#include "Enemy/Boss/Boss.h"
#include "base/CCDirector.h"
#include "renderer/CCMaterial.h" 
#include "2d/CCActionInterval.h" // 包含DelayTime
#include "2d/CCActionInstant.h"  // 包含Sequence, CallFunc

// =========================================================================
// 静态常量定义
// =========================================================================
const std::string Maria::ANIM_MODEL_PATH = "Maria.c3b";

// 基础动画
const std::string Maria::ANIM_IDLE = "Armature|idle";
const std::string Maria::ANIM_WALK = "Armature|walk";
const std::string Maria::ANIM_RUN = "Armature|run_forward";

// 攻击动画
const std::string Maria::ANIM_P_ATTACK1 = "Armature|slash_1";
const std::string Maria::ANIM_P_ATTACK2 = "Armature|slash_4";
const std::string Maria::ANIM_P_ATTACK3 = "Armature|slash_2";

// 技能动画
const std::string Maria::ANIM_SKILL_START = "Armature|pose";
const std::string Maria::ANIM_GHOST_1 = "Armature|slash_2";
const std::string Maria::ANIM_GHOST_2 = "Armature|slide_attack";
const std::string Maria::ANIM_GHOST_3 = "Armature|right_kick";
const std::string Maria::ANIM_GHOST_4 = "Armature|highSpinAttack";
const std::string Maria::ANIM_GHOST_5 = "Armature|left_kick";

// 特殊状态动画
const std::string Maria::ANIM_JUMP = "Armature|jump";            // 跳跃动画
const std::string Maria::ANIM_START_CROUCH = "Armature|crouch";  // 开始下蹲
const std::string Maria::ANIM_CROUCH_IDLE = "Armature|crouching";// 下蹲待机
const std::string Maria::ANIM_DE_CROUCH = "Armature|decrouch";   // 结束下蹲

const std::string Maria::ANIM_START_BLOCK = "Armature|block";    // 开始格挡
const std::string Maria::ANIM_BLOCK_IDLE = "Armature|block_hold";// 格挡待机
const std::string Maria::ANIM_DE_BLOCK = "Armature|deblock";     // 结束格挡

// 闪避动画
const std::string Maria::ANIM_DODGE_BACK = "Armature|dodge_backword";
const std::string Maria::ANIM_DODGE_FRONT = "Armature|dodge_forward";
const std::string Maria::ANIM_DODGE_LEFT = "Armature|dodge_right";
const std::string Maria::ANIM_DODGE_RIGHT = "Armature|dodge_left";

// 受击与死亡动画
const std::string Maria::ANIM_HURT = "Armature|impact_small";    // 受击动画
const std::string Maria::ANIM_DEAD = "Armature|dead";            // 死亡动画
const std::string Maria::ANIM_RECOVER = "Armature|casting";      // 回血动画

// =========================================================================
// 初始化与销毁相关方法
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

// =========================================================================
// 动画播放相关方法
// =========================================================================

/**
 * 播放指定动画
 * @param animName 动画名称
 * @param loop 是否循环播放
 */
void Maria::playAnimation(const std::string& animName, bool loop)
{
    this->stopAllActions();

    auto anim = Animation3D::create(Maria::ANIM_MODEL_PATH, animName);
    if (!anim) {
        CCLOGERROR("Animation not found: %s", animName.c_str());
        setState(MariaState::IDLE);
        return;
    }

    auto animate = Animate3D::create(anim); // 正确获取Animation3D*

    if (loop) {
        this->runAction(RepeatForever::create(animate));
    }
    else {
        this->runAction(animate);
    }
}

/**
 * 动画播放完成回调处理
 * @param animName 完成的动画名称
 */
void Maria::onAnimationFinished(const std::string& animName)
{
    // 清除连招窗口期
    if (_comboWindowAction) {
        this->stopAction(_comboWindowAction);
        _comboWindowAction = nullptr;
    }

    // 非攻击/下蹲/格挡状态自动返回idle
    if (_currentState != MariaState::ATTACKING &&
        _currentState != MariaState::CROUCH_IDLE &&
        _currentState != MariaState::BLOCK_IDLE)
    {
        setState(MariaState::IDLE);
    }
}

// =========================================================================
// 状态管理方法
// =========================================================================

/**
 * 设置角色状态并切换对应动画
 * @param newState 新状态 */

void Maria::setState(MariaState newState)
{
    if (_currentState == newState)
        return;

    MariaState oldState = _currentState; // 记录旧状态
    _currentState = newState;

    // 攻击/闪避状态结束时更新基准位置
    if (oldState == MariaState::ATTACKING || oldState == MariaState::DODGING) {
        _moveBasePos = getPosition3D();
        _isAttacking = false;           // 确保攻击状态结束
    }

    // 停止循环动作 (Tag 100用于蹲伏/格挡/跳跃的持续动作)
    this->stopActionByTag(100);

    switch (newState)
    {
        case MariaState::IDLE:
            playAnimation(ANIM_IDLE, true);
            _moveSpeed = 0.0f;
            _moveBasePos = getPosition3D(); // 重置基准位置
            break;

        case MariaState::HURT:
            _moveSpeed = 0.0f;
            _isAttacking = false; // 受击时停止所有攻击
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
            // JUMPING状态在runJump()中处理动画
            break;

        case MariaState::DODGING:
            _moveSpeed = 0.0f; // 速度在update的attack逻辑中处理
            break;

        default:
            break;
    }
}

// =========================================================================
// 移动与旋转相关方法
// =========================================================================

/**
 * 获取角色当前前向向量(XZ平面)
 * @return 标准化的前向向量
 */
Vec3 Maria::getForwardVector() const
{
    float yaw = CC_DEGREES_TO_RADIANS(getRotation3D().y);
    return Vec3(sinf(yaw), 0, cosf(yaw)).getNormalized();
}

/**
 * 执行移动逻辑
 * @param direction 输入方向向量
 * @param isRunning 是否为奔跑状态
 */
void Maria::runMove(const Vec3& direction, bool isRunning)
{
    // 攻击状态下仅允许旋转
    if (_isAttacking) {
        if (_isRotationLocked) {
            // 锁定状态下朝向锁定方向
            float targetAngle = CC_RADIANS_TO_DEGREES(atan2f(_lockedDirection.x, _lockedDirection.z));
            setRotation3D(Vec3(0, targetAngle, 0));
        }
        else {
            // 非锁定状态下朝向移动方向
            Vec3 dir(direction.x, 0, direction.z);
            if (dir.lengthSquared() > 0.01f) {
                dir.normalize();
                float angle = CC_RADIANS_TO_DEGREES(atan2f(dir.x, dir.z));
                setRotation3D(getRotation3D().lerp(Vec3(0, angle, 0), 0.2f));
            }
        }
        return;
    }

    // 闪避/攻击状态下不能移动
    if (_currentState == MariaState::DODGING || _isAttacking) {
        return;
    }

    // 1. 处理输入方向 (direction为本地坐标系的方向: X/Z)
    Vec3 moveInput(direction.x, 0, direction.z);
    if (moveInput.lengthSquared() < 0.0001f) {
        // 无输入时停止移动
        if (_currentState == MariaState::WALK || _currentState == MariaState::RUN) {
            setState(MariaState::IDLE);
        }
        return;
    }
    moveInput.normalize();

    // 2. 转换相机角度为弧度
    float cameraYaw = CC_DEGREES_TO_RADIANS(this->_cameraYawAngle);

    // 3. 计算相机坐标系的前向和右向向量 (XZ平面)
    Vec3 camForward = Vec3(sinf(cameraYaw), 0.0f, cosf(cameraYaw));
    Vec3 camRight = Vec3(camForward.z, 0.0f, -camForward.x);

    // 4. 将输入方向转换为世界坐标系方向
    Vec3 moveDir = camForward * moveInput.z - camRight * moveInput.x;
    moveDir.normalize();

    // 角色旋转处理
    if (!_isRotationLocked) {
        // 计算目标角度
        float targetAngle = CC_RADIANS_TO_DEGREES(atan2f(moveDir.x, moveDir.z));
        float currentAngle = getRotation3D().y;
        float angleDiff = targetAngle - currentAngle;

        // 处理角度跨越0/360度边界的情况
        while (angleDiff > 180.0f)  angleDiff -= 360.0f;
        while (angleDiff < -180.0f) angleDiff += 360.0f;

        // 平滑旋转
        float newAngle = currentAngle + angleDiff * 0.2f;
        setRotation3D(Vec3(0, newAngle, 0));
    }
    else {
        // 锁定状态下朝向锁定方向
        float targetAngle = CC_RADIANS_TO_DEGREES(atan2f(_lockedDirection.x, _lockedDirection.z));
        float currentAngle = getRotation3D().y;
        float angleDiff = targetAngle - currentAngle;

        while (angleDiff > 180.0f)  angleDiff -= 360.0f;
        while (angleDiff < -180.0f) angleDiff += 360.0f;

        setRotation3D(Vec3(0, currentAngle + angleDiff, 0));
    }

    // 设置移动状态
    setState(isRunning ? MariaState::RUN : MariaState::WALK);
    _moveDirection = moveDir; // 存储方向用于update()
}

/**
 * 停止移动
 */
void Maria::stopMove()
{
    // 仅在行走/奔跑状态时停止
    if (_currentState == MariaState::WALK || _currentState == MariaState::RUN) {
        setState(MariaState::IDLE);
    }
}

// =========================================================================
// 闪避相关方法
// =========================================================================

/**
 * 执行闪避动作
 * @param direction 闪避方向向量
 */
void Maria::runDodge(const Vec3& direction)
{
    // 仅在idle/walk/run状态可闪避
    if (_currentState != MariaState::IDLE &&
        _currentState != MariaState::WALK &&
        _currentState != MariaState::RUN) {
        return;
    }

    this->stopAllActions();

    // 1. 确定闪避动画和方向
    std::string dodgeAnim = ANIM_DODGE_BACK; // 默认后闪避
    Vec3 worldDodgeDir;

    // 相机坐标系转换
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
// 目标锁定相关方法
// =========================================================================

/**
 * 切换目标锁定状态
 * @param isLocked 是否锁定目标
 * @param targetDir 目标方向向量
 */
void Maria::toggleLock(bool isLocked, const cocos2d::Vec3& targetDir)
{
    _isRotationLocked = isLocked;

    if (isLocked) {
        // 锁定目标方向(XZ平面)
        _lockedDirection = Vec3(targetDir.x, 0, targetDir.z).getNormalized();

        // 朝向目标
        float targetAngle = CC_RADIANS_TO_DEGREES(atan2f(_lockedDirection.x, _lockedDirection.z));
        setRotation3D(Vec3(0, targetAngle, 0));
    }
}

// =========================================================================
// 跳跃相关方法
// =========================================================================

/**
 * 执行跳跃动作
 */
void Maria::runJump()
{
    if (_currentState == MariaState::IDLE ||
        _currentState == MariaState::WALK ||
        _currentState == MariaState::RUN) {
        setState(MariaState::JUMPING);
        playAnimation(ANIM_JUMP, false);

        // 跳跃结束后返回idle
        this->runAction(Sequence::create(
            DelayTime::create(0.8f),
            CallFunc::create([=]() { setState(MariaState::IDLE); }),
            nullptr
        ));
    }
}

// =========================================================================
// 下蹲与格挡相关方法
// =========================================================================

/**
 * 切换下蹲状态
 */
void Maria::toggleCrouch()
{
    if (_currentState == MariaState::IDLE || _currentState == MariaState::WALK) {
        // 从站立切换到下蹲
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
        // 从下蹲切换到站立
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

/**
 * 开始格挡
 */
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

/**
 * 结束格挡
 */
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
// 帧更新方法
// =========================================================================

/**
 * 每帧更新逻辑
 * @param dt 帧间隔时间
 */
void Maria::update(float dt)
{
    // 攻击/闪避状态下的位移更新
    if (_isAttacking && (_currentState == MariaState::ATTACKING ||
        _currentState == MariaState::DODGING)) {
        _attackElapsed += dt;
        float t = std::min(_attackElapsed / _attackDuration, 1.0f);

        float ease = 1.0f - (1.0f - t) * (1.0f - t);
        float currentTotalDist = _attackDistance * ease;

        // 更新基准位置并设置角色位置
        _moveBasePos = _attackStartPos + _attackDirection * currentTotalDist;
        setPosition3D(_moveBasePos);
    }
    // 行走/奔跑状态下的移动更新
    else if (_currentState == MariaState::WALK || _currentState == MariaState::RUN) {
        Vec3 moveDelta = _moveDirection * _moveSpeed * dt;
        _moveBasePos += moveDelta;
        setPosition3D(_moveBasePos);
    }

    // MP自动恢复(非死亡状态)
    if (_currentState != MariaState::DEAD) {
        if (_mp < _maxMp) {
            float regen = _mpRegenRate * dt;
            _mp = std::min((float)_maxMp, (float)_mp + regen);
        }
    }
}

// =========================================================================
// 攻击连招系统
// =========================================================================

/**
 * 执行攻击连招
 */
void Maria::runAttackCombo()
{
    // 1. 状态检查
    if (!canPerformAttack()) return;

    // 2. 处理连招缓冲
    if (_currentState == MariaState::ATTACKING) {
        _isNextComboBuffered = true;
        return;
    }

    // 3. 初始化攻击参数
    _isNextComboBuffered = false;
    _attackStartPos = _moveBasePos;
    _attackDirection = getForwardVector();
    _attackElapsed = 0.0f;
    _isAttacking = true;
    _currentState = MariaState::ATTACKING;

    // 4. 获取当前连招信息
    _comboCount = (_comboCount % 3) + 1;
    std::string nextAnim;
    getComboData(_comboCount, nextAnim, _attackDistance, _attackDuration);

    // 5. 播放攻击动画
    auto anim3d = Animation3D::create(Maria::ANIM_MODEL_PATH, nextAnim);
    auto animateAction = Animate3D::create(anim3d);

    auto attackSequence = Sequence::create(
        animateAction,
        CallFunc::create([this]() {
            this->executeDamageDetection(); // 执行伤害检测
            this->_isAttacking = false;     // 结束攻击状态
            this->handleComboEnd();         // 处理连招结束
            }),
        nullptr
    );

    this->stopAllActions();
    this->runAction(attackSequence);
}

/**
 * 检查是否可以执行攻击
 * @return 是否可以攻击
 */
bool Maria::canPerformAttack() {
    return !(_currentState == MariaState::SKILLING ||
        _currentState == MariaState::HURT ||
        _currentState == MariaState::BLOCK_IDLE ||
        _currentState == MariaState::CROUCH_IDLE);
}

/**
 * 获取连招数据
 * @param combo 连招序号
 * @param animName 输出动画名称
 * @param distance 输出位移距离
 * @param duration 输出持续时间
 */
void Maria::getComboData(int combo, std::string& animName, float& distance, float& duration) {
    if (combo == 1) {
        animName = ANIM_P_ATTACK1;
        distance = 1.8f;
        duration = 0.35f;
    }
    else if (combo == 2) {
        animName = ANIM_P_ATTACK2;
        distance = 3.0f;
        duration = 0.4f;
    }
    else {
        animName = ANIM_P_ATTACK3;
        distance = 15.0f;
        duration = 0.5f;
    }
}

/**
 * 执行伤害检测
 */
void Maria::executeDamageDetection() {
    auto scene = this->getParent();
    if (!scene) return;

    // 攻击中心偏移15.0f
    Vec3 attackCenter = this->getPosition3D() + _attackDirection * 15.0f;

    for (auto node : scene->getChildren()) {
        // 检测普通敌人
        if (auto enemy = dynamic_cast<EnemyBase*>(node)) {
            if (!enemy->isDead() && attackCenter.distance(enemy->getPosition3D()) < 100.0f) {
                enemy->takeDamage(_attackPower);
            }
        }
        // 检测Boss
        else if (auto boss = dynamic_cast<Boss*>(node)) {
            if (!boss->IsDead() && attackCenter.distance(boss->getPosition3D()) < 200.0f) {
                boss->TakeDamage(_attackPower);
            }
        }
    }
}

/**
 * 处理连招结束逻辑
 */
void Maria::handleComboEnd() {
    if (_isNextComboBuffered) {
        // 切换到IDLE状态以允许下一次攻击
        _currentState = MariaState::IDLE;
        runAttackCombo();
    }
    else {
        _comboCount = 0;
        setState(MariaState::IDLE);
    }
}

// =========================================================================
// 影子技能系统
// =========================================================================

/**
 * 执行影子技能
 */
void Maria::runSkillShadow()
{
    // 状态检查
    if (_currentState != MariaState::IDLE &&
        _currentState != MariaState::WALK &&
        _currentState != MariaState::RUN) {
        return;
    }

    // MP检查
    if (_mp < SKILL_MP_COST) {
        CCLOG("MP not enough! current: %d, need: %d", (int)_mp, SKILL_MP_COST);
        return;
    }

    // 消耗MP
    _mp -= SKILL_MP_COST;
    CCLOG("Skill showed! cost %d MP, rest: %d", SKILL_MP_COST, (int)_mp);

    // 执行技能逻辑
    _currentState = MariaState::SKILLING;
    this->stopAllActions();
    this->playAnimation(ANIM_SKILL_START, false);

    // 技能序列帧
    auto skillSequence = Sequence::create(
        DelayTime::create(0.4f),
        CallFunc::create([=]() { spawnGhostShadow(Vec3(5, 0, 10), ANIM_GHOST_1, 0.2f); }),
        DelayTime::create(0.6f),
        CallFunc::create([=]() { spawnGhostShadow(Vec3(0, 0, 0), ANIM_GHOST_2, 0.3f); }),
        DelayTime::create(0.5f),
        CallFunc::create([=]() { spawnGhostShadow(Vec3(0, 0, 8), ANIM_GHOST_3, 0.2f); }),
        DelayTime::create(0.4f),
        CallFunc::create([=]() { spawnGhostShadow(Vec3(0, 0, 15), ANIM_GHOST_4, 0.4f); }),
        DelayTime::create(0.5f),
        CallFunc::create([=]() { spawnGhostShadow(Vec3(-10, 0, 5), ANIM_GHOST_5, 0.2f); }),
        DelayTime::create(0.6f),
        CallFunc::create([=]() { this->onAnimationFinished(ANIM_SKILL_START); }),
        nullptr
    );

    this->runAction(skillSequence);
}

/**
 * 生成影子并执行攻击
 * @param offset 影子相对于角色的偏移
 * @param animName 影子播放的动画
 * @param delayDamage 伤害延迟时间
 */
void Maria::spawnGhostShadow(const Vec3& offset, const std::string& animName, float delayDamage)
{
    // 创建影子实例
    auto ghost = Maria::create(Maria::ANIM_MODEL_PATH);
    if (!ghost) return;

    // 设置影子属性
    ghost->setPosition3D(this->getPosition3D() + offset);
    ghost->setRotation3D(this->getRotation3D());
    ghost->setCameraMask(this->getCameraMask());
    ghost->setCascadeOpacityEnabled(true);
    ghost->setOpacity(180);
    ghost->setLightMask(0);
    ghost->setScale(0.5f);

    // 影子不需要更新逻辑
    ghost->unscheduleUpdate();
    this->getParent()->addChild(ghost);

    // 加载影子动画
    auto anim3d = Animation3D::create(Maria::ANIM_MODEL_PATH, animName);
    auto animate = Animate3D::create(anim3d);

    // 伤害检测逻辑
    auto damageLogic = CallFunc::create([this, ghost]() {
        float damageRange = 60.0f;
        int damageValue = (int)(this->_attackPower * 0.8f);
        Vec3 ghostPos = ghost->getPosition3D();

        auto scene = this->getParent();
        for (auto node : scene->getChildren()) {
            // 普通敌人检测
            auto enemy = dynamic_cast<EnemyBase*>(node);
            if (enemy && !enemy->isDead()) {
                if (ghostPos.distance(enemy->getPosition3D()) < damageRange) {
                    enemy->takeDamage(damageValue);
                }
            }
            // Boss检测
            auto boss = dynamic_cast<Boss*>(node);
            if (boss && !boss->IsDead()) {
                if (ghostPos.distance(boss->getPosition3D()) < 50.0f) {
                    boss->TakeDamage(damageValue);
                }
            }
        }
        });

    // 影子生命周期序列
    ghost->runAction(Sequence::create(
        Spawn::create(
            animate,
            Sequence::create(DelayTime::create(delayDamage), damageLogic, nullptr),
            nullptr
        ),
        CallFunc::create([ghost]() { ghost->removeFromParent(); }),
        nullptr
    ));
}

// =========================================================================
// 伤害与回血系统
// =========================================================================

/**
 * 受到伤害处理
 * @param damage 伤害值
 */
void Maria::takeDamage(int damage) {
    // 免疫状态检查
    if (_currentState == MariaState::DEAD || _currentState == MariaState::DODGING) {
        return;
    }

    // 计算最终伤害(格挡减伤)
    int finalDamage = damage;
    if (_currentState == MariaState::BLOCK_IDLE) {
        finalDamage = std::max(1, (int)(damage * 0.2f)); // 格挡至少受1点伤害
    }

    _hp -= finalDamage;
    CCLOG("Maria took %d damage, remaining HP: %d", finalDamage, _hp);

    // 回血状态特殊处理
    if (_currentState == MariaState::RECOVER) {
        if (_hp <= 0) { /* 处理死亡... */ }
        return;
    }

    // 停止当前所有动作
    this->stopAllActions();

    if (_hp <= 0) {
        _hp = 0;
        setState(MariaState::DEAD);
        playAnimation(ANIM_DEAD, false);
    }
    else {
        setState(MariaState::HURT);
        playAnimation(ANIM_HURT, false);

        // 受击后返回idle
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

/**
 * 执行回血动作
 */
void Maria::runRecover() {
    if (_recoverCount <= 0 ||
        _currentState == MariaState::DEAD ||
        _currentState == MariaState::RECOVER) {
        return;
    }

    // 1. 立即扣除次数并增加血量
    _recoverCount--;
    _hp = std::min(_hp + RECOVER_AMOUNT, 180);

    // 2. 进入回血状态并播放动画
    setState(MariaState::RECOVER);
    auto anim3d = Animation3D::create(Maria::ANIM_MODEL_PATH, ANIM_RECOVER);
    if (anim3d) {
        auto animate = Animate3D::create(anim3d);
        auto seq = Sequence::create(
            animate,
            CallFunc::create([this]() {
                // 动画结束后恢复到 IDLE 状态
                this->setState(MariaState::IDLE);
                this->playAnimation(ANIM_IDLE, true);
                }),
            nullptr
        );
        this->runAction(seq);
    }
}

/**
 * 攻击敌人
 * @param enemy 目标敌人
 */
void Maria::attackEnemy(EnemyBase* enemy) {
    if (enemy && !enemy->isDead()) {
        enemy->takeDamage(_attackPower);
    }
}