#include "HelloWorldScene.h"
#include "cocos2d.h"
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include <windows.h>
#endif
#include "SimpleAudioEngine.h"

USING_NS_CC;
using namespace CocosDenshion;

//------------------------------
// 场景创建与生命周期
//------------------------------

/**
 * 创建游戏主场景
 */
Scene* HelloWorld::createScene() {
    return HelloWorld::create();
}

/**
 * 析构函数：释放控制器资源
 * 注：控制器未加入节点树，需手动释放
 */
HelloWorld::~HelloWorld() {
    CC_SAFE_RELEASE(_cameraController);
    CC_SAFE_RELEASE(_inputController);
}

/**
 * 初始化场景
 * 流程：初始化相机->玩家->环境->敌人->UI->启动帧更新
 */
bool HelloWorld::init() {
    if (!Scene::init()) return false;

    setupCamera();
    setupPlayer();
    setupEnvironment();
    setupEnemies();
    setupUI();
    setupGameUI();

    this->scheduleUpdate();
    return true;
}

//------------------------------
// 帧更新逻辑
//------------------------------

/**
 * 帧更新函数
 * 处理游戏核心逻辑：
 * - 状态检查（暂停/结束/玩家死亡）
 * - 控制器更新（输入+相机）
 * - 敌人与Boss更新
 * - 场景切换逻辑
 */
void HelloWorld::update(float dt) {
    // 暂停/结束状态直接返回
    if (_isGamePaused || _isGameOver) return;

    // 玩家为空时终止更新
    if (!_player) return;

    // 玩家死亡判定（带最低HP容错）
    if (_player->getHP() <= 0) {
        _isGameOver = true;
        this->showEndGameUI(false);
        return;
    }

    // 更新控制器（输入+相机）
    if (_player && _inputController) _inputController->update(dt);
    if (_camera && _cameraController) _cameraController->update(dt);

    // =========================================================
// 🧱 5. 新增：空气墙逻辑 (Air Walls)
// =========================================================

// 获取控制器更新后的最新位置
    Vec3 currentPos = _player->getPosition3D();
    bool needCorrection = false; // 标记是否需要修正

    if (!_isLevelSwitched) {
        // --- 场景一：寺庙 (长条形走廊) ---
        // 基于传送门位置: Vec3(0, 25, -2500)

        // A. X轴限制 (走廊宽度)
        // 假设走廊总宽 300 (左右各 150)，根据实际模型微调这个数字
        float limitX = 400.0f;

        if (currentPos.x < -limitX) {
            currentPos.x = -limitX;
            needCorrection = true;
        }
        else if (currentPos.x > limitX) {
            currentPos.x = limitX;
            needCorrection = true;
        }

        // B. Z轴限制 (走廊长度)
        // 起点：Z = 200 (入口缓冲)
        // 终点：Z = -2550 (传送门在 -2500，多给 50 空间让你能撞到门)
        float startZ = 200.0f;
        float endZ = -2550.0f;

        if (currentPos.z > startZ) {
            currentPos.z = startZ; // 封锁入口，不能后退
            needCorrection = true;
        }
        else if (currentPos.z < endZ) {
            currentPos.z = endZ;   // 封锁尽头，不能穿过传送门后面的墙
            needCorrection = true;
        }
    }

    // 如果位置出界了，应用修正后的位置
    if (needCorrection) {
        _player->setPosition3D(currentPos);
    }

    // Boss战逻辑
    if (_isLevelSwitched && _boss) {
        // Boss死亡判定（延迟显示胜利界面）
        if (_boss->IsDead()) {
            _isGameOver = true;
            this->runAction(Sequence::create(
                DelayTime::create(2.0f),  // 延迟2秒显示，预留死亡动画时间
                CallFunc::create([this]() { showEndGameUI(true); }),
                nullptr
            ));
            return;
        }
        _boss->update(dt);
    }

    updateUI(dt); // 更新UI显示

    // 普通敌人更新与清理
    auto it = _enemies.begin();
    while (it != _enemies.end()) {
        if (*it == nullptr) {
            it = _enemies.erase(it);  // 移除空指针
        }
        else if ((*it)->isDead()) {
            (*it)->removeFromParent(); // 从场景移除
            it = _enemies.erase(it);   // 从容器移除
        }
        else {
            (*it)->update(dt);         // 更新敌人状态
            ++it;
        }
    }

    // 场景切换逻辑（仅在未切换且所有敌人被消灭时触发）
    if (!_isLevelSwitched && _enemies.empty()) {
        checkPortalTeleport();
    }
}

//------------------------------
// 基础交互回调
//------------------------------

/**
 * 关闭按钮回调：退出游戏
 */
void HelloWorld::menuCloseCallback(Ref* pSender) {
    Director::getInstance()->end();
}

//------------------------------
// 初始化相关实现
//------------------------------

/**
 * 初始化相机
 * 配置：透视相机，60度FOV，绑定USER1相机标志
 */
void HelloWorld::setupCamera() {
    auto size = Director::getInstance()->getWinSize();
    _camera = Camera::createPerspective(60, size.width / size.height, 1.0f, 20000.0f);
    _camera->setCameraFlag(CameraFlag::USER1);
    this->addChild(_camera);
}

/**
 * 初始化玩家
 * 流程：创建玩家->设置相机可见性->初始化控制器并绑定
 */
void HelloWorld::setupPlayer() {
    // 创建玩家角色
    _player = Maria::create("Maria.c3b");
    _player->setCameraMask((unsigned short)CameraFlag::USER1);
    _player->setGlobalZOrder(100);
    _player->setScale(0.4f);
    this->addChild(_player);

    // 初始化相机控制器（绑定相机与玩家）
    _cameraController = TPSCameraController::create(_camera, _player);
    _cameraController->retain(); // 手动持有，需在析构释放
    _cameraController->setDistance(250.0f); // 设置初始视角距离

    // 初始化输入控制器（绑定玩家与相机旋转）
    _inputController = PlayerInputController::create(_player, _cameraController);
    _inputController->retain();
}

/**
 * 初始化基础UI
 * 构建暂停界面：半透明遮罩->标题->继续/退出按钮
 */
void HelloWorld::setupUI() {
    auto size = Director::getInstance()->getWinSize();

    // 创建暂停界面容器
    _pauseLayer = Layer::create();

    // 半透明背景遮罩（暗化效果）
    auto shade = LayerColor::create(Color4B(0, 0, 0, 150));
    _pauseLayer->addChild(shade);

    // 暂停标题
    auto labelPaused = Label::createWithSystemFont("GAME PAUSED", "Arial", 60);
    labelPaused->setPosition(Vec2(size.width / 2, size.height * 0.7f));
    _pauseLayer->addChild(labelPaused);

    // 继续按钮
    auto itemResume = MenuItemFont::create("RESUME", [=](Ref* sender) {
        _isGamePaused = false;
        _pauseLayer->setVisible(false);
        SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
        });

    // 退出按钮
    auto itemExit = MenuItemFont::create("EXIT", [=](Ref* sender) {
        Director::getInstance()->end();
        });

    // 按钮布局
    auto pauseMenu = Menu::create(itemResume, itemExit, nullptr);
    pauseMenu->alignItemsVerticallyWithPadding(40);
    pauseMenu->setPosition(Vec2(size.width / 2, size.height * 0.4f));
    _pauseLayer->addChild(pauseMenu);

    // 初始隐藏，置于顶层（避免被场景模型遮挡）
    _pauseLayer->setVisible(false);
    this->addChild(_pauseLayer, 9999);
}

/**
 * 初始化场景环境
 * 包含：环境光->天空盒->寺庙模型->传送门（带动画与粒子效果）
 */
void HelloWorld::setupEnvironment() {
    // 环境光（基础照明）
    auto ambientLight = AmbientLight::create(Color3B(200, 200, 200));
    ambientLight->setCameraMask((unsigned short)CameraFlag::USER1);
    this->addChild(ambientLight);

    // 天空盒（背景）
    _skybox = Skybox::create(
        "background/background/picture/right.png", "background/background/picture/left.png",
        "background/background/picture/up.png", "background/background/picture/down.png",
        "background/background/picture/front.png", "background/background/picture/back.png"
    );

    if (_skybox) {
        _skybox->setCameraMask((unsigned short)CameraFlag::USER1);
        _skybox->setScale(1.0f);
        this->addChild(_skybox);
    }
    else {
        CCLOG("Skybox 加载失败！");
    }

    // 寺庙场景模型
    _temple = Sprite3D::create("background/background/3d/temple1.c3b");
    if (_temple) {
        _temple->setCameraMask((unsigned short)CameraFlag::USER1);
        _temple->setPosition3D(Vec3(0, -242, 0));
        this->addChild(_temple);
    }

    // 传送门（带浮动动画与粒子特效）
    _portal = Sprite3D::create("background/background/3d/portal.c3b");
    if (_portal) {
        _portal->setPosition3D(Vec3(0, 25, -2500));
        _portal->setRotation3D(Vec3(0, -40, 0));
        _portal->setScale(0.05f);
        _portal->setCameraMask((unsigned short)CameraFlag::USER1);
        this->addChild(_portal);

        // 浮动动画
        auto moveUp = MoveBy::create(1.0f, Vec3(0, 10, 0));
        _portal->runAction(RepeatForever::create(Sequence::create(moveUp, moveUp->reverse(), nullptr)));

        _haloEffect = Node::create();

        // 1. 让容器直接对齐传送门的位置
        _haloEffect->setPosition3D(_portal->getPosition3D());
        _haloEffect->setRotation3D(_portal->getRotation3D());

        this->addChild(_haloEffect);

        // 2. 粒子特效 (加到容器里)
        auto halo = ParticleGalaxy::create();
        halo->setScale(0.6f); // 调整大小
        // 位置：让它位于传送门圆环中心
        halo->setPosition3D(Vec3(-20, 5, -25));
        halo->setCameraMask((unsigned short)CameraFlag::USER1);

        _haloEffect->addChild(halo); // 🟢 加到 _haloEffect，而不是 this

    }

    // 添加背景音乐
    auto audio = SimpleAudioEngine::getInstance();
    audio->preloadBackgroundMusic("background/background/music/bgm1.mp3");
    audio->preloadBackgroundMusic("background/background/music/bgm2.mp3");
    audio->setBackgroundMusicVolume(0.5f);
    audio->playBackgroundMusic("background/background/music/bgm1.mp3", true);
}

//------------------------------
// 场景切换相关实现
//------------------------------

/**
 * 检查传送门触发条件
 * 逻辑：玩家进入传送范围->切换场景->加载斗兽场->召唤Boss
 */
void HelloWorld::checkPortalTeleport() {
    // 防重复触发锁 + 安全检查
    if (!_isLevelSwitched && _player && _portal) {
        float distance = _player->getPosition3D().distance(_portal->getPosition3D());

        if (distance < TELEPORT_DISTANCE) {
            _isLevelSwitched = true;

            // A.移除旧的模型
            if (_temple) _temple->removeFromParent();
            if (_portal) _portal->removeFromParent();
            if (_haloEffect) _haloEffect->removeFromParent();

            // ==========================================
            // 切换天空盒逻辑 
            // ==========================================

            // 1. 删除旧的天空盒
            if (_skybox) {
                _skybox->removeFromParent();
                _skybox = nullptr; // 安全置空
            }

            // 2. 创建新的天空盒 
            _skybox = Skybox::create(
                "background/background/picture/lava.png", "background/background/picture/lava.png",
                "background/background/picture/lava.png", "background/background/picture/lava.png",
                "background/background/picture/lava.png", "background/background/picture/lava.png"
            );

            // 3. 设置新天空盒属性
            if (_skybox) {
                _skybox->setCameraMask((unsigned short)CameraFlag::USER1);
                _skybox->setScale(1.0f);
                this->addChild(_skybox, -100);
            }

            // ==========================================

            // B.加载新的模型 (Colliseum)
            _newModel = Sprite3D::create("background/background/3d/colliseum.c3b");
            if (_newModel) {
                _newModel->setScale(10.0f);
                _newModel->setPosition3D(Vec3(1400, 700, -2300));
                _newModel->setRotation3D(Vec3(90, 90, 0));
                _newModel->setCameraMask((unsigned short)CameraFlag::USER1);
                _newModel->setColor(Color3B(80, 60, 40));
                this->addChild(_newModel);
            }


            // E.重置摄像机
            _camera->setPosition3D(Vec3(0, -100, -215));

            // F.播放音效 
            SimpleAudioEngine::getInstance()->playEffect("background/background/music/teleport.wav");

            // G.切换背景音乐
            auto audio = SimpleAudioEngine::getInstance();
            audio->stopBackgroundMusic();
            audio->playBackgroundMusic("background/background/music/bgm2.mp3", true);

            // 召唤Boss
            spawnBoss();
        }
    }
}

/**
 * 生成Boss
 * 配置：位置在玩家侧方->设置目标为玩家->设置缩放与可见性
 */
void HelloWorld::spawnBoss() {
    if (_boss != nullptr) return; // 避免重复生成

    _boss = Boss::createBoss("Mutant/Mutant.c3b");
    if (_boss) {
        _boss->setPosition3D(TEMPLE_DESTINATION + Vec3(300, 0, 0)); // 玩家侧方300单位
        _boss->setTarget(_player);
        _boss->setScale(1.0f);
        _boss->setCameraMask((unsigned short)CameraFlag::USER1);
        this->addChild(_boss);
    }
}

//------------------------------
// 敌人初始化相关实现
//------------------------------

/**
 * 初始化普通敌人
 * 生成：地精->骑士->牛头人，并加入敌人容器
 */
void HelloWorld::setupEnemies() {
    _enemies.clear();

    // 地精敌人
    auto goblin = EnemyGoblin::create();
    goblin->setPosition3D(Vec3(200, 0, -200));
    goblin->setScale(1.5f);
    goblin->setTarget(_player);
    goblin->setCameraMask((unsigned short)CameraFlag::USER1);
    this->addChild(goblin);
    _enemies.push_back(goblin);

    // 骑士敌人
    auto knight = EnemyKnight::create();
    knight->setPosition3D(Vec3(-200, 0, -1500));
    knight->setTarget(_player);
    knight->setScale(3.8f);
    knight->setCameraMask((unsigned short)CameraFlag::USER1);
    this->addChild(knight);
    _enemies.push_back(knight);

    // 牛头人敌人
    auto minotaur = EnemyMinotaur::create();
    minotaur->setPosition3D(Vec3(0, 0, -750));
    minotaur->setTarget(_player);
    minotaur->setCameraMask((unsigned short)CameraFlag::USER1);
    this->addChild(minotaur);
    _enemies.push_back(minotaur);
}

//------------------------------
// 游戏结束相关实现
//------------------------------

/**
 * 显示游戏结束界面
 * @param isVictory 是否胜利（决定显示文字与颜色）
 */
void HelloWorld::showEndGameUI(bool isVictory) {
    auto visibleSize = Director::getInstance()->getWinSize();

    // 半透明遮罩层
    _endGameUI = LayerColor::create(Color4B(0, 0, 0, 180));
    this->addChild(_endGameUI, 100);

    // 结果文字（胜利：黄色；失败：红色）
    std::string resultStr = isVictory ? "MISSION COMPLETE" : "GAME OVER";
    auto label = Label::createWithSystemFont(resultStr, "Arial", 48);
    label->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 + 50));
    label->setColor(isVictory ? Color3B::YELLOW : Color3B::RED);
    _endGameUI->addChild(label);

    // 重启按钮
    auto restartItem = MenuItemFont::create("RESTART", CC_CALLBACK_1(HelloWorld::restartGame, this));
    restartItem->setFontSizeObj(32);
    restartItem->setPosition(Vec2(visibleSize.width / 2, visibleSize.height / 2 - 50));

    auto menu = Menu::create(restartItem, nullptr);
    menu->setPosition(Vec2::ZERO);
    _endGameUI->addChild(menu);
}

/**
 * 重启游戏：切换到新场景实例
 */
void HelloWorld::restartGame(cocos2d::Ref* pSender) {
    auto scene = HelloWorld::createScene();
    Director::getInstance()->replaceScene(TransitionFade::create(1.0f, scene));
}

//------------------------------
// 暂停系统实现
//------------------------------

/**
 * 创建暂停界面
 * 包含：半透明遮罩、标题、玩家状态显示、功能按钮
 */
void HelloWorld::createPauseLayer() {
    auto visibleSize = Director::getInstance()->getWinSize();

    // 1. 创建遮罩层
    _pauseLayer = LayerColor::create(Color4B(0, 0, 0, 150));
    this->addChild(_pauseLayer, 200); // 确保层级最高

    // 2. 标题
    auto title = Label::createWithSystemFont("PAUSE", "Arial", 40);
    title->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.8f));
    _pauseLayer->addChild(title);

    // 3. 人物状态观察区 (HP/MP)
    std::string statusStr = StringUtils::format("Health: %d / 100\nMana: %d / 100",
        _player->getHP(), _player->getMP());
    _statusLabel = Label::createWithSystemFont(statusStr, "Consolas", 24);
    _statusLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.6f));
    _pauseLayer->addChild(_statusLabel);

    // 4. 按钮菜单
    auto resumeItem = MenuItemFont::create("RESUME", CC_CALLBACK_1(HelloWorld::resumeGame, this));
    auto quitItem = MenuItemFont::create("QUIT", CC_CALLBACK_1(HelloWorld::quitGame, this));

    resumeItem->setFontSizeObj(32);
    quitItem->setFontSizeObj(32);

    auto menu = Menu::create(resumeItem, quitItem, nullptr);
    menu->alignItemsVerticallyWithPadding(30);
    menu->setPosition(Vec2(visibleSize.width / 2, visibleSize.height * 0.3f));
    _pauseLayer->addChild(menu);

    // 暂停物理和逻辑
    Director::getInstance()->pause();
    _isGamePaused = true;
}

/** 继续游戏回调 */
void HelloWorld::resumeGame(Ref* pSender) {
    _pauseLayer->removeFromParent();
    _pauseLayer = nullptr;
    Director::getInstance()->resume();
    _isGamePaused = false;
}

/** 退出游戏回调 */
void HelloWorld::quitGame(Ref* pSender) {
    Director::getInstance()->end();
}

/** 切换暂停状态（暂停/继续） */
void HelloWorld::togglePause() {
    if (_isGameOver) return; // 游戏结束时不响应

    if (_isGamePaused) {
        resumeGame(nullptr);
    }
    else {
        createPauseLayer();
    }
}

/** 更新暂停界面中的玩家状态 */
void HelloWorld::updateStatusInPause() {
    // 1. 安全检查，确保玩家和标签对象都存在
    if (!_player || !_statusLabel) {
        return;
    }

    // 2. 获取玩家当前的 HP 和 MP
    int currentHp = _player->getHP();
    int maxHp = 180; // 假设最大值是 100
    int currentMp = _player->getMP();
    int maxMp = 100;

    // 3. 格式化状态字符串
    std::string statusStr = StringUtils::format(
        "--- PLAYER STATUS ---\n\n"
        "HEALTH : %d / %d\n"
        "MANA   : %d / %d\n\n"
        "STATE  : %s",
        currentHp, maxHp,
        currentMp, maxMp,
        "PAUSED"
    );

    // 4. 更新 UI 文本显示
    _statusLabel->setString(statusStr);
}

//------------------------------
// HUD 系统实现
//------------------------------

/** 初始化游戏HUD元素 */
void HelloWorld::setupGameUI() {
    auto visibleSize = Director::getInstance()->getWinSize();

    // 1. 初始化玩家 HUD 节点
    _playerHUD = DrawNode::create();
    this->addChild(_playerHUD, 100);

    // 2. 初始化 Boss UI 容器
    _bossUIContainer = Node::create();
    _bossUIContainer->setVisible(false);
    this->addChild(_bossUIContainer, 100);

    _bossHUD = DrawNode::create();
    _bossUIContainer->addChild(_bossHUD);

    _bossNameLabel = Label::createWithSystemFont("BOSS", "Arial", 26);
    _bossNameLabel->setPosition(Vec2(visibleSize.width / 2, visibleSize.height + 40));
    _bossUIContainer->addChild(_bossNameLabel);

    _recoverUI = DrawNode::create();
    this->addChild(_recoverUI, 100);
}

/** 更新游戏UI（每帧调用） */
void HelloWorld::updateUI(float dt) {
    if (!_player) return;
    auto visibleSize = Director::getInstance()->getWinSize();

    // --- 玩家 UI 更新 ---
    _playerHUD->clear(); // 清除上一帧的图形

    // A. 血条 (HP)
    float hpPercent = (float)_player->getHP() / 180.0f; // 假设满血 180
    // 底色 (深灰色背景)
    _playerHUD->drawSolidRect(Vec2(20, visibleSize.height - 40), Vec2(220, visibleSize.height - 20), Color4F(0, 0, 0, 0.5f));
    // 红色条 (根据百分比计算右侧坐标)
    _playerHUD->drawSolidRect(Vec2(20, visibleSize.height - 40), Vec2(20 + 200 * hpPercent, visibleSize.height - 20), Color4F::RED);

    // B. 法条 (MP)
    float mpPercent = (float)_player->getMP() / 100.0f; // 假设满蓝 100
    // 底色
    _playerHUD->drawSolidRect(Vec2(20, visibleSize.height - 55), Vec2(170, visibleSize.height - 45), Color4F(0, 0, 0, 0.5f));
    // 蓝色条
    _playerHUD->drawSolidRect(Vec2(20, visibleSize.height - 55), Vec2(20 + 150 * mpPercent, visibleSize.height - 45), Color4F::BLUE);

    updateRecoverUI();

    // --- Boss UI 更新逻辑 ---
    if (_isLevelSwitched && _boss && !_boss->IsDead()) {
        _bossUIContainer->setVisible(true);
        _bossHUD->clear(); // 每一帧重绘前必须清除旧的矩形

        auto visibleSize = Director::getInstance()->getWinSize();

        // 1. 计算血量百分比
        float currentHP = (float)_boss->getCurrentBlood();
        float maxHP = (float)_boss->getMaxBlood();
        float bossPercent = currentHP / maxHP;

        // 2. 设置位置参数 (屏幕正下方)
        float marginBottom = 60.0f; // 距离底部高度
        float barHalfWidth = 300.0f; // 血条半宽
        float barHeight = 25.0f;    // 血条厚度
        Vec2 centerPos = Vec2(visibleSize.width / 2, marginBottom);

        // 3. 绘制底色背景矩形
        _bossHUD->drawSolidRect(
            Vec2(centerPos.x - barHalfWidth, centerPos.y),
            Vec2(centerPos.x + barHalfWidth, centerPos.y + barHeight),
            Color4F(0, 0, 0, 0.7f)
        );

        // 4. Boss 阶段判定逻辑
        Color4F bossColor = Color4F::RED; // 默认红色
        if (bossPercent < 0.5f) {
            // 进入狂暴状态：颜色变紫红
            bossColor = Color4F(0.8f, 0.0f, 0.8f, 1.0f);
            _bossNameLabel->setString("BOSS - MAW (RAGE MODE)");
            _bossNameLabel->setColor(Color3B::RED);

            // 狂暴震动效果：让 UI 容器产生轻微随机位移
            _bossUIContainer->setPosition(Vec2(rand() % 3 - 1, rand() % 3 - 1));
        }
        else {
            // 正常状态
            _bossNameLabel->setString("BOSS - MUTANT");
            _bossNameLabel->setColor(Color3B::WHITE);
            _bossUIContainer->setPosition(Vec2::ZERO); // 恢复原位
        }

        // 5. 绘制实际血量条 (根据 bossPercent 缩放宽度)
        _bossHUD->drawSolidRect(
            Vec2(centerPos.x - barHalfWidth, centerPos.y),
            Vec2(centerPos.x - barHalfWidth + (barHalfWidth * 2 * bossPercent), centerPos.y + barHeight),
            bossColor
        );

        // 6. 同步名字标签位置 (放在血条上方)
        _bossNameLabel->setPosition(Vec2(centerPos.x, centerPos.y + barHeight + 20));

    }
    else {
        // Boss 没出现或已死，隐藏 UI 并重置位置
        _bossUIContainer->setVisible(false);
    }
}

/** 更新恢复道具UI显示 */
void HelloWorld::updateRecoverUI() {
    if (!_player || !_recoverUI) return;

    _recoverUI->clear();
    int count = _player->getRecoverCount();
    Vec2 startPos = Vec2(50, 50); // 左下角起始位置
    float radius = 10.0f;
    float spacing = 25.0f;

    for (int i = 0; i < 5; ++i) {
        // 金色表示可用，灰色表示已使用
        Color4F color = (i < count) ? Color4F(1.0f, 0.84f, 0.0f, 1.0f) : Color4F(0.5f, 0.5f, 0.5f, 0.5f);
        _recoverUI->drawSolidCircle(startPos + Vec2(i * spacing, 0), radius, 0, 32, color);
    }
}