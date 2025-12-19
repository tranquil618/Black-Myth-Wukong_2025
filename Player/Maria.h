#pragma once

#include "cocos2d.h"
#include "3d/CCSprite3D.h"
#include "3d/CCAnimation3D.h"
#include "3d/CCAnimate3D.h"
#include "Player.h"
#include <map>

USING_NS_CC;

// 前向声明，避免循环包含
class HelloWorld;

/**
 * Maria 角色状态枚举
 * 严格定义角色的每一个行为状态，用于状态机切换
 */
enum class MariaState {
    IDLE,           // 待机
    WALK,           // 行走
    RUN,            // 奔跑
    ATTACKING,      // 普攻中
    SKILLING,       // 技能释放中
    BLOCK_IDLE,     // 格挡保持
    CROUCH_IDLE,    // 下蹲保持
    JUMPING,        // 跳跃
    HURT,           // 受击硬直
    DODGING,        // 闪避无敌帧
    DEAD            // 死亡
};

/**
 * Maria 角色类
 * 继承自 Sprite3D (显示) 和 Player (接口)
 */
class Maria : public Sprite3D, public Player
{
public:
    static Maria* create(const std::string& modelPath);
    virtual bool init(const std::string& modelPath);

    // 基础关联设置
    void setScene(HelloWorld* scene) { _scene = scene; }

    // --- 接口实现 (来自 Player.h) ---
    virtual void takeDamage(int damage) override;
    virtual cocos2d::Vec3 getPosition3D() const override { return Sprite3D::getPosition3D(); }

    // --- 核心动作指令 (由 HelloWorldScene 输入调用) ---
    void runAttackCombo();              // 触发普攻
    void runSkillShadow();              // 1键：影子技能
    void runJump();                     // 跳跃
    void toggleCrouch();                // Q键：切换下蹲
    void startBlock();                  // 鼠标右键：开始格挡
    void stopBlock();                   // 鼠标右键：结束格挡
    void runDodge(const Vec3& direction); // 闪避
    void runMove(const Vec3& direction, bool isRunning); // 移动
    void stopMove();                    // 停止移动

    // --- 辅助工具 ---
    cocos2d::Vec3 getForwardVector() const;

private:
    // =========================================================================
    // 内部逻辑拆分函数 (重构后的工业级私有方法)
    // =========================================================================

    // 1. 状态守卫
    bool canAttack() const;             // 检查当前状态是否允许发起攻击

    // 2. 普攻拆分逻辑
    void executeAttackStep(int step);    // 执行具体某一段连招
    void performHitDetection(float delay, float radius, int damage); // 延迟碰撞检测

    // 3. 技能拆分逻辑
    void spawnGhostEffect(const std::string& animName, const Vec3& offset, float opacity = 180);

    // 4. 动画与状态管理
    void setState(MariaState newState);
    void playAnimation(const std::string& animName, bool loop = false);
    void onAnimationFinished(const std::string& animName);
    void executeDeath();                // 死亡处理逻辑

private:
    // =========================================================================
    // 成员变量
    // =========================================================================
    HelloWorld* _scene = nullptr;       // 指向场景的指针，用于碰撞检测获取敌人列表
    MariaState _currentState = MariaState::IDLE;

    // 战斗相关变量
    int _hp = 100;                      // 角色当前血量
    int _comboCount = 0;                // 当前连招段数
    bool _isAttacking = false;          // 是否处于攻击动作锁死状态
    Vec3 _attackStartPos;               // 攻击开始时的位置（用于计算位移偏移）

    // 动作句柄
    Action* _comboWindowAction = nullptr; // 连招输入窗口期动作

    // 属性常量
    const float _walkSpeed = 80.0f;
    const float _runSpeed = 150.0f;

public:
    // =========================================================================
    // 动画资源常量定义 (对应 .c3b 中的动画名)
    // =========================================================================
    static const std::string ANIM_MODEL_PATH;
    static const std::string ANIM_IDLE;
    static const std::string ANIM_WALK;
    static const std::string ANIM_RUN;
    static const std::string ANIM_P_ATTACK1;
    static const std::string ANIM_P_ATTACK2;
    static const std::string ANIM_P_ATTACK3;
    static const std::string ANIM_SKILL_START;
    static const std::string ANIM_GHOST_1;
    static const std::string ANIM_GHOST_2;
    static const std::string ANIM_GHOST_3;
    static const std::string ANIM_GHOST_4;
    static const std::string ANIM_GHOST_5;
    static const std::string ANIM_JUMP;
    static const std::string ANIM_START_CROUCH;
    static const std::string ANIM_CROUCH_IDLE;
    static const std::string ANIM_DE_CROUCH;
    static const std::string ANIM_START_BLOCK;
    static const std::string ANIM_BLOCK_IDLE;
    static const std::string ANIM_DE_BLOCK;
    static const std::string ANIM_DODGE_BACK;
    static const std::string ANIM_DODGE_FRONT;
    static const std::string ANIM_DODGE_LEFT;
    static const std::string ANIM_DODGE_RIGHT;
    static const std::string ANIM_HIT;
};