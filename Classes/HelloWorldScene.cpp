#include "HelloWorldScene.h"
#include "base/CCEventListenerMouse.h" // 包含鼠标监听器

USING_NS_CC;

const float HelloWorld::CAMERA_DISTANCE = 50.0f;
const float HelloWorld::CAMERA_HEIGHT = 20.0f;
const float HelloWorld::CAMERA_LAG = 0.2f;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

#define PALADIN_MODEL "test.c3b" // 再次提醒替换文件名!

bool HelloWorld::init()
{
    if (!Scene::init())
    {
        return false;
    }

    // --- 摄像机初始化 ---
    auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();

    // 创建透视相机（参数：视场角、宽高比、近裁剪面、远裁剪面）
    _camera = cocos2d::Camera::createPerspective(
        60.0f,                  // 视场角（推荐60°-75°，适合第三人称）
        visibleSize.width / visibleSize.height,  // 屏幕宽高比
        0.5f,                   // 近裁剪面（避免过近导致模型被裁剪）
        2000.0f                 // 远裁剪面
    );

    // 设置相机标识为默认相机（必须，否则场景无法通过该相机渲染）
    _camera->setCameraFlag(cocos2d::CameraFlag::DEFAULT);

    // 将相机添加到场景中（不添加则不会生效）
    this->addChild(_camera);

    // 实例化主角 Paladin
    _paladin = Paladin::create(PALADIN_MODEL);
    if (!_paladin) {
        CCLOGERROR("Failed to create Paladin. Check if %s exists!", PALADIN_MODEL);
        return true;
    }
    _paladin->setPosition3D(Vec3(0, 0, 0));
    this->addChild(_paladin);

    // --- 1. 键盘事件监听器 ---
    auto listener = EventListenerKeyboard::create();
    listener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        _keys[keyCode] = true;

        if (keyCode == EventKeyboard::KeyCode::KEY_1) { // 数字 1：技能
            _paladin->runSkillShadow();
        }
        else if (keyCode == EventKeyboard::KeyCode::KEY_SHIFT) { // Shift：跳跃
            _paladin->runJump();
        }
        else if (keyCode == EventKeyboard::KeyCode::KEY_Q) { // Q：下蹲 (切换)
            _paladin->toggleCrouch();
        }
        };

    listener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        _keys[keyCode] = false;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // --- 2. 鼠标事件监听器 ---
    auto mouseListener = EventListenerMouse::create();

    mouseListener->onMouseDown = [this](Event* event) {
        EventMouse* e = dynamic_cast<EventMouse*>(event);

        // 修正：使用正确的枚举名称
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) { // 鼠标左键：普攻
            _paladin->runAttackCombo();
        }
        // 修正：使用正确的枚举名称
        else if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) { // 鼠标右键：格挡开始
            _paladin->startBlock();
        }
        };

    mouseListener->onMouseUp = [this](Event* event) {
        EventMouse* e = dynamic_cast<EventMouse*>(event);

        // 修正：使用正确的枚举名称
        if (e->getMouseButton() == EventMouse::MouseButton::BUTTON_RIGHT) { // 鼠标右键：格挡结束
            _paladin->stopBlock();
        }
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

    auto ground = Sprite3D::create("grass.jpeg");

    if (ground)
    {
        // 2. 调整地面大小和位置

        // 设置缩放，使其变得巨大 (500x500倍)
        // 注意：如果您的平面模型默认是垂直的 (像墙壁)，您需要像之前那样旋转它。
        ground->setScaleX(500.0f);
        ground->setScaleZ(500.0f);

        // **确保它是水平的**
        // 如果您的模型是默认垂直的，需要旋转：
        ground->setRotation3D(Vec3(90, 0, 0));

        // 设置位置（在角色脚下）
        ground->setPosition3D(Vec3(0, 0, 0));

        // 3. 设置光照（可选，但推荐）
        // 确保地面材质对光照有响应。
        auto ambientLight = AmbientLight::create(Color3B(255, 255, 255)); // 亮白色环境光
        ambientLight->setIntensity(1.0f); // 保持高亮度
        this->addChild(ambientLight);

        // 4. 添加地面到场景
        this->addChild(ground);
    }

    this->scheduleUpdate(); // 开启场景的 update 循环

    return true;
}

// --- 3. 持续更新函数 (处理 WALK/RUN 逻辑) ---
// ------------------------------------
// 摄像机跟随逻辑
// ------------------------------------
void HelloWorld::updateCamera(float dt) {
    if (!_paladin || !_camera) return;

    Vec3 targetPos = _paladin->getPosition3D();
    float yaw = CC_DEGREES_TO_RADIANS(_paladin->getRotation3D().y); // 人物朝向

    // 摄像头在人物后方45°方向（俯视角度）
    float cameraYaw = yaw + CC_DEGREES_TO_RADIANS(180); // 与人物朝向相反
    float cameraPitch = CC_DEGREES_TO_RADIANS(30); // 俯视角度（30°向下）

    // 计算摄像头相对于人物的偏移
    Vec3 offset(
        sinf(cameraYaw) * cosf(cameraPitch),
        sinf(cameraPitch),
        cosf(cameraYaw) * cosf(cameraPitch)
    );
    offset.normalize();
    offset *= CAMERA_DISTANCE; // 距离人物的距离

    // 目标摄像头位置 = 人物位置 + 偏移
    Vec3 desiredCameraPos = targetPos + offset;
    // 平滑过渡（使用 CAMERA_LAG）
    _cameraCurrentPos = _cameraCurrentPos.lerp(desiredCameraPos, 1 - CAMERA_LAG);

    // 摄像头看向人物上方一点
    _camera->setPosition3D(_cameraCurrentPos);
    _camera->lookAt(targetPos + Vec3(0, 10, 0), Vec3(0, 1, 0)); // 提高注视点高度，增强俯视感
}



void HelloWorld::update(float dt)
{
    updateCamera(dt);
    if (!_paladin) return;

    Vec3 direction = Vec3::ZERO;

    // ================= 键盘输入 =================
    if (_keys[EventKeyboard::KeyCode::KEY_W]) direction += Vec3(0, 0, 1);
    if (_keys[EventKeyboard::KeyCode::KEY_S]) direction += Vec3(0, 0, -1);
    if (_keys[EventKeyboard::KeyCode::KEY_A]) direction += Vec3(-1, 0, 0);
    if (_keys[EventKeyboard::KeyCode::KEY_D]) direction += Vec3(1, 0, 0);

    bool isMoving = direction.lengthSquared() > 0.0001f;
    bool isRunning = isMoving && _keys[EventKeyboard::KeyCode::KEY_SPACE];

    // ================= 移动指令 =================
    if (isMoving)
    {
        direction.normalize();
        _paladin->runMove(direction, isRunning);
    }
    else
    {
        _paladin->stopMove();
    }

    // ================= 相机更新 =================
    updateCamera(dt);
}

