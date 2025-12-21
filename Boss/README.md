# Boss模块 README

## 一、模块说明

本模块实现了游戏中 **Boss** 的完整行为逻辑，负责：
- Boss 的 **状态机管理**
- 行为决策（追击 / 攻击 / 闪避）
- 与玩家（悟空）的 **最小耦合接口交互**

该模块 **不依赖悟空的具体实现**，仅通过 `Node*` 指针获取玩家位置，用于距离判定和朝向计算。

---

## 二、文件结构

```text
Classes/
├── Boss.h        # Boss 类声明，对外接口
├── Boss.cpp      # Boss 行为逻辑与动画控制
Resources/
└── maw.c3b       # Boss 3D 模型与动画文件
```
---

## 三、Boss状态机设计

Boss 内部通过 State 枚举管理行为互斥：
enum class State {
    IDLE,    // 待机
    WALK,    // 行走追击
    RUN,     // 奔跑追击（怒气阶段）
    ATTACK,  // 攻击
    HIT,     // 受击
    DODGE,   // 闪避
    RAGE,    // 怒气爆发
    DEAD     // 死亡
};

状态切换原则：
- ATTACK / HIT / DODGE / DEAD 状态下不会被打断
- 所有一次性行为（攻击 / 受击 / 闪避）结束后统一回到IDLE
- 怒气为 一次性状态转换，不会重复进入

## 四、对外接口说明

### 4.1 创建 Boss

```cpp
Boss* Boss::createBoss(const std::string& modelPath);
````

* `modelPath`：`.c3b` 模型路径
* 创建并初始化Boss
* 返回已 `autorelease` 的 Boss 对象

---

### 4.2 绑定玩家（悟空）

```cpp
void SetTarget(cocos2d::Node* player);
```

* 传入玩家节点指针
**必须在 Boss 创建并加入场景后调用**

---

### 4.3 Boss 受伤接口

```cpp
void TakeDamage(int damage);
```

功能说明：
* 一阶段下 **30% 概率触发闪避**
* 扣除血量并判断是否死亡
* 播放受击效果（红光 + 后退）
* 自动检测并进入怒气状态

---

### 4.4 查询 Boss 是否死亡

```cpp
bool IsDead() const;
```

* 用于关卡逻辑 / UI / 胜负判定
* 当状态为 `State::DEAD` 时返回 `true`

---

## 五、AI 行为逻辑（HandleAI）

```cpp
void HandleAI(float dt);
```

### 5.1 AI 触发前提

* 仅在以下状态运行：

  * `IDLE`
  * `WALK`
  * `RUN`

* 以下状态 **不会进入 AI 判断**：

  * `ATTACK`
  * `HIT`
  * `DODGE`
  * `DEAD`

---

### 5.2 行为判断优先级（由高到低）

1. **攻击范围内**

   * 冷却完成 → 执行攻击
2. **追踪范围内**

   * 一阶段 → `WALK`
   * 二阶段（怒气）→ `RUN`
3. **超出追踪范围**

   * 回到 `IDLE`

---

### 5.3 距离判定参数

* `attack_range`：攻击触发距离
* `track_range`：追击触发距离

---

## 六、攻击系统

### 6.1 攻击入口

```cpp
void PerformAttack(int id);
```

| id | 攻击行为    |
| -- | ------- |
| 0  | 一阶段固定攻击 |
| 1  | 二阶段拳击   |
| 2  | 二阶段跳劈   |
| 3  | 二阶段横扫   |

---

### 6.2 攻击流程

1. 状态切换为 `ATTACK`
2. 停止当前动画（`TAG_ANIM`）
3. 播放对应攻击动画
4. 从 `.c3b` 动态获取动画时长
5. 等待动画结束
6. 调用 `OnAttackComplete()` 恢复状态

---

### 6.3 攻击结束回调

```cpp
void OnAttackComplete();
```

* 状态恢复为 `IDLE`
* 重置攻击冷却
* 若处于怒气状态，恢复持续效果
* 自动播放待机动画

---

## 七、闪避系统（Dodge）

### 7.1 闪避触发

```cpp
void Dodge();
```

触发条件：

* 在 `TakeDamage()` 中触发
* 一阶段 **30% 概率**
* 当前状态不能为：

  * `ATTACK`
  * `DODGE`
  * `DEAD`

---

## 八、怒气系统（Rage）

### 8.1 触发条件

```text
current_blood <= max_blood / 2
```

* 只触发一次
* 进入后 `is_rage = true`

---

### 8.2 怒气流程

1. 播放咆哮动画
2. 等待动画结束
3. 攻击冷却减半
4. 启动持续怒气效果
5. 恢复至 `IDLE`

---

### 8.3 怒气持续效果

| 效果     | Tag               |
| ------ | ----------------- |
| 红色明暗闪烁 | `TAG_RAGE_COLOR`  |
| 呼吸缩放   | `TAG_RAGE_BREATH` |

* 均为 `RepeatForever`
* 在 Boss 析构中统一释放

---

## 九、移动与追击系统

### 9.1 向玩家移动

```cpp
void MoveToPlayer(float dt, bool is_run);
```

* 自动计算方向与旋转
* 一阶段走 / 二阶段跑
* 接近攻击距离后停止移动

---

### 9.2 移动限制

* 攻击距离内不移动
* 以下状态不移动：

  * `ATTACK`
  * `HIT`
  * `DEAD`

---

## 十、死亡流程

```cpp
void Die();
```

执行顺序：

1. 状态 → `DEAD`
2. 停止 AI 与动作
3. 播放死亡动画
4. 等待动画完成
5. 停顿 0.5 秒
6. 淡出 + 缩小
7. `RemoveSelf()`

---

## 十一、资源与动画要求

### 11.1 必需动画名

```text
Armature|maw_idle
Armature|maw_walk
Armature|maw_run
Armature|maw_jumpAttack_2
Armature|maw_punch
Armature|maw_jumpAttack
Armature|maw_swipe
Armature|maw_roar
Armature|mae_dead
Armature|dodge_right
```
