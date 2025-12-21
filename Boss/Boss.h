#pragma once
#include "cocos2d.h"

static constexpr int TAG_ANIM = 100; //管理 3D 动画（Animate3D）
static constexpr int TAG_HIT = 101;  //管理受击逻辑（红光、后退、回调）
static constexpr int TAG_RAGE_COLOR = 102;  // 怒气红光 Tag
static constexpr int TAG_RAGE_BREATH = 103;  // 怒气呼吸 Tag

class Boss : public cocos2d::Sprite3D
{
public:
    static Boss* createBoss(const std::string& modelPath);//创建Boss,传入模型路径
    virtual bool init(const std::string& modelPath);//初始化
    virtual void update(float dt) override; //每帧更新逻辑
    virtual ~Boss();

    // 接口 ---
    //创建好悟空后，通过此方法传入悟空的指针
    void SetTarget(cocos2d::Node* player);
    //受到伤害
    void TakeDamage(int damage);
    bool IsDead() const;

private:
    enum class State
    {
        IDLE,    //站立/呼吸
        HIT,     //受击中（不能攻击）
        ATTACK,  //攻击
        DEAD,    //死亡
        RAGE,    //怒气
        DODGE,   //闪避
        WALK,    //走
        RUN      //跑
    };
    State _state;             // 当前状态
    cocos2d::Node* _player; // 悟空
    cocos2d::Action* persistentRed;    // 存储持续变红动作
    cocos2d::Action* persistentBreath; // 存储持续呼吸动作
    std::string _modelPath; // 存储模型路径供重新加载动画使用

    int max_blood;            //满血量
    int current_blood;        //当前血量
    float attack_cooldown;    // 攻击冷却时间
    float attackTimer;        // 计时器
    bool is_rage;             //怒气状态
    float walk_speed = 40.0f; //走速度
    float run_speed = 90.0f;  //跑速度
    float attack_range = 150.0f;//距离在attack_range内攻击
    float track_range = 400.0f;//追踪范围

    // 内部函数
    void HandleAI(float dt);
    float Distance_BossPlayer();//计算boss与悟空距离
    void MoveToPlayer(float dt, bool is_run); // 统一移动接口
    
    void PlayAnim(const std::string& animName, bool loop);//动画统一入口
    void PerformAttack(int id); // 执行特定招式
    void OnAttackComplete();   // 攻击动作结束回调
    void Die();//死亡
    void EnterRage();// 进入怒气状态
    void EnterIdle();//呼吸
    void Dodge();//闪避
};