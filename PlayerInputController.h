#ifndef __PLAYER_INPUT_CONTROLLER_H__
#define __PLAYER_INPUT_CONTROLLER_H__

#include "cocos2d.h"
#include "Player/Maria.h"
#include "TPSCameraController.h"

/**
 * 玩家输入控制器
 * 负责处理键盘、鼠标输入，并映射到玩家和相机的行为
 */
class PlayerInputController : public cocos2d::Ref
{
public:
    /**
     * 创建输入控制器实例
     * @param player 玩家对象
     * @param cameraCtrl 相机控制器
     * @return 控制器实例，创建失败返回nullptr
     */
    static PlayerInputController* create(Maria* player, TPSCameraController* cameraCtrl);

    /**
     * 初始化控制器
     * @param player 玩家对象
     * @param cameraCtrl 相机控制器
     * @return 初始化成功返回true，否则返回false
     */
    bool init(Maria* player, TPSCameraController* cameraCtrl);

    /**
     * 析构函数：清理事件监听器
     */
    virtual ~PlayerInputController();

    /**
     * 每帧更新：处理玩家移动等持续输入
     * @param dt 帧间隔时间
     */
    void update(float dt);

private:
    /**
     * 初始化输入监听器（键盘和鼠标）
     */
    void setupListeners();

    /**
     * 配置键盘事件监听器
     */
    void handleKeyboardInput();

    /**
     * 配置鼠标事件监听器
     */
    void handleMouseInput();

    /**
     * 处理键盘按键按下事件
     * @param code 按键编码
     */
    void onKeyPressed(cocos2d::EventKeyboard::KeyCode code);

    /**
     * 处理键盘按键释放事件
     * @param code 按键编码
     */
    void onKeyReleased(cocos2d::EventKeyboard::KeyCode code);

    /**
     * 处理鼠标移动事件
     * @param e 鼠标事件对象
     */
    void onMouseMove(cocos2d::EventMouse* e);

    /**
     * 处理玩家移动输入（基于当前按键状态）
     * @param dt 帧间隔时间
     */
    void processMovement(float dt);

    // 受控对象
    Maria* _player = nullptr;               // 玩家对象
    TPSCameraController* _cameraCtrl = nullptr;  // 相机控制器

    // 事件监听器
    cocos2d::EventListenerKeyboard* _keyboardListener = nullptr;  // 键盘监听器
    cocos2d::EventListenerMouse* _mouseListener = nullptr;        // 鼠标监听器

    // 输入状态记录
    std::unordered_map<cocos2d::EventKeyboard::KeyCode, bool> _keys;  // 按键状态映射表
    float _lastMouseX = 0.0f;  // 上一帧鼠标X坐标
    float _lastMouseY = 0.0f;  // 上一帧鼠标Y坐标
};

#endif