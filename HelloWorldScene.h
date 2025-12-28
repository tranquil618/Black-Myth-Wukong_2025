#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#include "Player/Maria.h"
#include "Enemy/EnemyGoblin.h"
#include "Enemy/EnemyKnight.h"
#include "Enemy/EnemyMinotaur.h"
#include "Enemy/Boss/Boss.h"
#include "TPSCameraController.h"
#include "PlayerInputController.h"
#include "ui/CocosGUI.h"
#include <vector>

using namespace CocosDenshion;

/**
 * 游戏主场景类
 * 统筹管理：
 * - 玩家控制与状态
 * - 敌人生成与行为
 * - 场景切换逻辑
 * - 相机与输入系统
 * - UI界面显示（HUD、暂停、结束界面等）
 */
class HelloWorld : public cocos2d::Scene
{
public:
    /** 创建场景实例 */
    static cocos2d::Scene* createScene();

    /** 初始化场景（重写） */
    virtual bool init() override;

    /** 帧更新函数（重写） */
    virtual void update(float dt) override;

    /** 关闭按钮回调 */
    void menuCloseCallback(cocos2d::Ref* pSender);

    // Cocos2d-x 创建设备宏
    CREATE_FUNC(HelloWorld);

    /** 析构函数：释放手动持有的资源 */
    virtual ~HelloWorld();

    /** 切换暂停状态 */
    void togglePause();

private:
    //------------------------------
    // 初始化相关函数
    //------------------------------
    /** 初始化相机（设置透视参数与相机标志） */
    void setupCamera();

    /** 初始化玩家角色（创建、设置属性、绑定控制器） */
    void setupPlayer();

    /** 初始化场景环境（光照、天空盒、场景模型、传送门） */
    void setupEnvironment();

    /** 初始化普通敌人（生成各类敌人并加入容器） */
    void setupEnemies();

    /** 初始化基础UI（暂停界面等） */
    void setupUI();

    //------------------------------
    // 游戏逻辑相关函数
    //------------------------------
    /** 生成Boss（设置位置、目标与属性） */
    void spawnBoss();

    /** 检查玩家是否触发传送门（距离判定与场景切换） */
    void checkPortalTeleport();

    /** 显示游戏结束界面（胜利/失败状态） */
    void showEndGameUI(bool isVictory);

    /** 重启游戏回调（切换到新场景实例） */
    void restartGame(cocos2d::Ref* pSender);

    //------------------------------
    // UI相关函数
    //------------------------------
    /** 初始化游戏HUD（玩家状态、Boss状态等） */
    void setupGameUI();

    /** 更新UI显示（每帧刷新血条、状态等） */
    void updateUI(float dt);

    /** 更新恢复道具UI显示 */
    void updateRecoverUI();

    /** 创建暂停界面（遮罩、标题、按钮） */
    void createPauseLayer();

    /** 更新暂停界面中的玩家状态文字 */
    void updateStatusInPause();

    //------------------------------
    // 按钮回调
    //------------------------------
    /** 继续游戏回调 */
    void resumeGame(cocos2d::Ref* pSender);

    /** 退出游戏回调 */
    void quitGame(cocos2d::Ref* pSender);

    //------------------------------
    // 核心对象成员
    //------------------------------
    cocos2d::Camera* _camera = nullptr;               // 游戏主相机
    Maria* _player = nullptr;                         // 玩家角色
    std::vector<EnemyBase*> _enemies;                 // 敌人容器
    TPSCameraController* _cameraController = nullptr; // 相机控制器
    PlayerInputController* _inputController = nullptr;// 输入控制器

    //------------------------------
    // 场景模型成员
    //------------------------------
    cocos2d::Sprite3D* _temple = nullptr;             // 寺庙场景模型
    cocos2d::Sprite3D* _portal = nullptr;             // 传送门模型
    cocos2d::Sprite3D* _colosseum = nullptr;          // 斗兽场场景模型

    //------------------------------
    // 状态控制成员
    //------------------------------
    bool _isLevelSwitched = false;                    // 场景是否已切换（寺庙->斗兽场）
    bool _isGamePaused = false;                       // 游戏是否暂停
    cocos2d::Layer* _pauseLayer = nullptr;            // 暂停界面层
    Boss* _boss = nullptr;                            // Boss对象
    bool _isGameOver = false;                         // 游戏是否结束
    cocos2d::LayerColor* _endGameUI = nullptr;        // 游戏结束界面

    //------------------------------
    // HUD UI组件
    //------------------------------
    cocos2d::DrawNode* _playerHUD = nullptr;          // 左上角玩家状态UI
    cocos2d::DrawNode* _bossHUD = nullptr;            // 顶部Boss状态UI
    cocos2d::Node* _bossUIContainer = nullptr;        // Boss UI容器
    cocos2d::Label* _bossNameLabel = nullptr;         // Boss名称标签
    cocos2d::DrawNode* _recoverUI = nullptr;          // 恢复道具UI
    cocos2d::Label* _statusLabel = nullptr;           // 暂停界面状态文字

    //------------------------------
    // 常量定义
    //------------------------------
    const float TELEPORT_DISTANCE = 60.0f;            // 触发传送的距离阈值
    const cocos2d::Vec3 TEMPLE_DESTINATION = cocos2d::Vec3(0, 0, 0); // 传送目标位置

    //地板和天空盒相关
    cocos2d::Sprite* _secondFloor = nullptr;
    cocos2d::Skybox* _skybox = nullptr;
    cocos2d::Node* _haloEffect;  // 光晕特效 
    cocos2d::Sprite3D* _newModel;    // 新场景模型
};

#endif