#include "Paladin.h"
#include "base/CCDirector.h"
#include "renderer/CCMaterial.h" 
#include "2d/CCActionInterval.h" // 包含 DelayTime
#include "2d/CCActionInstant.h" // 包含 Sequence, CallFunc

// =========================================================================
// 静态常量定义
// =========================================================================
const std::string Paladin::ANIM_MODEL_PATH = "test.c3b";

const std::string Paladin::ANIM_IDLE = "Armature|mixamo.com.ReadyToAttackIdle";
const std::string Paladin::ANIM_WALK = "Armature|mixamo.com.Walk";
const std::string Paladin::ANIM_RUN = "Armature|mixamo.com.Run";

const std::string Paladin::ANIM_P_ATTACK1 = "Armature|mixamo.com.SwingAndSlash";
const std::string Paladin::ANIM_P_ATTACK2 = "Armature|mixamo.com.SlashFromBottomToTop";
const std::string Paladin::ANIM_P_ATTACK3 = "Armature|mixamo.com.SwingAndSlashSpinForward";

const std::string Paladin::ANIM_SKILL_START = "Armature|mixamo.com.DeclarePosition";
const std::string Paladin::ANIM_GHOST_1 = "Armature|mixamo.com.GreatJumpAttack";
const std::string Paladin::ANIM_GHOST_2 = "Armature|mixamo.com.JumpInPlaceSpinAndSlash";
const std::string Paladin::ANIM_GHOST_3 = "Armature|mixamo.com.Kick";
const std::string Paladin::ANIM_GHOST_4 = "Armature|mixamo.com.SpinAttackForward";
const std::string Paladin::ANIM_GHOST_5 = "Armature|mixamo.com.SpinAttackLeft";

// 新增动作
const std::string Paladin::ANIM_JUMP = "Armature|mixamo.com.JumpInPlace"; // 假设跳跃
const std::string Paladin::ANIM_START_CROUCH = "Armature|mixamo.com.StartingCrouch";
const std::string Paladin::ANIM_CROUCH_IDLE = "Armature|mixamo.com.CrouchingIdle";
const std::string Paladin::ANIM_DE_CROUCH = "Armature|mixamo.com.DeCrouching";

const std::string Paladin::ANIM_START_BLOCK = "Armature|mixamo.com.StartBlock";
const std::string Paladin::ANIM_BLOCK_IDLE = "Armature|mixamo.com.Blocking";
const std::string Paladin::ANIM_DE_BLOCK = "Armature|mixamo.com.DeBlock";


// =========================================================================
// 初始化和动画播放逻辑
// =========================================================================

Paladin* Paladin::create(const std::string& modelPath)
{
    Paladin* paladin = new (std::nothrow) Paladin();
    if (paladin && paladin->init(modelPath))
    {
        paladin->autorelease();
        return paladin;
    }
    CC_SAFE_DELETE(paladin);
    return nullptr;
}

bool Paladin::init(const std::string& modelPath)
{
    if (!Sprite3D::init()) {
        return false;
    }

    if (!Sprite3D::initWithFile(modelPath)) {
        CCLOGERROR("Failed to load 3D model: %s", modelPath.c_str());
        return false;
    }
    _moveBasePos = getPosition3D();
    setState(PaladinState::IDLE);
    this->scheduleUpdate();

    return true;
}

// 播放动画 (包含 Animate3D 修正)
void Paladin::playAnimation(const std::string& animName, bool loop)
{
    this->stopAllActions();

    auto anim = Animation3D::create(Paladin::ANIM_MODEL_PATH, animName);
    if (!anim) {
        CCLOGERROR("Animation not found: %s", animName.c_str());
        setState(PaladinState::IDLE);
        return;
    }

    auto animate = Animate3D::create(anim); // 修正：正确接收 Animation3D*

    if (loop) {
        this->runAction(RepeatForever::create(animate));
    }
    else {
        this->runAction(animate);
    }
}

// 获取角色前向向量（基于Y轴旋转）
Vec3 Paladin::getForwardVector() const
{
    float yaw = CC_DEGREES_TO_RADIANS(getRotation3D().y);
    return Vec3(sinf(yaw), 0, cosf(yaw)).getNormalized(); // 前向向量（XZ平面）
}

// 动画结束后的回调 (用于重置状态和连击)
void Paladin::onAnimationFinished(const std::string& animName)
{
    // 清理连击计时器（仅清理变量，实际Action在setState或Sequence中被清理）
    if (_comboWindowAction) {
        this->stopAction(_comboWindowAction);
        _comboWindowAction = nullptr;
    }

    // 注意：所有攻击后的位置同步和状态切换逻辑，现在都已移动到 runAttackCombo() 中的 Sequence::create 里了。
    // 因此，这里不再需要处理 ATTACKING 状态，只需要保留 IDLE 恢复逻辑（如果需要的话）。

    // 技能结束后的恢复，如果不是攻击状态，且不是保持状态，则恢复IDLE（此逻辑可能需要保留）
    if (_currentState != PaladinState::ATTACKING &&
        _currentState != PaladinState::CROUCH_IDLE &&
        _currentState != PaladinState::BLOCK_IDLE)
    {
        // 确保非循环动作结束后，比如跳跃或技能，能回到 IDLE
        setState(PaladinState::IDLE);
    }
}


// =========================================================================
// 状态和动画切换逻辑 (setState)
// =========================================================================

void Paladin::setState(PaladinState newState)
{
    if (_currentState == newState) 
        return;

    // --- 强制清理攻击连击计时器并同步位置 ---
    if (_comboWindowAction) {
        this->stopAction(_comboWindowAction);
        _comboWindowAction = nullptr;

        // 当攻击动作被新状态（如WALK/RUN）打断时，必须将当前视觉位置赋值给_moveBasePos
        if (_currentState == PaladinState::ATTACKING) {
            _moveBasePos = getPosition3D();
        }
    }
    // ----------------------------------------------------

    PaladinState oldState = _currentState; // 记录旧状态
    _currentState = newState;

    // 清理非循环动作 (Tag 100 用于格挡/下蹲/跳跃的过渡动作)
    this->stopActionByTag(100);

    switch (newState)
    {
        case PaladinState::IDLE:
            playAnimation(ANIM_IDLE, true);
            _moveSpeed = 0.0f;

            // 处理 ATTACKING/JUMPING 状态恢复到 IDLE 时的位置同步
            // 尽管 onAnimationFinished/runJump 结束时会尝试同步，
            // 但如果用户操作打断了这些动作（比如连按移动），保险起见再次同步。
            if (oldState == PaladinState::ATTACKING || oldState == PaladinState::JUMPING) {
                _moveBasePos = getPosition3D();
            }
            break;

        case PaladinState::WALK:
            playAnimation(ANIM_WALK, true);
            _moveSpeed = _walkSpeed; // 使用常量 _walkSpeed 而非硬编码 50.0f
            break;

        case PaladinState::RUN:
            playAnimation(ANIM_RUN, true);
            _moveSpeed = _runSpeed; // 使用常量 _runSpeed 而非硬编码 150.0f
            break;
        case PaladinState::BLOCK_IDLE:
            playAnimation(ANIM_BLOCK_IDLE, true);
            _moveSpeed = 0.0f;
            break;
        case PaladinState::CROUCH_IDLE:
            playAnimation(ANIM_CROUCH_IDLE, true);
            _moveSpeed = 0.0f;
            break;
        case PaladinState::JUMPING:
            // JUMPING 状态在 runJump() 中处理，这里防止意外覆盖
            break;
        default:
            break;
    }
}


// =========================================================================
// 移动和跳跃
// =========================================================================
void Paladin::runMove(const Vec3& direction, bool isRunning)
{
    // 过滤无效状态
    if (_isAttacking) {
        // 攻击中，只允许少量平滑转向，转向方向应由锁定状态决定或保持。
        // 为了简化，我们只允许在非锁定状态下转向，或者保持当前锁定方向。
        if (_isRotationLocked) {
            // 攻击中保持锁定朝向
            float targetAngle = CC_RADIANS_TO_DEGREES(atan2f(_lockedDirection.x, _lockedDirection.z));
            setRotation3D(Vec3(0, targetAngle, 0));
        }
        else {
            // 非锁定攻击中，允许少量朝向输入的转向
            Vec3 dir(direction.x, 0, direction.z);
            if (dir.lengthSquared() > 0.01f) {
                dir.normalize();
                float angle = CC_RADIANS_TO_DEGREES(atan2f(dir.x, dir.z));
                setRotation3D(getRotation3D().lerp(Vec3(0, angle, 0), 0.2f));
            }
        }
        return;
    }

    // 1. 处理输入向量 (direction 视为 相对镜头 的输入: X/Z)
    Vec3 moveInput(direction.x, 0, direction.z);
    if (moveInput.lengthSquared() < 0.0001f) {
        // 如果没有输入，且不是保持状态，则停止移动
        if (_currentState == PaladinState::WALK || _currentState == PaladinState::RUN) {
            setState(PaladinState::IDLE);
        }
        return;
    }
    moveInput.normalize();

    // 相对镜头方向计算世界移动向量**
    // 2. 获取镜头的 Y 轴旋转角 (转换为弧度)
    float cameraYaw = CC_DEGREES_TO_RADIANS(this->_cameraYawAngle);

    // 3. 计算镜头在世界坐标系上的前向和右向向量 (XZ平面)
    Vec3 camForward = Vec3(sinf(cameraYaw), 0.0f, cosf(cameraYaw));
    Vec3 camRight = Vec3(camForward.z, 0.0f, -camForward.x);

    // 4. 计算世界移动方向：将相对输入 (moveInput) 投影到世界坐标系
    // moveInput.z (前后输入) 乘以 camForward
    // moveInput.x (左右输入) 乘以 camRight
    Vec3 moveDir = camForward * moveInput.z - camRight * moveInput.x;
    moveDir.normalize();


    // 角色朝向处理（结合锁定）
    if (!_isRotationLocked) {
        // 未锁定：平滑转向移动方向（人物面向行走方向）
        float targetAngle = CC_RADIANS_TO_DEGREES(atan2f(moveDir.x, moveDir.z));
        float currentAngle = getRotation3D().y;

        // 使用 LERP 平滑转向，避免瞬间跳转
        float newAngle = currentAngle + (targetAngle - currentAngle) * 0.2f;

        setRotation3D(Vec3(0, newAngle, 0));
    }
    else {
        // 已锁定：保持锁定方向（人物面向 _lockedDirection）
        // _lockedDirection 在 toggleLock 中已设置
        float targetAngle = CC_RADIANS_TO_DEGREES(atan2f(_lockedDirection.x, _lockedDirection.z));
        setRotation3D(Vec3(0, targetAngle, 0));
    }
    // -----------------------------------------------------------------

    // 更新移动状态（保留）
    setState(isRunning ? PaladinState::RUN : PaladinState::WALK);

    _moveDirection = moveDir; // 更新位移方向，由 update() 处理
}


void Paladin::stopMove()
{
    // 只有在 WALK/RUN 状态时才停止
    if (_currentState == PaladinState::WALK || _currentState == PaladinState::RUN) {
        setState(PaladinState::IDLE);
    }
}

// =========================================================================
// 锁定逻辑
// =========================================================================
void Paladin::toggleLock(bool isLocked, const cocos2d::Vec3& targetDir)
{
    _isRotationLocked = isLocked;

    if (isLocked) {
        // 确保传入的锁定方向是归一化的 (XZ平面)
        _lockedDirection = Vec3(targetDir.x, 0, targetDir.z).getNormalized();

        // 锁定后，立即将角色朝向目标方向
        float targetAngle = CC_RADIANS_TO_DEGREES(atan2f(_lockedDirection.x, _lockedDirection.z));
        setRotation3D(Vec3(0, targetAngle, 0));
    }
}

void Paladin::runJump()
{
    if (_currentState == PaladinState::IDLE || _currentState == PaladinState::WALK || _currentState == PaladinState::RUN) {
        setState(PaladinState::JUMPING);
        playAnimation(ANIM_JUMP, false);

        // 跳跃动作结束后自动恢复待机
        this->runAction(Sequence::create(
            DelayTime::create(0.8f), // 假设跳跃动作时长 0.8s
            CallFunc::create([=]() { setState(PaladinState::IDLE); }),
            nullptr
        ));
    }
}

// =========================================================================
// 格挡和下蹲 (Q 是切换，RMB 是长按)
// =========================================================================

void Paladin::toggleCrouch()
{
    if (_currentState == PaladinState::IDLE || _currentState == PaladinState::WALK) {
        // 由站立/行走进入下蹲
        playAnimation(ANIM_START_CROUCH, false);

        auto action = Sequence::create(
            DelayTime::create(0.5f), // 假设过渡动画时长
            CallFunc::create([=]() { setState(PaladinState::CROUCH_IDLE); }),
            nullptr
        );
        action->setTag(100); // 标记非循环动作，方便清理
        this->runAction(action);

    }
    else if (_currentState == PaladinState::CROUCH_IDLE) {
        // 由下蹲退出
        playAnimation(ANIM_DE_CROUCH, false);

        auto action = Sequence::create(
            DelayTime::create(0.5f), // 假设过渡动画时长
            CallFunc::create([=]() { setState(PaladinState::IDLE); }),
            nullptr
        );
        action->setTag(100);
        this->runAction(action);
    }
}

void Paladin::startBlock()
{
    if (_currentState == PaladinState::IDLE || _currentState == PaladinState::WALK) {
        playAnimation(ANIM_START_BLOCK, false);

        auto action = Sequence::create(
            DelayTime::create(0.3f), // 假设格挡发起时长
            CallFunc::create([=]() { setState(PaladinState::BLOCK_IDLE); }), // 进入保持状态
            nullptr
        );
        action->setTag(100);
        this->runAction(action);
    }
}

void Paladin::stopBlock()
{
    if (_currentState == PaladinState::BLOCK_IDLE) {
        playAnimation(ANIM_DE_BLOCK, false);

        auto action = Sequence::create(
            DelayTime::create(0.3f), // 假设格挡取消时长
            CallFunc::create([=]() { setState(PaladinState::IDLE); }),
            nullptr
        );
        action->setTag(100);
        this->runAction(action);
    }
}

// =========================================================================
// 帧更新 (Mat4::createRotation 修正)
// =========================================================================
void Paladin::update(float dt)
{
    // ================= 攻击位移（重点） =================
    if (_isAttacking)
    {
        _attackElapsed += dt;

        float t = _attackElapsed / _attackDuration;
        if (t > 1.0f) t = 1.0f;

        // ease-out：前快后慢
        float ease = 1.0f - (1.0f - t) * (1.0f - t);

        float currentDist = _attackDistance * ease;

        Vec3 targetPos =
            _attackStartPos + _attackDirection * currentDist;

        setPosition3D(targetPos);
    }

    // ================= 【关键修正 2：新增】 步行/奔跑位移 =================
    else if (_currentState == PaladinState::WALK || _currentState == PaladinState::RUN)
    {
        // 确保 _moveBasePos 是正确的基准位置

        // 计算本帧位移向量
        Vec3 moveDelta = _moveDirection * _moveSpeed * dt;

        // 更新基准位置
        _moveBasePos += moveDelta;

        // 最终更新角色的世界坐标 (setPosition3D)
        setPosition3D(_moveBasePos);
    }
    // =========================================================================
}


// =========================================================================
// 普攻连击系统 (Combo Attack)
// =========================================================================
void Paladin::runAttackCombo()
{
    // 如果处于格挡、下蹲、技能或受伤状态，则不能攻击
    if (_currentState == PaladinState::SKILLING ||
        _currentState == PaladinState::HURT ||
        _currentState == PaladinState::BLOCK_IDLE ||
        _currentState == PaladinState::CROUCH_IDLE)
    {
        return;
    }

    // 记录攻击前的基准位置（用于计算攻击位移）
    _attackStartPos = _moveBasePos;
    _currentState = PaladinState::ATTACKING;

    // 停止正在运行的连击计时器
    if (_comboWindowAction) {
        this->stopAction(_comboWindowAction);
        _comboWindowAction = nullptr;
    }

    // 连击计数器 (1, 2, 3 循环)
    _comboCount = (_comboCount % 3) + 1;

    std::string nextAnim;
    if (_comboCount == 1) {
        nextAnim = ANIM_P_ATTACK1;
    }
    else if (_comboCount == 2) {
        nextAnim = ANIM_P_ATTACK2;
    }
    else {
        nextAnim = ANIM_P_ATTACK3;
    }

    _currentState = PaladinState::ATTACKING;
    _isAttacking = true;
    _attackElapsed = 0.0f;

    _attackDirection = getForwardVector();

    // 根据第几段普攻，设置位移参数
    if (_comboCount == 1) {
        _attackDistance = 1.8f;
        _attackDuration = 0.35f;
    }
    else if (_comboCount == 2) {
        _attackDistance = 3.0f;
        _attackDuration = 0.4f;
    }
    else { // 第三段
        _attackDistance = 10.0f;
        _attackDuration = 0.5f;
    }

    this->playAnimation(nextAnim); // 播放攻击动画 (这个动画是一个独立的 Action)

    // ------------------- 【修改建议】 -------------------
    // 1. 统一处理所有攻击的计时器，确保在动画结束后调用 onAnimationFinished。
    // 2. 移除重复的 if (_comboCount < 3) 块。

    auto anim3d = Animation3D::create(Paladin::ANIM_MODEL_PATH, nextAnim);
    if (!anim3d) return;

    float totalDuration = anim3d->getDuration();

    // 创建 Animate3D 动作
    auto animateAction = Animate3D::create(anim3d);

    // 2. 创建一个 Sequence 动作来管理攻击流程
    // 动作流程：[播放动画] -> [同步位置并恢复IDLE状态]
    auto attackSequence = Sequence::create(
        // Step A: 播放攻击动画
        animateAction,

        // Step B: 动画结束，立即同步位置并设置状态
        CallFunc::create([=]() {
            // **核心修改：在这里执行原本在 onAnimationFinished 中的位置同步和状态重置逻辑**

            // 攻击位移同步
            Vec3 attackDisplacement = getForwardVector() * _attackDistance;
            Vec3 finalPos = _attackStartPos + attackDisplacement;

            // 设定最终位置并更新基准位置
            this->setPosition3D(finalPos);
            this->_moveBasePos = finalPos;

            // 清理连击计数
            this->_comboCount = 0;
            this->_isAttacking = false;

            // 恢复 IDLE 状态
            // 注意：setState(IDLE) 会调用 playAnimation(ANIM_IDLE, true) 来替换当前动作，
            // 从而彻底结束 Animate3D 的控制。
            this->setState(PaladinState::IDLE);

            // 清理 _comboWindowAction (虽然现在不用它来管理 A3 结束，但保险起见)
            if (this->_comboWindowAction) {
                this->stopAction(this->_comboWindowAction);
                this->_comboWindowAction = nullptr;
            }
            }),
        nullptr
    );

    // 3. 运行整个序列
    this->runAction(attackSequence);
}


// =========================================================================
// 重影分身技能 (Ghost Shadow Skill)
// =========================================================================
void Paladin::runSkillShadow()
{
    // 只有在空闲、行走、奔跑状态才能释放技能
    if (_currentState != PaladinState::IDLE && _currentState != PaladinState::WALK && _currentState != PaladinState::RUN) {
        return;
    }

    _currentState = PaladinState::SKILLING;
    this->stopAllActions();

    // 1. 主角发起动作 (非循环)
    this->playAnimation(ANIM_SKILL_START, false);

    // 2. 重影攻击队列 (使用 Sequence 串联 5 个阶段)
    auto skillSequence = Sequence::create(
        // 等待发起动画进行 0.4s
        DelayTime::create(0.4f),

        // --- 阶段 I: GreatJumpAttack (大跳砸击) ---
        CallFunc::create([=]() {
            // 创建重影
            auto ghost1 = Paladin::create(Paladin::ANIM_MODEL_PATH);
            ghost1->setPosition3D(this->getPosition3D() + Vec3(5, 0, 10)); // 偏右前方
            ghost1->setRotation3D(this->getRotation3D());
            ghost1->setOpacity(180);
            ghost1->setScale(0.1f);
            this->getParent()->addChild(ghost1);

            // 播放动画并结束后移除
            ghost1->runAction(Sequence::create(
                Animate3D::create(Animation3D::create(Paladin::ANIM_MODEL_PATH, ANIM_GHOST_1)),
                CallFunc::create([ghost1]() { ghost1->removeFromParent(); }),
                nullptr
            ));
            }),
        DelayTime::create(0.6f), // 间隔 0.6s

        // --- 阶段 II: JumpInPlaceSpinAndSlash (原地转圈斩击) ---
        CallFunc::create([=]() {
            auto ghost2 = Paladin::create(Paladin::ANIM_MODEL_PATH);
            ghost2->setPosition3D(this->getPosition3D() + Vec3(0, 0, 0)); // 原地出现
            ghost2->setRotation3D(this->getRotation3D());
            ghost2->setOpacity(180);
            ghost2->setScale(0.1f);
            this->getParent()->addChild(ghost2);

            ghost2->runAction(Sequence::create(
                Animate3D::create(Animation3D::create(Paladin::ANIM_MODEL_PATH, ANIM_GHOST_2)),
                CallFunc::create([ghost2]() { ghost2->removeFromParent(); }),
                nullptr
            ));
            }),
        DelayTime::create(0.5f), // 间隔 0.5s

        // --- 阶段 III: Kick (向前踢) ---
        CallFunc::create([=]() {
            auto ghost3 = Paladin::create(Paladin::ANIM_MODEL_PATH);
            ghost3->setPosition3D(this->getPosition3D() + Vec3(0, 0, 8)); // 正前方近距离
            ghost3->setRotation3D(this->getRotation3D());
            ghost3->setOpacity(180);
            ghost3->setScale(0.1f);
            this->getParent()->addChild(ghost3);

            ghost3->runAction(Sequence::create(
                Animate3D::create(Animation3D::create(Paladin::ANIM_MODEL_PATH, ANIM_GHOST_3)),
                CallFunc::create([ghost3]() { ghost3->removeFromParent(); }),
                nullptr
            ));
            }),
        DelayTime::create(0.4f), // 间隔 0.4s

        // --- 阶段 IV: SpinAttackForward (往前转圈斩击) ---
        CallFunc::create([=]() {
            auto ghost4 = Paladin::create(Paladin::ANIM_MODEL_PATH);
            ghost4->setPosition3D(this->getPosition3D() + Vec3(0, 0, 15)); // 正前方远距离
            ghost4->setRotation3D(this->getRotation3D());
            ghost4->setOpacity(180);
            ghost4->setScale(0.1f);
            this->getParent()->addChild(ghost4);

            ghost4->runAction(Sequence::create(
                Animate3D::create(Animation3D::create(Paladin::ANIM_MODEL_PATH, ANIM_GHOST_4)),
                CallFunc::create([ghost4]() { ghost4->removeFromParent(); }),
                nullptr
            ));
            }),
        DelayTime::create(0.5f), // 间隔 0.5s

        // --- 阶段 V: SpinAttackLeft (向左前方转圈斩击) ---
        CallFunc::create([=]() {
            auto ghost5 = Paladin::create(Paladin::ANIM_MODEL_PATH);
            ghost5->setPosition3D(this->getPosition3D() + Vec3(-10, 0, 5)); // 偏左前方
            ghost5->setRotation3D(this->getRotation3D());
            ghost5->setOpacity(180);
            ghost5->setScale(0.1f);
            this->getParent()->addChild(ghost5);

            ghost5->runAction(Sequence::create(
                Animate3D::create(Animation3D::create(Paladin::ANIM_MODEL_PATH, ANIM_GHOST_5)),
                CallFunc::create([ghost5]() { ghost5->removeFromParent(); }),
                nullptr
            ));
            }),
        DelayTime::create(0.6f), // 确保最后一段有时间播放

        // 3. 技能序列完成，主角恢复
        CallFunc::create([=]() {
            this->onAnimationFinished(ANIM_SKILL_START);
            }),
        nullptr
    );

    this->runAction(skillSequence);
}