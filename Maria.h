#pragma once

#include "cocos2d.h"
#include "3d/CCSprite3D.h"
#include "3d/CCAnimation3D.h"
#include "3d/CCAnimate3D.h"
#include <map>
#include "Player.h"

USING_NS_CC;

/**
 * Maria角色状态枚举
 * 定义角色可能的所有状态
 */
enum class MariaState {
    IDLE,           // 空闲状态
    WALK,           // 行走状态
    RUN,            // 奔跑状态
    ATTACKING,      // 攻击状态
    SKILLING,       // 技能释放状态
    BLOCK_IDLE,     // 格挡持续状态
    CROUCH_IDLE,    // 蹲伏持续状态
    JUMPING,        // 跳跃状态
    HURT,           // 受击状态
    DODGING,        // 闪避状态
    DEAD            //死亡
};

/**
 * Maria角色类
 * 继承自Sprite3D，实现角色控制核心功能
 */
class Maria : public Sprite3D,public Player
{
public:
    /**
     * 创建Maria实例
     * @param modelPath 模型文件路径
     * @return 成功返回Maria实例，失败返回nullptr
     */
    static Maria* create(const std::string& modelPath);

    /**
     * 初始化函数
     * @param modelPath 模型文件路径
     * @return 初始化成功返回true，否则返回false
     */
    bool init(const std::string& modelPath);


    //------------------------------
    // 角色行为响应函数（供外部调用）
    //------------------------------

    /**
     * 执行攻击连招
     * 实现组合攻击逻辑
     */
    void runAttackCombo();

    /**
     * 获取角色当前朝向（XZ平面）
     * @return 标准化的前向向量
     */
    Vec3 getForwardVector() const;

    /**
     * 执行影子技能
     * 释放特殊技能1效果
     */
    void runSkillShadow();

    /**
     * 执行跳跃动作
     * 响应Shift按键
     */
    void runJump();

    /**
     * 切换蹲伏状态（蹲下/站起）
     * 响应Q按键
     */
    void toggleCrouch();

    /**
     * 开始格挡
     * 响应鼠标右键按下事件
     */
    void startBlock();

    /**
     * 结束格挡
     * 响应鼠标右键释放事件
     */
    void stopBlock();

    /**
     * 移动角色
     * 响应WASD按键，根据Space实现加速移动
     * @param direction 移动方向向量
     * @param isRunning 是否为奔跑状态
     */
    void runMove(const Vec3& direction, bool isRunning);

    /**
     * 停止移动
     * 结束移动状态
     */
    void stopMove();

    /**
     * 执行闪避动作
     * @param direction 闪避方向向量
     */
    void runDodge(const Vec3& direction);

    //------------------------------
    // 相机相关控制函数
    //------------------------------

    /**
     * 设置相机Y轴旋转角度
     * @param angle 角度值（角度制）
     */
    void setCameraYawAngle(float angle) { _cameraYawAngle = angle; }

    /**
     * 切换目标锁定状态
     * @param isLocked 是否锁定目标
     * @param targetDir 目标方向向量，锁定时为目标方向，未锁定时为角色前向
     */
    void toggleLock(bool isLocked, const Vec3& targetDir);

    /**
     * 获取当前是否处于锁定状态
     * @return 锁定状态：true为已锁定，false为未锁定
     */
    bool isRotationLocked() const { return _isRotationLocked; }

    //------------------------------
    // 公共成员变量
    //------------------------------
    bool _isRotationLocked = false;  // 是否处于旋转锁定状态

    // 实现 Player 接口
    virtual void takeDamage(int damage) override;
    virtual cocos2d::Vec3 getPosition3D() const override { return Sprite3D::getPosition3D(); }
    virtual void attackEnemy(EnemyBase* enemy) override;
private:
    //------------------------------
    // 状态管理相关变量
    //------------------------------
    MariaState _currentState = MariaState::IDLE;  // 当前角色状态
    int _comboCount = 0;                          // 连招计数
    bool _isAttacking = false;                    // 是否正在攻击
    bool _isDodging = false;                      // 是否正在闪避

    //------------------------------
    // 攻击相关变量
    //------------------------------
    Vec3 _attackStartPos;         // 攻击开始时的位置（用于位移计算）
    float _attackElapsed = 0.0f;  // 攻击经过的时间
    float _attackDuration = 0.0f; // 攻击总时间
    float _attackDistance = 0.0f; // 攻击位移距离
    Vec3 _attackDirection;        // 攻击方向（本地状态下）

    //------------------------------
    // 血量与伤害相关变量
    //------------------------------
    int _hp = 100;
    int _attackPower = 20;
    float _attackRange = 15.0f; // 攻击检测范围

    //------------------------------
    // 移动相关变量
    //------------------------------
    Vec3 _moveBasePos;            // 移动基准位置
    Vec3 _moveDirection;          // 移动方向向量
    float _moveSpeed = 0.0f;      // 当前移动速度
    const float _walkSpeed = 80.0f;  // 行走速度
    const float _runSpeed = 150.0f;  // 奔跑速度

    //------------------------------
    // 闪避相关变量
    //------------------------------
    const float _dodgeDistance = 12.0f;  // 闪避距离
    const float _dodgeDuration = 0.4f;   // 闪避持续时间

    //------------------------------
    // 相机相关控制变量
    //------------------------------
    float _cameraYawAngle = 0.0f;        // 相机Y轴旋转角度（角度制）
    Vec3 _lockedDirection = Vec3::ZERO;  // 锁定时角色面对的方向

    //------------------------------
    // 连招相关变量
    //------------------------------
    Action* _comboWindowAction = nullptr;  // 连招窗口动作

    //------------------------------
    // 动画资源路径定义
    //------------------------------
    const static std::string ANIM_MODEL_PATH;  // 模型文件路径

    // 基础动画
    const static std::string ANIM_IDLE;        // 空闲动画
    const static std::string ANIM_WALK;        // 行走动画
    const static std::string ANIM_RUN;         // 奔跑动画

    // 攻击动画
    const static std::string ANIM_P_ATTACK1;   // 普通攻击1
    const static std::string ANIM_P_ATTACK2;   // 普通攻击2
    const static std::string ANIM_P_ATTACK3;   // 普通攻击3

    // 技能动画
    const static std::string ANIM_SKILL_START; // 技能开始动画
    const static std::string ANIM_GHOST_1;     // 影子技能1
    const static std::string ANIM_GHOST_2;     // 影子技能2
    const static std::string ANIM_GHOST_3;     // 影子技能3
    const static std::string ANIM_GHOST_4;     // 影子技能4
    const static std::string ANIM_GHOST_5;     // 影子技能5

    // 其他动作
    const static std::string ANIM_JUMP;        // 跳跃动画
    const static std::string ANIM_START_CROUCH;// 开始蹲伏动画
    const static std::string ANIM_CROUCH_IDLE; // 蹲伏持续动画
    const static std::string ANIM_DE_CROUCH;   // 结束蹲伏动画
    const static std::string ANIM_START_BLOCK; // 开始格挡动画
    const static std::string ANIM_BLOCK_IDLE;  // 格挡持续动画
    const static std::string ANIM_DE_BLOCK;    // 结束格挡动画
    const static std::string ANIM_HURT;        // 受伤动画
    const static std::string ANIM_DEAD;        // 死亡动画

    // 闪避相关动画
    const static std::string ANIM_DODGE_BACK;  // 后闪避
    const static std::string ANIM_DODGE_FRONT; // 前闪避
    const static std::string ANIM_DODGE_LEFT;  // 左闪避
    const static std::string ANIM_DODGE_RIGHT; // 右闪避

    //------------------------------
    // 内部函数
    //------------------------------

    /**
     * 播放动画
     * @param animName 动画名称
     * @param loop 是否循环播放
     */
    void playAnimation(const std::string& animName, bool loop = false);

    /**
     * 动画播放完成回调
     * @param animName 完成的动画名称
     */
    void onAnimationFinished(const std::string& animName);

    /**
     * 设置角色状态
     * @param newState 新状态
     */
    void setState(MariaState newState);

    /**
     * 帧更新函数
     * @param dt 帧间隔时间
     */
    void update(float dt) override;
};