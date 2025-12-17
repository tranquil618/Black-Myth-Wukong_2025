#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "Paladin/Paladin.h" 
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <windows.h> // Win32 API, 用于 SetCursorPos
#endif
class HelloWorld : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC(HelloWorld);

    // 摄像机相关常量
    static const float CAMERA_DISTANCE;
    static const float CAMERA_HEIGHT;
    static const float CAMERA_LAG;

private:
    // 模块化函数声明
    bool setupCamera();
    bool setupPaladin();
    void setupInputListeners();
    void setupWorld();
    void setupCursorControl(); // 初始化屏幕中心和鼠标位置

    void update(float dt) override;
    void updateCamera(float dt);

    // --- 成员变量 ---
    cocos2d::Camera* _camera = nullptr;          // 摄像机对象
    Paladin* _paladin = nullptr;        // 主角对象
    cocos2d::Vec3 _cameraCurrentPos;             // 摄像机当前位置（用于平滑过渡）
    std::unordered_map<cocos2d::EventKeyboard::KeyCode, bool> _keys; // 键盘状态

    // 鼠标/镜头控制成员
    float _currentCameraYaw = 0.0f;     // 当前摄像机 Yaw 角 (度数)
    float _mouseSensitivity = 0.5f;     // 鼠标敏感度 (可调整)

    // 上次鼠标位置和屏幕中心 (用于无限旋转)
    float _lastMouseX = 0.0f;
    float _lastMouseY = 0.0f;
    float _screenCenterX = 0.0f;
    float _screenCenterY = 0.0f;
};


#endif // __HELLOWORLD_SCENE_H__