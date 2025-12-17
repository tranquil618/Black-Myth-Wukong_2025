// HelloWorldScene.cpp

#include "HelloWorldScene.h"
#include "base/CCEventListenerMouse.h"
#include "platform/CCGLView.h" // 包含 setCursorPosition 接口

USING_NS_CC;

// 常量定义 (使用用户提供的数值)
const float HelloWorld::CAMERA_DISTANCE = 50.0f;
const float HelloWorld::CAMERA_HEIGHT = 20.0f;
const float HelloWorld::CAMERA_LAG = 0.2f;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

#define PALADIN_MODEL "test.c3b" // 确保文件名正确

// =========================================================================
// 模块化设置函数实现
// =========================================================================

// 1. 摄像机初始化
bool HelloWorld::setupCamera()
{
    Director::getInstance()->setClearColor(Color4F(0.5f, 0.5f, 0.5f, 1.0f));

    auto visibleSize = Director::getInstance()->getVisibleSize();

    _camera = Camera::createPerspective(
        60.0f,
        visibleSize.width / visibleSize.height,
        0.5f,
        2000.0f
    );

    _camera->setCameraFlag(CameraFlag::DEFAULT);
    this->addChild(_camera);

    // 初始化摄像机当前位置
    _cameraCurrentPos = _camera->getPosition3D();

    return _camera != nullptr;
}

// 2. 角色初始化
bool HelloWorld::setupPaladin()
{
    _paladin = Paladin::create(PALADIN_MODEL);
    if (!_paladin) {
        CCLOGERROR("Failed to create Paladin. Check if %s exists!", PALADIN_MODEL);
        return false;
    }
    _paladin->setPosition3D(Vec3(0, 0, 0));
    _paladin->setScale(0.1f);
    this->addChild(_paladin);
    return true;
}

// 3. 地面/环境初始化
void HelloWorld::setupWorld()
{
    auto ground = Sprite3D::create("ground.c3b");

    if (ground)
    {
        ground->setScale(10.0f);
        ground->setPosition3D(Vec3(0, 0, 0));
        this->addChild(ground);
    }

    // 光照设置
    auto ambientLight = AmbientLight::create(Color3B(255, 255, 255));
    ambientLight->setIntensity(1.0f);
    this->addChild(ambientLight);
}

// 4. 光标控制和初始化 (实现无限旋转的关键)
void HelloWorld::setupCursorControl()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();

    // 计算 Cocos2d-x 坐标系下的中心点
    _screenCenterX = visibleSize.width / 2.0f;
    _screenCenterY = visibleSize.height / 2.0f;

    // 初始化上次鼠标位置为中心点
    _lastMouseX = _screenCenterX;
    _lastMouseY = _screenCenterY;

    // 隐藏系统光标
    auto glview = Director::getInstance()->getOpenGLView();
    if (glview) {
        glview->setCursorVisible(false); // 隐藏光标
    }

    // 初始重置光标到中心 (防止启动时位移过大)
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
// 强制光标重置：必须使用屏幕坐标
    auto winSize = Director::getInstance()->getWinSize();
    POINT center = { (long)_screenCenterX, (long)_screenCenterY };

    // 将 Cocos2d-x 坐标转换为屏幕坐标
    ClientToScreen((HWND)glview->getWin32Window(), &center);
    SetCursorPos(center.x, center.y);
#endif
}


// 5. 输入监听器设置
void HelloWorld::setupInputListeners()
{
    // --- 键盘事件监听器 (保持不变) ---
    auto keyboardListener = EventListenerKeyboard::create();
    keyboardListener->onKeyPressed = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        _keys[keyCode] = true;
        if (keyCode == EventKeyboard::KeyCode::KEY_1) {
            _paladin->runSkillShadow();
        }
        else if (keyCode == EventKeyboard::KeyCode::KEY_SHIFT) {
            _paladin->runJump();
        }
        else if (keyCode == EventKeyboard::KeyCode::KEY_Q) {
            _paladin->toggleCrouch();
        }
        };
    keyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, Event* event) {
        _keys[keyCode] = false;
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);

    // --- 鼠标事件监听器 ---
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
    // 鼠标移动
    mouseListener->onMouseMove = [this](Event* event) {
        EventMouse* e = dynamic_cast<EventMouse*>(event);

        // 1. 获取当前鼠标位置
        float currentX = e->getCursorX();

        // 2. 计算鼠标在 X 轴上的位移 (deltaX = 当前 X - 上次 X)
        float deltaX = currentX - this->_lastMouseX;

        // 3. 更新摄像机 Yaw 角
        this->_currentCameraYaw -= deltaX * this->_mouseSensitivity;

        // 4. 将新的 Yaw 角传递给 Paladin
        this->_paladin->setCameraYawAngle(this->_currentCameraYaw);

        // 5. 平台相关的光标重置代码，实现无限旋转

        // --- Windows 平台实现 (最常见) ---
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
        {
            auto glview = Director::getInstance()->getOpenGLView();
            if (glview) {

                this->_lastMouseX = currentX;
                this->_lastMouseY = e->getCursorY();
            }
        }
#elif CC_TARGET_PLATFORM == CC_PLATFORM_LINUX || CC_TARGET_PLATFORM == CC_PLATFORM_MAC

#else
        this->_lastMouseX = currentX;
        this->_lastMouseY = e->getCursorY();
#endif
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);
}


// =========================================================================
// 场景核心函数
// =========================================================================

// init 函数 (集成模块化设置)
bool HelloWorld::init()
{
    if (!Scene::init())
    {
        return false;
    }

    // 调用模块化设置函数
    if (!setupCamera()) return false;
    if (!setupPaladin()) return false;

    setupWorld();
    setupCursorControl(); // 必须在设置输入监听前初始化中心点
    setupInputListeners();

    this->scheduleUpdate(); // 开启场景的 update 循环

    return true;
}

// updateCamera 函数 (使用鼠标控制的 Yaw 角)
void HelloWorld::updateCamera(float dt) {
    if (!_paladin || !_camera) return;

    Vec3 targetPos = _paladin->getPosition3D();

    // **使用鼠标控制的 _currentCameraYaw**
    // 加上 180° 使摄像机位于角色后方
    float cameraYaw = CC_DEGREES_TO_RADIANS(this->_currentCameraYaw + 180.0f);
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
    _cameraCurrentPos = _cameraCurrentPos.lerp(desiredCameraPos, 1.0f - CAMERA_LAG);

    // 摄像头看向人物上方一点
    _camera->setPosition3D(_cameraCurrentPos);
    _camera->lookAt(targetPos + Vec3(0, 10, 0), Vec3(0, 1, 0));
}

// update 函数 (只保留一次 updateCamera 调用)
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
}