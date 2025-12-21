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
    // 更新 Yaw 角（水平旋转）
    _yaw -= deltaX * _sensitivity;
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
    _currentCameraPos = _currentCameraPos.lerp(desiredPos, 1.0f - _lag);

    // 应用位置和朝向
    _camera->setPosition3D(_currentCameraPos);
    _camera->lookAt(targetPos + Vec3(0, 10, 0), Vec3(0, 1, 0));
}