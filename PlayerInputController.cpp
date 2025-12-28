#include "PlayerInputController.h"
#include "HelloWorldScene.h"
USING_NS_CC;

PlayerInputController* PlayerInputController::create(Maria* p, TPSCameraController* c)
{
    auto ret = new (std::nothrow) PlayerInputController();
    if (ret && ret->init(p, c))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

PlayerInputController::~PlayerInputController()
{
    auto dispatcher = Director::getInstance()->getEventDispatcher();
    // 移除键盘监听器（避免野指针）
    if (_keyboardListener)
    {
        dispatcher->removeEventListener(_keyboardListener);
    }
    // 移除鼠标监听器
    if (_mouseListener)
    {
        dispatcher->removeEventListener(_mouseListener);
    }
}

bool PlayerInputController::init(Maria* p, TPSCameraController* c)
{
    _player = p;
    _cameraCtrl = c;
    setupListeners();  // 初始化输入监听器
    return true;
}

void PlayerInputController::setupListeners()
{
    handleKeyboardInput();  // 配置键盘输入
    handleMouseInput();     // 配置鼠标输入
}
void PlayerInputController::handleKeyboardInput()
{
    _keyboardListener = EventListenerKeyboard::create();

    // 绑定按键按下回调
    _keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*)
        {
            onKeyPressed(code);
        };

    // 绑定按键释放回调
    _keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*)
        {
            onKeyReleased(code);
        };

    // 注册监听器（固定优先级1）
    Director::getInstance()->getEventDispatcher()
        ->addEventListenerWithFixedPriority(_keyboardListener, 1);
}

void PlayerInputController::onKeyPressed(EventKeyboard::KeyCode code)
{
    _keys[code] = true;  // 记录按键按下状态

    // 技能/动作映射
    if (code == EventKeyboard::KeyCode::KEY_1)
    {
        _player->runSkillShadow();  // 释放影子技能
    }
    else if (code == EventKeyboard::KeyCode::KEY_X)
    {
        _player->runJump();  // 跳跃
    }
    else if (code == EventKeyboard::KeyCode::KEY_Q)
    {
        _player->toggleCrouch();  // 切换蹲下/站立状态
    }
    else if (code == EventKeyboard::KeyCode::KEY_SPACE)
    {
        // 计算闪避方向（基于当前WASD按键状态）
        Vec3 dir(
            (_keys[EventKeyboard::KeyCode::KEY_D] ? 1 : 0) - (_keys[EventKeyboard::KeyCode::KEY_A] ? 1 : 0),
            0,
            (_keys[EventKeyboard::KeyCode::KEY_W] ? 1 : 0) - (_keys[EventKeyboard::KeyCode::KEY_S] ? 1 : 0)
        );
        _player->runDodge(dir);  // 闪避
    }
    else if (code == EventKeyboard::KeyCode::KEY_R)
    {
        _player->runRecover();  // 回血
    }
    else if (code == EventKeyboard::KeyCode::KEY_ESCAPE)
    {
        // 切换游戏暂停状态
        auto scene = dynamic_cast<HelloWorld*>(Director::getInstance()->getRunningScene());
        if (scene)
        {
            scene->togglePause();
        }
    }
}

void PlayerInputController::onKeyReleased(EventKeyboard::KeyCode code)
{
    _keys[code] = false;  // 更新按键释放状态
}

void PlayerInputController::handleMouseInput()
{
    _mouseListener = EventListenerMouse::create();

    // 鼠标按下事件
    _mouseListener->onMouseDown = [this](EventMouse* e)
        {
            if (!_player) return;

            if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT)
            {
                _player->runAttackCombo();  // 左键：攻击连招
            }
            else if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT)
            {
                _player->startBlock();  // 右键：开始格挡
            }
        };

    // 鼠标释放事件
    _mouseListener->onMouseUp = [this](EventMouse* e)
        {
            if (!_player) return;

            if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT)
            {
                _player->stopBlock();  // 右键释放：结束格挡
            }
        };

    // 鼠标移动事件
    _mouseListener->onMouseMove = [this](EventMouse* e)
        {
            onMouseMove(e);  // 处理视角旋转
        };

    // 注册监听器（固定优先级1）
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(_mouseListener, 1);
}

void PlayerInputController::onMouseMove(EventMouse* e)
{
    if (!_cameraCtrl || !_player) return;

    // 计算鼠标移动增量
    float currentX = e->getCursorX();
    float deltaX = currentX - _lastMouseX;

    float currentY = e->getCursorY();
    float deltaY = currentY - _lastMouseY;

    // 当X轴有有效移动时，更新相机视角并同步玩家朝向
    if (std::abs(deltaX) > 0.0001f)
    {
        _cameraCtrl->handleMouseMove(deltaX, deltaY);
        _player->setCameraYawAngle(_cameraCtrl->getYaw());
    }

    // 更新上一帧鼠标位置
    _lastMouseX = currentX;
    _lastMouseY = currentY;
}

void PlayerInputController::update(float dt)
{
    processMovement(dt);  // 处理移动输入
}

void PlayerInputController::processMovement(float dt)
{
    Vec3 dir = Vec3::ZERO;  // 移动方向向量

    // 根据WASD按键计算方向
    if (_keys[EventKeyboard::KeyCode::KEY_W]) dir.z += 1.0f;   // 前
    if (_keys[EventKeyboard::KeyCode::KEY_S]) dir.z -= 1.0f;   // 后
    if (_keys[EventKeyboard::KeyCode::KEY_A]) dir.x -= 1.0f;   // 左
    if (_keys[EventKeyboard::KeyCode::KEY_D]) dir.x += 1.0f;   // 右

    // 若有有效移动方向
    if (dir.lengthSquared() > 0.001f)
    {
        dir.normalize();  // 标准化方向向量
        // 移动（Shift键控制是否加速）
        _player->runMove(dir, _keys[EventKeyboard::KeyCode::KEY_SHIFT]);
    }
    else
    {
        _player->stopMove();  // 停止移动
    }
}