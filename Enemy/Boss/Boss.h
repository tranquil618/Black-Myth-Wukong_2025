#pragma once
#include "cocos2d.h"

// 定义常量标签，防止重复定义
#ifndef BOSS_CONSTANTS
#define BOSS_CONSTANTS
static constexpr int TAG_ANIM = 100;  // 动画动作标签
static constexpr int TAG_HIT = 101;   // 受击动作标签
#endif

/**
 * Boss类 - 继承自Sprite3D，实现Boss的所有行为逻辑
 */
class Boss : public cocos2d::Sprite3D
{
public:
    /**
     * 创建Boss实例
     * @param modelPath 模型文件路径
     * @return Boss实例指针
     */
    static Boss* createBoss(const std::string& modelPath);

    /**
     * 初始化Boss
     * @param modelPath 模型文件路径
     * @return 初始化是否成功
     */
    virtual bool init(const std::string& modelPath);

    /**
     * 帧更新函数
     * @param dt 帧间隔时间
     */
    virtual void update(float dt);

    /**
     * 析构函数
     */
    virtual ~Boss();

    /**
     * 设置攻击目标（玩家）
     * @param player 玩家节点指针
     */
    void setTarget(cocos2d::Node* player);

    /**
     * 承受伤害
     * @param damage 伤害值
     */
    void TakeDamage(int damage);

    /**
     * 判断Boss是否死亡
     * @return 是否死亡
     */
    bool IsDead() const;

    /**
     * 获取当前血量
     * @return 当前血量值
     */
    int getCurrentBlood() const { return current_blood; }

    /**
     * 获取最大血量
     * @return 最大血量值
     */
    int getMaxBlood() const { return max_blood; }

private:
    // Boss状态枚举
    enum class State
    {
        IDLE,       // 闲置
        ATTACK,     // 攻击
        DEAD,       // 死亡
        WALK,       // 行走
        RUN,        // 奔跑
        HIT,        // 受击
        RAGING,     // 狂暴中
        DODGING     // 闪避中
    };

    // AI与逻辑处理
    void HandleAI(float dt);              // 处理AI逻辑
    void MoveToPlayer(float dt);          // 向玩家移动
    void PerformAttack(int type);         // 执行攻击动作
    void Die();                           // 死亡处理
    void enterRageMode();                 // 进入狂暴模式
    void performDodge();                  // 执行闪避动作

    // 动画控制
    void CrossFadeAnim(const std::string& animName, bool loop, float duration = 0.2f);  // 动画切换
    void OnAttackFrameReached(int damage);  // 攻击判定帧处理
    void OnActionFinished();                // 动作结束处理
    float Distance_BossPlayer();            // 计算与玩家的距离

    // 成员变量
    State _state;                          // 当前状态
    cocos2d::Node* _player = nullptr;      // 目标玩家
    std::string _modelPath;                // 模型路径
    std::string _currentAnimName = "";     // 当前播放的动画名称

    int max_blood = 500;                   // 最大血量
    int current_blood;                     // 当前血量
    float attack_cooldown = 7.0f;          // 攻击冷却时间
    float attackTimer = 0.0f;              // 攻击计时器
    bool is_rage = false;                  // 是否处于狂暴状态

    float walk_speed = 60.0f;              // 行走速度
    float run_speed = 90.0f;               // 奔跑速度
    float attack_range = 170.0f;           // 攻击范围
};