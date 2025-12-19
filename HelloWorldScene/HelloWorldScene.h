#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include <vector>
#include <unordered_map>

// 使用前向声明，避免头文件循环包含
class Maria;
class EnemyBase;

class HelloWorld : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init() override;
    CREATE_FUNC(HelloWorld);

    // --- 外部访问接口 ---
    /** 获取当前场景所有敌人，用于 Maria 的攻击碰撞检测 */
    const std::vector<EnemyBase*>& getEnemies() const { return _enemies; }

    /** 将敌人添加到场景并同步到管理列表 */
    void addEnemyToScene(EnemyBase* enemy);

    // --- 配置常量 ---
    static const float CAMERA_DISTANCE;
    static const float CAMERA_HEIGHT;
    static const float CAMERA_LAG;

private:
    // --- 场景初始化模块 ---
    bool setupCamera();
    bool setupMaria();
    void setupInputListeners();
    void setupWorld();
    void setupCursorControl();

    // --- 核心逻辑循环 ---
    void update(float dt) override;
    void updateCamera(float dt);

private:
    // --- 核心对象指针 ---
    cocos2d::Camera* _camera = nullptr;
    Maria* _maria = nullptr;

    // --- 数据容器 ---
    std::vector<EnemyBase*> _enemies; // 敌人管理列表
    std::unordered_map<cocos2d::EventKeyboard::KeyCode, bool> _keys; // 键盘按键状态

    // --- 摄像机与控制逻辑变量 ---
    cocos2d::Vec3 _cameraCurrentPos;
    float _currentCameraYaw = 0.0f;
    float _mouseSensitivity = 0.5f;
    float _lastMouseX = 0.0f;
    float _lastMouseY = 0.0f;
};

#endif // __HELLOWORLD_SCENE_H__