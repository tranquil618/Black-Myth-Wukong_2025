#ifndef __TPS_CAMERA_CONTROLLER_H__
#define __TPS_CAMERA_CONTROLLER_H__

#include "cocos2d.h"

class TPSCameraController : public cocos2d::Ref
{
public:
    // 静态创建方法
    static TPSCameraController* create(cocos2d::Camera* camera, cocos2d::Node* target);


    // 初始化设置
    bool init(cocos2d::Camera* camera, cocos2d::Node* target);

    // 每帧调用，处理平滑跟随逻辑
    void update(float dt);

    // 处理鼠标移动带来的旋转
    void handleMouseMove(float deltaX, float deltaY);

    // 配置参数
    void setDistance(float distance) { _distance = distance; }
    void setSensitivity(float sensitivity) { _sensitivity = sensitivity; }
    void setLag(float lag) { _lag = lag; }
    void setPitch(float pitch) { _pitch = pitch; } // 俯视角

    float getYaw() const { return _yaw; }
	float getPitch() const { return _pitch; }
private:
    TPSCameraController();

    // 弱引用
    cocos2d::Camera* _camera = nullptr;
    cocos2d::Node* _target = nullptr;

    // 状态变量
    float _yaw = 0.0f;           // 水平旋转角
    float _pitch = 30.0f;        // 俯仰角 (默认30度)
    float _distance = 50.0f;     // 距离
    float _sensitivity = 0.2f;   // 灵敏度
    float _lag = 0.2f;           // 平滑延迟 (0-1)

    cocos2d::Vec3 _currentCameraPos; // 缓存当前位置用于平滑过渡
};

#endif