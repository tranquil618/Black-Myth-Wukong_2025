#include "TPSCameraController.h"

USING_NS_CC;

TPSCameraController::TPSCameraController() {}

TPSCameraController* TPSCameraController::create(Camera* camera, Node* target)
{
    auto controller = new (std::nothrow) TPSCameraController();
    if (controller && controller->init(camera, target))
    {
        controller->autorelease();
        return controller;
    }
    CC_SAFE_DELETE(controller);
    return nullptr;
}

bool TPSCameraController::init(Camera* camera, Node* target)
{
    // 校验相机和目标是否有效
    if (!camera || !target)
        return false;

    _camera = camera;
    _target = target;
    _currentCameraPos = _camera->getPosition3D();  // 初始化相机位置

    return true;
}

void TPSCameraController::handleMouseMove(float deltaX, float deltaY)
{
    // 更新偏航角（水平旋转）：鼠标X轴移动控制
    _yaw -= deltaX * _sensitivity;

    // 更新俯仰角（垂直旋转）：鼠标Y轴移动控制
    _pitch -= deltaY * _sensitivity;

    // 限制俯仰角范围（-10度到80度），避免过度旋转导致视角异常
    _pitch = std::max(-10.0f, std::min(80.0f, _pitch));
}

void TPSCameraController::update(float dt)
{
    // 若相机或目标无效，则不执行更新
    if (!_camera || !_target)
        return;

    // 获取目标当前位置
    Vec3 targetPos = _target->getPosition3D();

    // 将角度转换为弧度（用于三角函数计算）
    float radiansYaw = CC_DEGREES_TO_RADIANS(_yaw + 180.0f);  // 偏航角偏移180度修正方向
    float radiansPitch = CC_DEGREES_TO_RADIANS(_pitch);       // 俯仰角

    // 计算相机相对于目标的偏移量（球面坐标转笛卡尔坐标）
    Vec3 offset(
        sinf(radiansYaw) * cosf(radiansPitch),  // X轴偏移
        sinf(radiansPitch),                     // Y轴偏移（高度）
        cosf(radiansYaw) * cosf(radiansPitch)   // Z轴偏移
    );
    offset.normalize();              // 标准化方向向量
    offset *= _distance;             // 按距离缩放偏移量

    // 计算相机期望位置（目标位置 + 偏移量）
    Vec3 desiredPos = targetPos + offset;

    // 计算当前位置与期望位置的距离
    float dist = _currentCameraPos.distance(desiredPos);

    // 处理相机位置更新
    if (dist > 100.0f)
    {
        // 若距离过远，直接跳到目标位置（避免异常情况下的过度延迟）
        _currentCameraPos = desiredPos;
    }
    else
    {
        // 平滑过渡到期望位置（线性插值）
        _currentCameraPos = _currentCameraPos.lerp(desiredPos, 1.0f - _lag);
    }

    // 更新相机实际位置
    _camera->setPosition3D(_currentCameraPos);

    // 让相机看向目标上方（Y轴偏移10单位，避免视角过低）
    Vec3 lookAtTarget = targetPos + Vec3(0, 10, 0);
    _camera->lookAt(lookAtTarget, Vec3(0, 1, 0));  // 以Y轴为上方向
}
