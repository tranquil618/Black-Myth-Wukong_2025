#pragma once

#include "cocos2d.h"
#include "3d/CCSprite3D.h"
#include "3d/CCAnimation3D.h"
#include "3d/CCAnimate3D.h"
#include <map>

USING_NS_CC;

// Paladin 状态枚举
enum class PaladinState {
    IDLE,
    WALK,
    RUN,
    ATTACKING,
    SKILLING,
    BLOCK_IDLE,  // 格挡保持
    CROUCH_IDLE, // 下蹲保持
    JUMPING,     // 跳跃
    HURT
};

class Paladin : public Sprite3D
{
public:
    static Paladin* create(const std::string& modelPath);
    bool init(const std::string& modelPath);

    // 核心输入处理函数 (与外部输入绑定)
    void runAttackCombo(); // 鼠标左键
    cocos2d::Vec3 getForwardVector() const; // 获取角色前向向量
    void runSkillShadow(); // 数字 1
    void runJump();        // Shift
    void toggleCrouch();   // Q (切换)
    void startBlock();     // 鼠标右键按下
    void stopBlock();      // 鼠标右键释放

    // 移动函数 (增加 isRunning 参数)
    void runMove(const Vec3& direction, bool isRunning); // WASD + Space
    void stopMove();

private:
    PaladinState _currentState = PaladinState::IDLE;
    int _comboCount = 0;
    cocos2d::Vec3 _attackStartPos; // 记录攻击开始时的位置（用于处理位移）

    bool _isAttacking = false;
    float _attackElapsed = 0.0f;     // 已经过的攻击时间
    float _attackDuration = 0.0f;    // 本段攻击总时长
    float _attackDistance = 0.0f;    // 本段攻击总位移距离
    Vec3  _attackDirection;          // 攻击方向（锁定）
    Vec3  _moveBasePos;
    cocos2d::Vec3 _moveDirection;

    Action* _comboWindowAction = nullptr;
    float _moveSpeed = 0.0f;

    // 动作名称宏定义 (请务必确保这些名称与 c3b 文件中的动画片段名称完全一致)
    const static std::string ANIM_MODEL_PATH;

    const static std::string ANIM_IDLE;
    const static std::string ANIM_WALK;
    const static std::string ANIM_RUN;

    const static std::string ANIM_P_ATTACK1;
    const static std::string ANIM_P_ATTACK2;
    const static std::string ANIM_P_ATTACK3;

    const static std::string ANIM_SKILL_START;

    const static std::string ANIM_GHOST_1;
    const static std::string ANIM_GHOST_2;
    const static std::string ANIM_GHOST_3;
    const static std::string ANIM_GHOST_4;
    const static std::string ANIM_GHOST_5;

    // 新增动作
    const static std::string ANIM_JUMP;          // 起跳动作
    const static std::string ANIM_START_CROUCH;  // 站立 -> 蹲下
    const static std::string ANIM_CROUCH_IDLE;   // 下蹲待机
    const static std::string ANIM_DE_CROUCH;     // 蹲下 -> 站立
    const static std::string ANIM_START_BLOCK;   // 格挡发起
    const static std::string ANIM_BLOCK_IDLE;    // 格挡保持
    const static std::string ANIM_DE_BLOCK;      // 取消格挡

    //移动速度
    const float _walkSpeed = 80.0f;
    const float _runSpeed = 150.0f;

    // 内部帮助函数
    void playAnimation(const std::string& animName, bool loop = false);
    void onAnimationFinished(const std::string& animName);
    void setState(PaladinState newState);

    // 帧更新回调
    void update(float dt) override;
};