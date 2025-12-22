#include "HelloWorldScene.h"
#include "cocos2d.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <windows.h>
#endif

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

bool HelloWorld::init()
{
    if (!Scene::init()) return false;
    // 设置背景颜色：深邃的夜空蓝
    // 参数是：红, 绿, 蓝, 透明度 (范围 0.0 ~ 1.0)
    Director::getInstance()->setClearColor(Color4F(0.05f, 0.1f, 0.25f, 1.0f));

    auto size = Director::getInstance()->getWinSize();
    // ==========================================
   // 创建摄像机 (并保存到成员变量 _camera)
   // ==========================================
    _camera = Camera::createPerspective(60, size.width / size.height, 1.0f, 20000.0f);
    _camera->setPosition3D(Vec3(0, 50, 200));
    _camera->lookAt(Vec3(0, 0, 0));
    _camera->setCameraFlag(CameraFlag::USER1);

    // 参数1: 颜色 (深蓝色)
    // 参数2: 深度 (通常设为 1.0f)
    auto bgBrush = CameraBackgroundBrush::createColorBrush(Color4F(0.05f, 0.1f, 0.25f, 1.0f), 1.0f);
    _camera->setBackgroundBrush(bgBrush);

    this->addChild(_camera);

    // ==========================================
    // 天空盒 (Skybox)
    // ==========================================
    auto skybox = Skybox::create(
        "picture/right.png",
        "picture/left.png",
        "picture/up.png",
        "picture/down.png",
        "picture/front.png",
        "picture/back.png"
    );

    if (skybox) {
        // 1. 设置掩码
        skybox->setCameraMask((unsigned short)CameraFlag::USER1);

        // 2. 稍微缩小一点点 (防止极少数情况下被裁剪)
        skybox->setScale(1.0f);

        // 3. 加到场景
        this->addChild(skybox);
    }
    else {
        // 🔴 如果这里报错，说明图片路径不对，或者尺寸不是2的幂
        CCLOG("❌ Skybox 加载失败！请检查 sky 文件夹内图片的尺寸和名字！");
    }


    // ==========================================
    // 键盘监听 (设置按键开关)
    // ==========================================
    auto listener = EventListenerKeyboard::create();

    // 按下键：开关打开
    listener->onKeyPressed = [=](EventKeyboard::KeyCode code, Event* event) {
        switch (code) {
        case EventKeyboard::KeyCode::KEY_W: _isWPressed = true; break;
        case EventKeyboard::KeyCode::KEY_S: _isSPressed = true; break;
        case EventKeyboard::KeyCode::KEY_A: _isAPressed = true; break; // A 左移
        case EventKeyboard::KeyCode::KEY_D: _isDPressed = true; break; // D 右移
        }
        };

    // 松开键：开关关闭
    listener->onKeyReleased = [=](EventKeyboard::KeyCode code, Event* event) {
        switch (code) {
        case EventKeyboard::KeyCode::KEY_W: _isWPressed = false; break;
        case EventKeyboard::KeyCode::KEY_S: _isSPressed = false; break;
        case EventKeyboard::KeyCode::KEY_A: _isAPressed = false; break;
        case EventKeyboard::KeyCode::KEY_D: _isDPressed = false; break;
        }
        };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    // ==========================================
    // 鼠标监听 (按住左键拖动旋转)
    // ==========================================
    auto mouseListener = EventListenerMouse::create();

    // 1. 按下鼠标：开启“拖动模式”
    mouseListener->onMouseDown = [=](EventMouse* event) {
        // 只响应左键 (BUTTON_LEFT)
        if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            _isDragging = true;

            // ⚠️ 关键修正：按下的一瞬间，立刻更新“上一次鼠标位置”
            // 否则视角会因为之前的鼠标移动而突然“跳”一下
            _lastMousePos = event->getLocation();
        }
        };

    // 2. 松开鼠标：关闭“拖动模式”
    mouseListener->onMouseUp = [=](EventMouse* event) {
        if (event->getMouseButton() == EventMouse::MouseButton::BUTTON_LEFT) {
            _isDragging = false;
        }
        };

    // 3. 移动鼠标：只有在拖动模式下才旋转
    mouseListener->onMouseMove = [=](EventMouse* event) {
        // 如果没按左键无效果
        if (!_isDragging) {
            return;
        }

        Vec2 currentPos = event->getLocation();


        float sensitivity = 0.1f;
        float deltaX = currentPos.x - _lastMousePos.x;
        float deltaY = currentPos.y - _lastMousePos.y;

        _lastMousePos = currentPos;

        _rotationY -= deltaX * sensitivity;
        _rotationX -= deltaY * sensitivity;

        // 限制角度 (不可以和看天看地)
        if (_rotationX > 89.0f) _rotationX = 89.0f;
        if (_rotationX < -89.0f) _rotationX = -89.0f;

        _camera->setRotation3D(Vec3(_rotationX, _rotationY, 0));
        };

    _eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, this);

    // 开启每帧更新 

    this->scheduleUpdate();

    // ==========================================
   // 灯光与模型 
    // ==========================================
    auto light = AmbientLight::create(Color3B(200, 200, 200));
    light->setCameraMask((unsigned short)CameraFlag::USER1);
    this->addChild(light);

    auto model = Sprite3D::create("3d1/temple2.c3b");
    if (model) {
        model->setScale(0.05f);
        model->setPosition3D(Vec3(0, 10, 0));
        model->setRotation3D(Vec3(0, 0, 0));
        model->setCameraMask((unsigned short)CameraFlag::USER1);
        this->addChild(model);
    }
    // ==========================================
    // 加载传送门 
    // ==========================================

    auto portal = Sprite3D::create("3d2/portal.c3b");

    if (portal) {
        // --- 1. 大小调整 ---
        portal->setScale(0.025f);

        // --- 2. 位置调整 ---
        // X=0 (居中)
        // Y
        // Z(放在寺庙深处，你需要走过去才能看见)
        portal->setPosition3D(Vec3(0, 25, -115));

        // --- 3. 角度修正 ---
        portal->setRotation3D(Vec3(0, -40, 0));

        // --- 4. 摄像机掩码 ---
        portal->setCameraMask((unsigned short)CameraFlag::USER1);

        this->addChild(portal);

        // 想让它上下浮动
        auto moveUp = MoveBy::create(1.0f, Vec3(0, 10, 0));
        auto seq = Sequence::create(moveUp, moveUp->reverse(), nullptr);
        portal->runAction(RepeatForever::create(seq));

        Vec3 targetPos = Vec3(0, 25, -115);
        Vec3 targetRot = Vec3(0, 0, 0);

        // =========================================================
        // 特效：淡雅的银河
        // =========================================================
        auto halo = ParticleGalaxy::create();

        // 1. 位置与旋转
        halo->setPosition3D(targetPos);
        halo->setRotation3D(targetRot);

        // 2. 颜色变淡：使用低透明度的淡紫色
        // R=0.6, G=0.5, B=1.0 (淡紫)
        // A=0.3 (透明度只有 30%，非常通透，不会遮挡后面的东西)
        halo->setStartColor(Color4F(0.6f, 0.5f, 1.0f, 0.3f));

        // 结束时完全消失
        halo->setEndColor(Color4F(0.0f, 0.0f, 0.0f, 0.0f));

        // 3. 混合模式：叠加模式
        // 这种模式下，粒子像光一样叠加，只会增加亮度，不会产生黑烟，看起来很轻盈
        halo->setBlendFunc(BlendFunc::ADDITIVE);

        // 4. 形态调整 (慢一点，少一点)
        halo->setScale(0.6f);            // 稍微大一点范围
        halo->setTotalParticles(80);     // 粒子数量少一点 (之前是200)
        halo->setEmissionRate(20);       // 发射得慢一点，稀疏一点
        halo->setSpeed(40.0f);           // 飘动的速度
        halo->setLife(2.0f);             // 存活时间长一点，让它慢慢消失

        // 5. 贴图
        halo->setTexture(Director::getInstance()->getTextureCache()->addImage("CloseSelected.png"));

        // 6. 摄像机掩码
        halo->setCameraMask((unsigned short)CameraFlag::USER1);

        this->addChild(halo);

        // =========================================================
        // 柔和的微光
        // =========================================================
        // 范围变小 (200)，颜色变淡 (100, 80, 200)
        auto haloLight = PointLight::create(targetPos, Color3B(100, 80, 200), 200.0f);
        haloLight->setCameraMask((unsigned short)CameraFlag::USER1);

        // 缓慢的呼吸效果
        auto pulse = Sequence::create(
            TintTo::create(3.0f, 50, 40, 100), // 慢慢变暗
            TintTo::create(3.0f, 100, 80, 200), // 慢慢变亮
            nullptr
        );
        haloLight->runAction(RepeatForever::create(pulse));

        this->addChild(haloLight);
    }
    else {
        CCLOG("Error: 找不到 3d2/portal.c3b，请检查文件名！");
    }

    // 关闭按钮 
    auto closeItem = MenuItemImage::create("CloseNormal.png", "CloseSelected.png",
        CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    closeItem->setPosition(Vec2(size.width - 50, 50));
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu);

    // ==========================================
    // 添加背景音乐 (BGM)
    // ==========================================

    // 1. 获取音频引擎实例
    auto audio = SimpleAudioEngine::getInstance();

    // 2. 预加载音乐 (防止第一次播放卡顿)
    audio->preloadBackgroundMusic("music/bgm.mp3");

    // 3. 调节音量 (0.0 到 1.0)
    // 0.5 是半音量，防止把你耳朵震聋
    audio->setBackgroundMusicVolume(0.5f);

    // 4. 开始播放
    // 参数1: 文件名
    // 参数2: true = 单曲循环 (一直在神殿里放), false = 只放一次
    audio->playBackgroundMusic("music/bgm.mp3", true);

    return true;
}

void HelloWorld::update(float dt)
{
    // 1. 安全检查
    if (!_camera) return;

    // 2. 定义移动速度 
    float speed = 100.0f * dt;

    // 3. 【核心算法】计算 FPS 移动方向
    // 根据当前鼠标旋转的角度(_rotationY)来决定前后左右
    float radians = CC_DEGREES_TO_RADIANS(_rotationY);
    float sinVal = sinf(radians);
    float cosVal = cosf(radians);

    Vec3 nextPos = _camera->getPosition3D();

    // 4. 根据按键更新位置 (随视角移动)
    if (_isWPressed) {
        nextPos.x -= speed * sinVal;
        nextPos.z -= speed * cosVal;
    }
    if (_isSPressed) {
        nextPos.x += speed * sinVal;
        nextPos.z += speed * cosVal;
    }
    if (_isAPressed) {
        nextPos.x -= speed * cosVal;
        nextPos.z += speed * sinVal;
    }
    if (_isDPressed) {
        nextPos.x += speed * cosVal;
        nextPos.z -= speed * sinVal;
    }

    // =========================================================
    // 空气墙限制 
    // =========================================================

    // X轴限制：如果不小心走到了 -24，强行拉回 -23
    if (nextPos.x < -23.0f) {
        nextPos.x = -23.0f;
    }
    else if (nextPos.x > 23.0f) {
        nextPos.x = 23.0f;
    }

    // Z轴限制：如果不小心走到了 -101，强行拉回 -100
    if (nextPos.z < -100.0f) {
        nextPos.z = -100.0f;
    }
    else if (nextPos.z > 100.0f) {
        nextPos.z = 100.0f;
    }

    // 5. 应用最终修正后的位置
    _camera->setPosition3D(nextPos);
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    Director::getInstance()->end();
}