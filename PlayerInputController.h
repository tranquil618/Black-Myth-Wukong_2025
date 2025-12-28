#ifndef __PLAYER_INPUT_CONTROLLER_H__
#define __PLAYER_INPUT_CONTROLLER_H__

#include "cocos2d.h"
#include "Player/Maria.h"
#include "TPSCameraController.h"

class PlayerInputController : public cocos2d::Ref {
public:
    static PlayerInputController* create(Maria* player, TPSCameraController* cameraCtrl);
    bool init(Maria* player, TPSCameraController* cameraCtrl);
    virtual ~PlayerInputController(); // 必须添加析构函数
    // 每帧调用，处理持续性的移动逻辑
    void update(float dt);

private:
    // 事件初始化
    void setupListeners();
    void handleKeyboardInput();
    void handleMouseInput();

    // 具体逻辑拆分（确保函数短小）
    void onKeyPressed(cocos2d::EventKeyboard::KeyCode code);
    void onKeyReleased(cocos2d::EventKeyboard::KeyCode code);
    void onMouseMove(cocos2d::EventMouse* e);
    void processMovement(float dt);

    // 引用对象
    Maria* _player = nullptr;
    TPSCameraController* _cameraCtrl = nullptr;
    cocos2d::EventListenerKeyboard* _keyboardListener = nullptr;
    cocos2d::EventListenerMouse* _mouseListener = nullptr;
    // 状态记录
    std::unordered_map<cocos2d::EventKeyboard::KeyCode, bool> _keys;
    float _lastMouseX = 0.0f;
	float _lastMouseY = 0.0f;
};

#endif