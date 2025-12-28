#ifndef __TPS_CAMERA_CONTROLLER_H__
#define __TPS_CAMERA_CONTROLLER_H__

#include "cocos2d.h"

/**
 * 第三人称视角相机控制器
 * 负责处理相机跟随目标、鼠标控制视角等功能
 */
class TPSCameraController : public cocos2d::Ref
{
public:
    /**
     * 创建控制器实例
     * @param camera 被控制的相机
     * @param target 相机跟随的目标节点
     * @return 控制器实例，创建失败返回nullptr
     */
    static TPSCameraController* create(cocos2d::Camera* camera, cocos2d::Node* target);

    /**
     * 初始化控制器
     * @param camera 被控制的相机
     * @param target 相机跟随的目标节点
     * @return 初始化成功返回true，否则返回false
     */
    bool init(cocos2d::Camera* camera, cocos2d::Node* target);

    /**
     * 每帧更新相机位置（平滑过渡逻辑）
     * @param dt 帧间隔时间
     */
    void update(float dt);

    /**
     * 处理鼠标移动事件，更新相机旋转角度
     * @param deltaX 鼠标X轴移动增量
     * @param deltaY 鼠标Y轴移动增量
     */
    void handleMouseMove(float deltaX, float deltaY);

    // 设置属性的接口
    void setDistance(float distance) { _distance = distance; }       // 设置相机与目标的距离
    void setSensitivity(float sensitivity) { _sensitivity = sensitivity; } // 设置鼠标灵敏度
    void setLag(float lag) { _lag = lag; }               // 设置平滑过渡延迟（0-1之间）
    void setPitch(float pitch) { _pitch = pitch; }         // 设置俯仰角

    // 获取属性的接口
    float getYaw() const { return _yaw; }    // 获取偏航角（水平旋转）
    float getPitch() const { return _pitch; }  // 获取俯仰角（垂直旋转）

private:
    TPSCameraController();  // 构造函数私有化

    // 受控对象
    cocos2d::Camera* _camera = nullptr;  // 被控制的相机
    cocos2d::Node* _target = nullptr;    // 跟随目标

    // 相机状态参数
    float _yaw = 0.0f;           // 偏航角（水平旋转角度）
    float _pitch = 30.0f;        // 俯仰角（垂直旋转角度，默认30度）
    float _distance = 50.0f;     // 相机与目标的距离
    float _sensitivity = 0.2f;   // 鼠标灵敏度（旋转速度系数）
    float _lag = 0.2f;           // 平滑过渡延迟（值越小过渡越快）

    cocos2d::Vec3 _currentCameraPos;  // 相机当前位置（用于平滑过渡计算）
};

#endif