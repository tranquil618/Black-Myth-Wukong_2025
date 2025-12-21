#include "PlayerInputController.h"

USING_NS_CC;

PlayerInputController* PlayerInputController::create(Maria* p, TPSCameraController* c) {
    auto ret = new (std::nothrow) PlayerInputController();
    if (ret && ret->init(p, c)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool PlayerInputController::init(Maria* p, TPSCameraController* c) {
    _player = p;
    _cameraCtrl = c;
    setupListeners();
    return true;
}

void PlayerInputController::setupListeners() {
    handleKeyboardInput();
    handleMouseInput();
}

void PlayerInputController::handleKeyboardInput() {
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = [this](EventKeyboard::KeyCode code, Event*) { onKeyPressed(code); };
    listener->onKeyReleased = [this](EventKeyboard::KeyCode code, Event*) { onKeyReleased(code); };
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(listener, 1);
}

void PlayerInputController::onKeyPressed(EventKeyboard::KeyCode code) {
    _keys[code] = true;
    if (code == EventKeyboard::KeyCode::KEY_1) _player->runSkillShadow();
    if (code == EventKeyboard::KeyCode::KEY_X) _player->runJump();
    if (code == EventKeyboard::KeyCode::KEY_Q) _player->toggleCrouch();
    if (code == EventKeyboard::KeyCode::KEY_SPACE) {
        // ÉÁ±ÜÂß¼­·â×°
        Vec3 dir((_keys[EventKeyboard::KeyCode::KEY_D] ? 1 : 0) - (_keys[EventKeyboard::KeyCode::KEY_A] ? 1 : 0),
            0,
            (_keys[EventKeyboard::KeyCode::KEY_W] ? 1 : 0) - (_keys[EventKeyboard::KeyCode::KEY_S] ? 1 : 0));
        _player->runDodge(dir);
    }
}

void PlayerInputController::onKeyReleased(EventKeyboard::KeyCode code) {
    _keys[code] = false;
}

void PlayerInputController::handleMouseInput() {
    auto listener = EventListenerMouse::create();
    listener->onMouseDown = [this](EventMouse* e) {
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) _player->runAttackCombo();
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) _player->startBlock();
        };
    listener->onMouseUp = [this](EventMouse* e) {
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) _player->stopBlock();
        };
    listener->onMouseMove = [this](EventMouse* e) { onMouseMove(e); };
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(listener, 1);
}

void PlayerInputController::onMouseMove(EventMouse* e) {
    float currentX = e->getCursorX();
    float deltaX = currentX - _lastMouseX;

    if (_cameraCtrl) {
        _cameraCtrl->handleMouseMove(deltaX, 0);
        _player->setCameraYawAngle(_cameraCtrl->getYaw());
    }
    _lastMouseX = currentX;
}

void PlayerInputController::update(float dt) {
    processMovement(dt);
}

void PlayerInputController::processMovement(float dt) {
    Vec3 dir = Vec3::ZERO;
    if (_keys[EventKeyboard::KeyCode::KEY_W]) dir.z += 1.0f;
    if (_keys[EventKeyboard::KeyCode::KEY_S]) dir.z -= 1.0f;
    if (_keys[EventKeyboard::KeyCode::KEY_A]) dir.x -= 1.0f;
    if (_keys[EventKeyboard::KeyCode::KEY_D]) dir.x += 1.0f;

    if (dir.lengthSquared() > 0.001f) {
        dir.normalize();
        _player->runMove(dir, _keys[EventKeyboard::KeyCode::KEY_SHIFT]);
    }
    else {
        _player->stopMove();
    }
}