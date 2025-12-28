#include "PlayerInputController.h"
#include "HelloWorldScene.h"
USING_NS_CC;

PlayerInputController* PlayerInputController::create(Maria* p, TPSCameraController* c)
{
    auto ret = new (std::nothrow) PlayerInputController();
    if (ret && ret->init(p, c)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

// 1. 析构函数：这是防止崩溃的关键
PlayerInputController::~PlayerInputController() {
    auto dispatcher = Director::getInstance()->getEventDispatcher();
    if (_keyboardListener) {
        dispatcher->removeEventListener(_keyboardListener); // 安全移除键盘监听
    }
    if (_mouseListener) {
        dispatcher->removeEventListener(_mouseListener); // 安全移除鼠标监听
    }
}

bool PlayerInputController::init(Maria* p, TPSCameraController* c) 
{
    _player = p;
    _cameraCtrl = c;
    setupListeners();
    return true;
}

void PlayerInputController::setupListeners()
{
    handleKeyboardInput();
    handleMouseInput();
}

void PlayerInputController::handleKeyboardInput() {
    _keyboardListener = EventListenerKeyboard::create(); // 保存到成员变量
    _keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*) {
        onKeyPressed(code);
        };
    _keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*) {
        onKeyReleased(code);
        };
    Director::getInstance()->getEventDispatcher()
        ->addEventListenerWithFixedPriority(_keyboardListener, 1);
}

void PlayerInputController::onKeyPressed(EventKeyboard::KeyCode code) 
{
    _keys[code] = true;
    if (code == EventKeyboard::KeyCode::KEY_1)
        _player->runSkillShadow();
    if (code == EventKeyboard::KeyCode::KEY_X)
        _player->runJump();
    if (code == EventKeyboard::KeyCode::KEY_Q)
        _player->toggleCrouch();
    if (code == EventKeyboard::KeyCode::KEY_SPACE) {
        // 闪避逻辑封装
        Vec3 dir((_keys[EventKeyboard::KeyCode::KEY_D] ? 1 : 0) - (_keys[EventKeyboard::KeyCode::KEY_A] ? 1 : 0),
            0,
            (_keys[EventKeyboard::KeyCode::KEY_W] ? 1 : 0) - (_keys[EventKeyboard::KeyCode::KEY_S] ? 1 : 0));
        _player->runDodge(dir);
    }
    if (code == EventKeyboard::KeyCode::KEY_R) {
        _player->runRecover(); // 按下R执行回血
    }
    if (code == EventKeyboard::KeyCode::KEY_ESCAPE) {
        // 通过场景指针触发暂停
        auto scene = dynamic_cast<HelloWorld*>(Director::getInstance()->getRunningScene());
        if (scene) {
            // 如果已经暂停了，则恢复；否则开启暂停
            // 假设 HelloWorld 有一个接口 handlePause()
            scene->togglePause();
        }
    }
}

void PlayerInputController::onKeyReleased(EventKeyboard::KeyCode code)
{
    _keys[code] = false;
}

void PlayerInputController::handleMouseInput()
{
    // 1. 创建监听器并保存到成员变量，以便析构时移除
    _mouseListener = EventListenerMouse::create();

    // 2. 鼠标点击逻辑（左键攻击，右键格挡）
    _mouseListener->onMouseDown = [this](EventMouse* e) {
        // 这里的 this 必须指向有效的 PlayerInputController 实例
        if (!_player) return;

        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            _player->runAttackCombo();
        }
        else if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            _player->startBlock();
        }
        };

    // 3. 鼠标松开逻辑（停止格挡）
    _mouseListener->onMouseUp = [this](EventMouse* e) {
        if (!_player) return;

        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            _player->stopBlock();
        }
        };

    // 4. 鼠标移动逻辑（视角旋转）
    _mouseListener->onMouseMove = [this](EventMouse* e) {
        // 执行核心移动逻辑
        this->onMouseMove(e);
        };

    // 5. 使用固定优先级注册（优先级为1）
    // 注意：因为用了 FixedPriority，请务必在析构函数里调用 removeEventListener
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(_mouseListener, 1);
}

void PlayerInputController::onMouseMove(EventMouse* e)
{
    // 增加安全检查
    if (!_cameraCtrl || !_player) return;

    float currentX = e->getCursorX();
    float deltaX = currentX - _lastMouseX;

	float currentY = e->getCursorY();
	float deltaY = currentY - _lastMouseY;

    // 只有当鼠标真的移动了才处理，减少无效计算
    if (std::abs(deltaX) > 0.0001f) {
        _cameraCtrl->handleMouseMove(deltaX, deltaY);
        _player->setCameraYawAngle(_cameraCtrl->getYaw());
    }

    _lastMouseX = currentX;
	_lastMouseY = currentY;
}

void PlayerInputController::update(float dt) 
{
    processMovement(dt);
}

void PlayerInputController::processMovement(float dt) 
{
    Vec3 dir = Vec3::ZERO;
    if (_keys[EventKeyboard::KeyCode::KEY_W])
        dir.z += 1.0f;
    if (_keys[EventKeyboard::KeyCode::KEY_S]) 
        dir.z -= 1.0f;
    if (_keys[EventKeyboard::KeyCode::KEY_A])
        dir.x -= 1.0f;
    if (_keys[EventKeyboard::KeyCode::KEY_D])
        dir.x += 1.0f;

    if (dir.lengthSquared() > 0.001f) {
        dir.normalize();
        _player->runMove(dir, _keys[EventKeyboard::KeyCode::KEY_SHIFT]);
    }
    else {
        _player->stopMove();
    }
}