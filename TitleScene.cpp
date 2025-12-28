#include "TitleScene.h"
#include "HelloWorldScene.h" //引入游戏场景的头文件
#include "SimpleAudioEngine.h"

USING_NS_CC;
using namespace CocosDenshion;

Scene* TitleScene::createScene()
{
    return TitleScene::create();
}

bool TitleScene::init()
{
    if (!Scene::init()) return false;

    auto size = Director::getInstance()->getVisibleSize();

    // ==========================================
    // 1. 设置背景 
    // ==========================================
    // 如果你有封面图，可以用 Sprite::create("cover.png");
    // 这里先用纯色背景代替
    auto bgLayer = LayerColor::create(Color4B(15, 15, 30, 255)); // 深蓝黑
    this->addChild(bgLayer);

    // ==========================================
    // 2. 游戏标题 (MYTH: WUKONG)
    // ==========================================
    auto titleLabel = Label::createWithSystemFont("HOLY: MARIA", "Arial", 80);
    titleLabel->setPosition(Vec2(size.width / 2, size.height * 0.7));
    titleLabel->enableShadow(Color4B::BLACK, Size(2, -2), 2); // 加点阴影
    titleLabel->setTextColor(Color4B(255, 200, 50, 255));     // 金色字体
    this->addChild(titleLabel);

    // ==========================================
    // 3. 创建菜单按钮
    // ==========================================

    // 按钮 A: 开始游戏
    auto startLabel = Label::createWithSystemFont("START GAME", "Arial", 40);
    auto startItem = MenuItemLabel::create(startLabel, CC_CALLBACK_1(TitleScene::menuStartCallback, this));

    // 按钮 B: 退出
    auto exitLabel = Label::createWithSystemFont("EXIT", "Arial", 40);
    auto exitItem = MenuItemLabel::create(exitLabel, CC_CALLBACK_1(TitleScene::menuExitCallback, this));

    // 放入菜单容器
    auto menu = Menu::create(startItem, exitItem, NULL);
    menu->alignItemsVerticallyWithPadding(50); // 垂直排列，间距50
    menu->setPosition(Vec2(size.width / 2, size.height * 0.4));
    this->addChild(menu);

    // ==========================================
    // 4.播放标题音乐
    // ==========================================
    //SimpleAudioEngine::getInstance()->playBackgroundMusic("background/music/bgm1.mp3", true);

    return true;
}

void TitleScene::menuStartCallback(Ref* pSender)
{
    CCLOG("点击开始，切换场景...");

    // 1. 创建游戏场景
    auto gameScene = HelloWorld::createScene();

    // 2. 创建切换特效 (淡出淡入，时长 1.0秒)
    // TransitionFade 是最经典的转场效果，黑色渐变
    auto transition = TransitionFade::create(1.0f, gameScene, Color3B::BLACK);

    // 3. 执行切换
    Director::getInstance()->replaceScene(transition);
}

void TitleScene::menuExitCallback(Ref* pSender)
{
    // 关闭游戏
    Director::getInstance()->end();
}