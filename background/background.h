#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "SimpleAudioEngine.h" // 引入音频引擎
using namespace CocosDenshion; // 使用音频命名空间

class HelloWorld : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();

    // 声明 update 函数
    virtual void update(float dt);

    void menuCloseCallback(cocos2d::Ref* pSender);

    CREATE_FUNC(HelloWorld);

private:
    // 摄像机
    cocos2d::Camera* _camera;

    // 按键状态
    bool _isWPressed = false;
    bool _isSPressed = false;
    bool _isAPressed = false;
    bool _isDPressed = false;

    // 鼠标控制相关
    float _rotationX = 0.0f; // 上下角度
    float _rotationY = 0.0f; // 左右角度

    cocos2d::Vec2 _lastMousePos;

    bool _isFirstMouse = true;

    bool _isDragging = false;



};

#endif // __HELLOWORLD_SCENE_H__