#include "HelloWorldScene.h"
#include "Player/Maria.h"
#include "Enemy/EnemyBase.h"
#include "Enemy/EnemyFactory.h"
#include "platform/CCGLView.h"

USING_NS_CC;

// 常量定义
const float HelloWorld::CAMERA_DISTANCE = 120.0f;
const float HelloWorld::CAMERA_HEIGHT = 40.0f;
const float HelloWorld::CAMERA_LAG = 0.15f;

Scene* HelloWorld::createScene() {
    return HelloWorld::create();
}

#define PALADIN_MODEL "Maria.c3b"

bool HelloWorld::init() {
    if (!Scene::init()) return false;

    // 按顺序初始化各个模块
    if (!setupCamera()) return false;
    if (!setupMaria())  return false;

    setupWorld();
    setupInputListeners();
    setupCursorControl();

    this->scheduleUpdate();
    return true;
}

// 1. 摄像机初始化
bool HelloWorld::setupCamera() {
    Director::getInstance()->setClearColor(Color4F(0.4f, 0.4f, 0.4f, 1.0f));
    auto visibleSize = Director::getInstance()->getVisibleSize();

    _camera = Camera::createPerspective(60.0f, visibleSize.width / visibleSize.height, 0.5f, 2000.0f);
    _camera->setCameraFlag(CameraFlag::DEFAULT);
    this->addChild(_camera);

    _cameraCurrentPos = Vec3(0, CAMERA_HEIGHT, CAMERA_DISTANCE);
    return true;
}

// 2. 角色初始化
bool HelloWorld::setupMaria() {
    _maria = Maria::create(PALADIN_MODEL);
    if (!_maria) return false;

    _maria->setPosition3D(Vec3(0, 0, 0));
    _maria->setScene(this); // 关键：建立双向关联
    this->addChild(_maria);
    return true;
}

// 3. 输入监听优化（关键改动）
void HelloWorld::setupInputListeners() {
    auto eventDispatcher = Director::getInstance()->getEventDispatcher();

    // --- 键盘监听 ---
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        _keys[keyCode] = true;

        // 【单次触发动作】
        if (keyCode == EventKeyboard::KeyCode::KEY_Q)     _maria->toggleCrouch();  // 切换下蹲
        if (keyCode == EventKeyboard::KeyCode::KEY_1)     _maria->runSkillShadow(); // 释放技能
        if (keyCode == EventKeyboard::KeyCode::KEY_C) _maria->runJump();        // 跳跃
        };

    keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        _keys[keyCode] = false;
        };
    eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

    // --- 鼠标监听（处理视角与战斗） ---
    auto mouseListener = EventListenerMouse::create();
    mouseListener->onMouseMove = [this](Event* event) {
        auto e = (EventMouse*)event;
        float diffX = e->getCursorX() - _lastMouseX;
        _currentCameraYaw -= diffX * _mouseSensitivity;
        _lastMouseX = e->getCursorX();
        };

    mouseListener->onMouseDown = [this](Event* event) {
        auto e = (EventMouse*)event;
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            _maria->runAttackCombo(); // 普攻
        }
        else if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            _maria->startBlock();     // 开始格挡
        }
        };

    mouseListener->onMouseUp = [this](Event* event) {
        auto e = (EventMouse*)event;
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) {
            _maria->stopBlock();      // 停止格挡
        }
        };
    eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}

// 4. 世界物体初始化
void HelloWorld::setupWorld() {
    //auto sprite = Sprite3D::create("boss_map.c3b");
    //sprite->setScale(2.0f);
    //sprite->setPosition3D(Vec3(0, -1, 0));
    //this->addChild(sprite);

    // 示例：生成一个哥布林
    auto goblin = EnemyFactory::createEnemy(EnemyType::GOBLIN, Vec3(100, 0, 50));
    if (goblin) {
        goblin->setTarget(_maria);
        this->addEnemyToScene(goblin);
    }
}

void HelloWorld::addEnemyToScene(EnemyBase* enemy) {
    if (enemy) {
        this->addChild(enemy);
        _enemies.push_back(enemy);
    }
}

// 5. 鼠标锁定与中心设置
void HelloWorld::setupCursorControl() {
    auto visibleSize = Director::getInstance()->getVisibleSize();
    _lastMouseX = visibleSize.width / 2;
    _lastMouseY = visibleSize.height / 2;
}

// --- 逻辑循环 ---

void HelloWorld::update(float dt) {
    updateCamera(dt);
    if (!_maria) return;

    // 1. 处理移动输入 (持续按键检测)
    Vec3 direction = Vec3::ZERO;
    if (_keys[EventKeyboard::KeyCode::KEY_W]) direction += Vec3(0, 0, 1);
    if (_keys[EventKeyboard::KeyCode::KEY_S]) direction += Vec3(0, 0, -1);
    if (_keys[EventKeyboard::KeyCode::KEY_A]) direction += Vec3(1, 0, 0);
    if (_keys[EventKeyboard::KeyCode::KEY_D]) direction += Vec3(-1, 0, 0);

    bool isShiftPressed = _keys[EventKeyboard::KeyCode::KEY_SHIFT];

    if (direction.lengthSquared() > 0) {
        // 将 Shift 的状态传递给 runMove
        _maria->runMove(direction, isShiftPressed);
    }
    else {
        _maria->stopMove();
    }

    // 2. 检查并清理已死亡的敌人
    for (auto it = _enemies.begin(); it != _enemies.end(); ) {
        auto enemy = *it;
        if (enemy->isDead()) {
            enemy->removeFromParent(); // 从场景移除
            it = _enemies.erase(it);   // 必须从 vector 中删掉，否则 Maria 会扫到它
        }
        else {
            enemy->update(dt);
            it++;
        }
    }
}

void HelloWorld::updateCamera(float dt) {
    if (!_maria || !_camera) return;

    Vec3 targetPos = _maria->getPosition3D();
    float cameraYaw = CC_DEGREES_TO_RADIANS(this->_currentCameraYaw + 180.0f);
    float cameraPitch = CC_DEGREES_TO_RADIANS(30);

    // 计算旋转偏移
    Vec3 offset(
        sinf(cameraYaw) * cosf(cameraPitch),
        sinf(cameraPitch),
        cosf(cameraYaw) * cosf(cameraPitch)
    );
    offset.normalize();
    offset *= CAMERA_DISTANCE;

    Vec3 desiredCameraPos = targetPos + offset;

    // 平滑插值跟踪
    _cameraCurrentPos = _cameraCurrentPos.lerp(desiredCameraPos, 1.0f - CAMERA_LAG);

    _camera->setPosition3D(_cameraCurrentPos);
    _camera->lookAt(targetPos + Vec3(0, 15, 0), Vec3(0, 1, 0));
}