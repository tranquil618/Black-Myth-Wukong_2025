#include "HelloWorldScene.h"

USING_NS_CC;

// 原有的 CAMERA_DISTANCE 等常量已迁移至 TPSCameraController 内部

Scene* HelloWorld::createScene() {
    return HelloWorld::create();
}

HelloWorld::~HelloWorld() {
    // 必须释放手动 retain 的控制器
    CC_SAFE_RELEASE(_cameraController);
    CC_SAFE_RELEASE(_inputHandler);
}

bool HelloWorld::init() {
    if (!Scene::init()) return false;

    // 1. 初始化基础实体
    if (!setupCamera()) return false;
    if (!setupMaria()) return false;
    if (!setupEnemies()) return false;
    setupWorld();

    // 2. 初始化分离出的控制器 (关键重构点)
    // 相机控制器：负责计算位置和 LookAt
    _cameraController = TPSCameraController::create(_camera, _player);
    _cameraController->retain();

    // 输入控制器：负责键鼠监听，并关联相机控制器的 Yaw 角
    _inputHandler = PlayerInputController::create(_player, _cameraController);
    _inputHandler->retain();

    this->scheduleUpdate();
    return true;
}

// =========================================================================
// 核心逻辑：现在 update 极其简洁
// =========================================================================
void HelloWorld::update(float dt) {
    // 将具体逻辑分发给专门的控制器处理
    if (_cameraController) {
        _cameraController->update(dt);
    }

    if (_inputHandler) {
        _inputHandler->update(dt);
    }
}

// =========================================================================
// 资源加载函数 (每个函数都非常短小)
// =========================================================================

bool HelloWorld::setupCamera() {
    Director::getInstance()->setClearColor(Color4F(0.5f, 0.5f, 0.5f, 1.0f));
    auto visibleSize = Director::getInstance()->getVisibleSize();
    _camera = Camera::createPerspective(60.0f, visibleSize.width / visibleSize.height, 0.5f, 2000.0f);
    _camera->setCameraFlag(CameraFlag::DEFAULT);
    this->addChild(_camera);
    return true;
}

bool HelloWorld::setupMaria() {
    _player = Maria::create("Maria.c3b");
    if (!_player) return false;
    _player->setScale(0.1f);
    this->addChild(_player);
    return true;
}

bool HelloWorld::setupEnemies() {
    _minotaur = EnemyMinotaur::create();
    if (_minotaur && _player) {
        _minotaur->setPosition3D(Vec3(200, 0, 100));
        _minotaur->setTarget(_player); // 建立 AI 追踪目标
        this->addChild(_minotaur);
        return true;
    }
    return false;
}

void HelloWorld::setupWorld() {
    auto ground = Sprite3D::create("ground.c3b");
    if (ground) {
        ground->setScale(10.0f);
        this->addChild(ground);
    }
    this->addChild(AmbientLight::create(Color3B::WHITE));
}