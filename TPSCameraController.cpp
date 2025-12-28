#include "TPSCameraController.h"

USING_NS_CC;

TPSCameraController::TPSCameraController() {}

TPSCameraController* TPSCameraController::create(Camera* camera, Node* target) {
    auto controller = new (std::nothrow) TPSCameraController();
    if (controller && controller->init(camera, target)) {
        controller->autorelease();
        return controller;
    }
    CC_SAFE_DELETE(controller);
    return nullptr;
}

bool TPSCameraController::init(Camera* camera, Node* target) {
    if (!camera || !target) return false;
    _camera = camera;
    _target = target;
    _currentCameraPos = _camera->getPosition3D();
    return true;
}

void TPSCameraController::handleMouseMove(float deltaX, float deltaY) {
    // 水平旋转 (Yaw)
    _yaw -= deltaX * _sensitivity;

    // 上下旋转 (Pitch)
    
    _pitch -= deltaY * _sensitivity; // 累加鼠标纵向偏移量

    // 限制角度：通常限制在 -20 到 80 度之间，防止相机穿地或翻转
    _pitch = std::max(-10.0f, std::min(80.0f, _pitch));
}

void TPSCameraController::update(float dt) {
    if (!_camera || !_target) return;

    Vec3 targetPos = _target->getPosition3D();

    // 将角度转为弧度
    float radiansYaw = CC_DEGREES_TO_RADIANS(_yaw + 180.0f);
    float radiansPitch = CC_DEGREES_TO_RADIANS(_pitch);

    // 经典的球面坐标转笛卡尔坐标计算偏移
    Vec3 offset(
        sinf(radiansYaw) * cosf(radiansPitch),
        sinf(radiansPitch),
        cosf(radiansYaw) * cosf(radiansPitch)
    );
    offset.normalize();
    offset *= _distance;

    // 计算理想位置并进行平滑插值 (Lerp)
    Vec3 desiredPos = targetPos + offset;

    // 优化：计算当前相机与理想位置的距离
    float dist = _currentCameraPos.distance(desiredPos);

    if (dist > 100.0f) {
        // 如果距离过远（比如传送或超高速移动），直接同步位置，防止拉扯
        _currentCameraPos = desiredPos;
    }
    else {
        // 正常的平滑插值
        _currentCameraPos = _currentCameraPos.lerp(desiredPos, 1.0f - _lag);
    }

    _camera->setPosition3D(_currentCameraPos);

    // 优化：LookAt 的目标也进行平滑，防止镜头瞬间抖动
    Vec3 lookAtTarget = targetPos + Vec3(0, 10, 0);
    _camera->lookAt(lookAtTarget, Vec3(0, 1, 0));
}