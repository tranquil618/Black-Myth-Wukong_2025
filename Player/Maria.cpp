#include "Player/Maria.h"
#include "Enemy/EnemyBase.h"
#include "HelloWorldScene.h"
#include "base/CCDirector.h"
#include "renderer/CCMaterial.h" 
#include "2d/CCActionInterval.h" 
#include "2d/CCActionInstant.h" 

USING_NS_CC;

// =========================================================================
// 静态常量定义：动画片段名称需与 .c3b 模型文件内一致
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
const std::string Maria::ANIM_GHOST_5 = "Armature|slash_2";
const std::string Maria::ANIM_JUMP = "Armature|jump_start";
const std::string Maria::ANIM_START_CROUCH = "Armature|crouch_start";
const std::string Maria::ANIM_CROUCH_IDLE = "Armature|crouch_idle";
const std::string Maria::ANIM_DE_CROUCH = "Armature|crouch_end";
const std::string Maria::ANIM_START_BLOCK = "Armature|block_start";
const std::string Maria::ANIM_BLOCK_IDLE = "Armature|block_idle";
const std::string Maria::ANIM_DE_BLOCK = "Armature|block_end";
const std::string Maria::ANIM_DODGE_BACK = "Armature|dodge_back";
const std::string Maria::ANIM_DODGE_FRONT = "Armature|dodge_front";
const std::string Maria::ANIM_DODGE_LEFT = "Armature|dodge_left";
const std::string Maria::ANIM_DODGE_RIGHT = "Armature|dodge_right";
const std::string Maria::ANIM_HIT = "Armature|hit";

// =========================================================================
// 初始化与基础逻辑
// =========================================================================

Maria* Maria::create(const std::string& modelPath) {
    auto ret = new (std::nothrow) Maria();
    if (ret && ret->init(modelPath)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool Maria::init(const std::string& modelPath) {
    if (!Sprite3D::initWithFile(modelPath)) return false;

    this->setScale(0.1f);
    setState(MariaState::IDLE);
    return true;
}

void Maria::setState(MariaState newState) {
    if (_currentState == newState && newState != MariaState::ATTACKING) return;
    _currentState = newState;

    // 根据新状态切换基础动画循环
    switch (_currentState) {
        case MariaState::IDLE:        playAnimation(ANIM_IDLE, true); break;
        case MariaState::WALK:        playAnimation(ANIM_WALK, true); break;
        case MariaState::RUN:         playAnimation(ANIM_RUN, true); break;
        case MariaState::BLOCK_IDLE:  playAnimation(ANIM_BLOCK_IDLE, true); break;
        case MariaState::CROUCH_IDLE: playAnimation(ANIM_CROUCH_IDLE, true); break;
        default: break;
    }
}

void Maria::playAnimation(const std::string& animName, bool loop) {
    this->stopAllActionsByTag(100); // 使用特定 Tag 停止之前的动画动作
    auto anim3d = Animation3D::create(ANIM_MODEL_PATH, animName);
    if (anim3d) {
        auto animate = Animate3D::create(anim3d);
        auto action = loop ? (Action*)RepeatForever::create(animate) : (Action*)animate;
        action->setTag(100);
        this->runAction(action);
    }
}

// =========================================================================
// 核心战斗：普攻连招逻辑拆分
// =========================================================================

/**
 * 触发连招主入口
 */
void Maria::runAttackCombo() {
    // 1. 状态前置检查
    if (_currentState == MariaState::SKILLING || _currentState == MariaState::DEAD)
        return;

    // 2. 连招计数逻辑：如果正在攻击则推进一步
    if (_isAttacking) {
        // 连招窗口期判断逻辑可在此扩展
    }

    // 3. 重置窗口计时器
    if (_comboWindowAction) {
        this->stopAction(_comboWindowAction);
        _comboWindowAction = nullptr;
    }

    _comboCount = (_comboCount % 3) + 1;
    _isAttacking = true;
    _attackStartPos = getPosition3D();
    setState(MariaState::ATTACKING);

    // 4. 执行具体段位动作
    executeAttackStep(_comboCount);
}

/**
 * 执行具体的连招段位动画与位移
 */
void Maria::executeAttackStep(int step) {
    std::string animName;
    float moveDist = 0.0f;
    float hitRadius = 40.0f;

    // 段位参数配置
    if (step == 1) { 
        animName = ANIM_P_ATTACK1; 
        moveDist = 5.0f; 
    }
    else if (step == 2) { 
        animName = ANIM_P_ATTACK2; 
        moveDist = 8.0f;
    }
    else {
        animName = ANIM_P_ATTACK3; 
        moveDist = 15.0f; 
        hitRadius = 60.0f;
    }

    auto anim3d = Animation3D::create(ANIM_MODEL_PATH, animName);
    auto animate = Animate3D::create(anim3d);

    // 动作序列：播放动画 -> 物理位移同步 -> 状态重置
    auto seq = Sequence::create(animate, CallFunc::create([=]() {
        Vec3 finalPos = _attackStartPos + getForwardVector() * moveDist;
        this->setPosition3D(finalPos);
        this->_isAttacking = false;
        this->setState(MariaState::IDLE);

        // 开启连招宽限窗口（0.5秒内不按则重置回第一段）
        _comboWindowAction = Sequence::create(DelayTime::create(0.5f), CallFunc::create([this]() {
            _comboCount = 0;
            }), nullptr);
        this->runAction(_comboWindowAction);
        }), nullptr);

    this->runAction(seq);

    // 5. 触发延迟打击检测（在动画挥砍到中间时生效）
    performHitDetection(0.2f, hitRadius, 15);
}

/**
 * 打击检测逻辑
 */
void Maria::performHitDetection(float delay, float radius, int damage) {
    auto detect = Sequence::create(DelayTime::create(delay), CallFunc::create([=]() {
        // 1. 检查场景和玩家本身是否还活着
        if (!this || !_scene || _hp <= 0) return;

        auto& enemies = _scene->getEnemies();
        for (auto enemy : enemies) {
            // 2. 检查敌人是否有效：不为空、有父节点（没被移除）、没死
            if (enemy && enemy->getParent() && !enemy->isDead()) {
                float dist = getPosition3D().distance(enemy->getPosition3D());
                if (dist <= radius) {
                    enemy->takeDamage(damage);
                }
            }
        }
        }), nullptr);
    this->runAction(detect);
}

// =========================================================================
// 技能系统：影子残影逻辑拆分
// =========================================================================

void Maria::runSkillShadow() {
    if (_currentState == MariaState::SKILLING) return;

    setState(MariaState::SKILLING);
    playAnimation(ANIM_SKILL_START, false);

    // 构建技能释放序列（残影流）
    auto skillSeq = Sequence::create(
        DelayTime::create(0.3f),
        CallFunc::create([this]() { spawnGhostEffect(ANIM_GHOST_1, Vec3(10, 0, 10)); }),
        DelayTime::create(0.4f),
        CallFunc::create([this]() { spawnGhostEffect(ANIM_GHOST_2, Vec3(-10, 0, 15)); }),
        DelayTime::create(0.4f),
        CallFunc::create([this]() { spawnGhostEffect(ANIM_GHOST_3, Vec3(0, 0, 20)); }),
        DelayTime::create(0.5f),
        CallFunc::create([this]() { this->setState(MariaState::IDLE); }),
        nullptr
    );
    this->runAction(skillSeq);
}

/**
 * 创建并运行残影特效
 */
void Maria::spawnGhostEffect(const std::string& animName, const Vec3& offset, float opacity) {
    auto ghost = Maria::create(ANIM_MODEL_PATH);
    if (!ghost) return;

    ghost->setPosition3D(this->getPosition3D() + offset);
    ghost->setRotation3D(this->getRotation3D());
    ghost->setOpacity(opacity);
    ghost->setScale(0.1f);
    this->getParent()->addChild(ghost);

    auto anim = Animate3D::create(Animation3D::create(ANIM_MODEL_PATH, animName));
    ghost->runAction(Sequence::create(anim, CallFunc::create([ghost]() {
        ghost->removeFromParent();
        }), nullptr));
}

// =========================================================================
// 移动与防御系统
// =========================================================================

void Maria::runMove(const Vec3& direction, bool isRunning) {
    if (_isAttacking || _currentState == MariaState::DODGING ||
        _currentState == MariaState::SKILLING)
        return;

    // 设置朝向
    float angle = CC_RADIANS_TO_DEGREES(atan2(direction.x, direction.z));
    this->setRotation3D(Vec3(0, angle, 0));

    // 计算速度
    float currentSpeed = isRunning ? _runSpeed : _walkSpeed;
    Vec3 nextPos = getPosition3D() + direction * currentSpeed * 0.016f; // 简易 delta 补偿
    this->setPosition3D(nextPos);

    setState(isRunning ? MariaState::RUN : MariaState::WALK);
}

void Maria::stopMove() {
    if (_currentState == MariaState::WALK || _currentState == MariaState::RUN) {
        setState(MariaState::IDLE);
    }
}

void Maria::runDodge(const Vec3& direction) {
    if (_currentState == MariaState::DODGING) return;

    setState(MariaState::DODGING);
    std::string dodgeAnim = ANIM_DODGE_FRONT; // 默认前闪

    // 逻辑位移：通过 Action 实现
    auto moveBy = MoveBy::create(0.4f, direction * 50.0f);
    auto anim = Animate3D::create(Animation3D::create(ANIM_MODEL_PATH, dodgeAnim));

    this->runAction(Sequence::create(
        Spawn::create(moveBy, anim, nullptr),
        CallFunc::create([this]() { this->setState(MariaState::IDLE); }),
        nullptr
    ));
}


Vec3 Maria::getForwardVector() const {
    float angle = CC_DEGREES_TO_RADIANS(this->getRotation3D().y);
    return Vec3(sinf(angle), 0, cosf(angle));
}

// 辅助回调（针对非 Action 控制的动画）
void Maria::onAnimationFinished(const std::string& animName) {
    this->setState(MariaState::IDLE);
}

/**
 * 切换下蹲状态 (Q键)
 * 职责：处理 站立->蹲下 与 蹲下->站立 的双向过渡
 */
void Maria::toggleCrouch() {
    if (_currentState == MariaState::DEAD || _isAttacking) return;

    if (_currentState != MariaState::CROUCH_IDLE) {
        // 执行：站立 -> 蹲下
        setState(MariaState::CROUCH_IDLE);

        auto anim = Animate3D::create(Animation3D::create(ANIM_MODEL_PATH, ANIM_START_CROUCH));
        auto seq = Sequence::create(anim, CallFunc::create([this]() {
            // 过渡动作完成后，循环播放蹲下待机
            this->playAnimation(ANIM_CROUCH_IDLE, true);
            }), nullptr);
        this->runAction(seq);
    }
    else {
        // 执行：蹲下 -> 站立 (恢复)
        auto anim = Animate3D::create(Animation3D::create(ANIM_MODEL_PATH, ANIM_DE_CROUCH));
        auto seq = Sequence::create(anim, CallFunc::create([this]() {
            this->setState(MariaState::IDLE);
            }), nullptr);
        this->runAction(seq);
    }
}

/**
 * 开始格挡 (鼠标右键按下)
 */
void Maria::startBlock() {
    if (_currentState == MariaState::DEAD || _isAttacking) return;

    setState(MariaState::BLOCK_IDLE);

    auto anim = Animate3D::create(Animation3D::create(ANIM_MODEL_PATH, ANIM_START_BLOCK));
    auto seq = Sequence::create(anim, CallFunc::create([this]() {
        // 过渡完成后保持格挡姿势
        this->playAnimation(ANIM_BLOCK_IDLE, true);
        }), nullptr);
    this->runAction(seq);
}

/**
 * 停止格挡 (鼠标右键释放)
 */
void Maria::stopBlock() {
    if (_currentState != MariaState::BLOCK_IDLE) return;

    auto anim = Animate3D::create(Animation3D::create(ANIM_MODEL_PATH, ANIM_DE_BLOCK));
    auto seq = Sequence::create(anim, CallFunc::create([this]() {
        this->setState(MariaState::IDLE);
        }), nullptr);
    this->runAction(seq);
}

// =========================================================================
// 移动与位移技能逻辑
// =========================================================================

/**
 * 跳跃/冲刺逻辑 (Shift/Space)
 * 职责：执行向上/向前的物理模拟（简单位移）并播放对应动画
 */
void Maria::runJump() {
    if (_currentState == MariaState::JUMPING || _currentState == MariaState::DEAD) return;

    setState(MariaState::JUMPING);

    // 1. 播放起跳动画
    auto anim = Animate3D::create(Animation3D::create(ANIM_MODEL_PATH, ANIM_JUMP));

    // 2. 模拟简单的跳跃弧线位移 (向上提一段距离再降落)
    auto jumpUp = JumpBy::create(0.8f, Vec2(0, 0), 30.0f, 1);

    auto seq = Sequence::create(
        Spawn::create(anim, jumpUp, nullptr), // 动画与位移同步执行
        CallFunc::create([this]() {
            this->setState(MariaState::IDLE);
            }),
        nullptr
    );

    this->runAction(seq);
}

/**
 * 校验当前角色状态是否允许发起攻击
 * 职责：过滤掉处于 硬直、死亡、技能中、格挡中 的非法攻击状态
 */
bool Maria::canAttack() const {
    // 1. 如果角色已经死亡，禁止任何操作
    if (_currentState == MariaState::DEAD) {
        return false;
    }

    // 2. 如果正在释放影子技能，为了动作完整性，禁止穿插普攻
    if (_currentState == MariaState::SKILLING) {
        return false;
    }

    // 3. 如果处于受击硬直（HURT）或 闪避中（DODGING），通常不能攻击
    if (_currentState == MariaState::HURT || _currentState == MariaState::DODGING) {
        return false;
    }

    // 4. 如果正在格挡或下蹲，可以根据游戏设计决定。
    // 这里默认设计为：格挡时点击左键会自动取消格挡并攻击，所以返回 true。
    // 但如果设计为“格挡时完全不能动”，则需加上判断。

    return true;
}


/**
 * 处理角色受击逻辑
 * @param damage 伤害数值
 * 职责：状态过滤、扣血、播放受击/死亡动画、处理硬直
 */
void Maria::takeDamage(int damage) {
    if (this == nullptr) return;

    // 1. 守卫检查：如果已经死亡或处于闪避无敌帧，免疫伤害
    if (_currentState == MariaState::DEAD || _currentState == MariaState::DODGING) {
        return;
    }

    // 2. 扣除血量（假设 Player 基类或 Maria 类中有 _hp 成员）
    // 如果你还没有定义 _hp，请在 .h 中添加 int _hp = 100;
    _hp -= damage;
    CCLOG("Maria took %d damage, remaining HP: %d", damage, _hp);

    // 3. 死亡判定
    if (_hp <= 0) {
        _hp = 0;
        executeDeath();
        return;
    }

    // 4. 受击反馈（硬直）
    // 如果当前正在攻击，受击会打断攻击动作
    if (this->getNumberOfRunningActions() > 0) {
        this->stopAllActionsByTag(100);
    }            // 停止当前动画动作

    setState(MariaState::HURT);

    auto anim3d = Animation3D::create(ANIM_MODEL_PATH, ANIM_HIT);
    auto animate = Animate3D::create(anim3d);

    // 受击序列：播放受击动画 -> 恢复到 IDLE 状态
    auto hitSequence = Sequence::create(
        animate,
        CallFunc::create([this]() {
            if (this->_currentState != MariaState::DEAD) {
                this->setState(MariaState::IDLE);
            }
            }),
        nullptr
    );

    hitSequence->setTag(100);
    this->runAction(hitSequence);
}

/**
 * 内部私有函数：执行死亡逻辑
 * 职责：播放死亡动画，禁用所有输入反馈
 */
void Maria::executeDeath() {
    setState(MariaState::DEAD);
    this->stopAllActionsByTag(100);

    // 假设你有 ANIM_DEAD 动画，如果没有，通常会停在 ANIM_HIT 的最后一帧或倒地
    // 这里演示播放死亡动画并停留在最后一帧
    auto anim3d = Animation3D::create(ANIM_MODEL_PATH, "Armature|death"); // 请确认你的模型中死亡动画的名称
    if (anim3d) {
        auto animate = Animate3D::create(anim3d);
        this->runAction(animate);
    }

    CCLOG("Maria has been defeated!");
}