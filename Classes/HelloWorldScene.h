#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "Paladin/Paladin.h" 

class HelloWorld : public Scene
{
public:
    static Scene* createScene();
    virtual bool init();
    CREATE_FUNC(HelloWorld);

    // 摄像机相关常量（关键！）
    static const float CAMERA_DISTANCE; // 摄像机与角色的水平距离
    static const float CAMERA_HEIGHT;   // 摄像机的垂直高度
    static const float CAMERA_LAG;      // 摄像机滞后系数（平滑跟随）

private:
    void update(float dt) override;
    void updateCamera(float dt);

    Camera* _camera = nullptr;          // 摄像机对象
    Paladin* _paladin = nullptr;        // 主角对象
    Vec3 _cameraCurrentPos;             // 摄像机当前位置（用于平滑过渡）
    std::unordered_map<EventKeyboard::KeyCode, bool> _keys; // 键盘状态
};


#endif // __HELLOWORLD_SCENE_H__