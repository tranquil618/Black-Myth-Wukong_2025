#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "Player/Maria.h" 
#include "Enemy/EnemyGoblin.h"
#include "Enemy/EnemyMinotaur.h"

// 引入新分离的控制器
#include "TPSCameraController.h"
#include "PlayerInputController.h"

class HelloWorld : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
    virtual ~HelloWorld(); // 需要析构函数来释放控制器
    CREATE_FUNC(HelloWorld);

private:
    // 模块化设置函数 (保持每个函数职责单一)
    bool setupCamera();
    bool setupMaria();
    bool setupEnemies();
    void setupWorld();

    void update(float dt) override;

    // --- 成员变量 (仅保留业务对象和控制器) ---
    cocos2d::Camera* _camera = nullptr;
    Maria* _player = nullptr;
    EnemyMinotaur* _minotaur = nullptr;

    // 控制器句柄
    TPSCameraController* _cameraController = nullptr;
    PlayerInputController* _inputHandler = nullptr;
};

#endif