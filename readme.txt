1.已经能够将哥布林和Maria放在一个场景里面初步进行交互，尽管效果不那么好
2.这里面对EnemyGoblin的代码进行了一定调整，一个是EnemyGoblin.h中增加了
float _detectionRange = 150.0f; // 警戒范围，超过这个距离不理人
另一个是EnemyGoblin.cpp中增加了
//警戒范围判定
float distanceToPlayer = this->getPosition3D().distance(_target->getPosition3D());

if (distanceToPlayer > _detectionRange) {
    // 如果玩家在范围外，保持 IDLE 并返回
    if (_state != EnemyState::IDLE) {
        changeState(EnemyState::IDLE);
    }
    return;
}
作为攻击范围的判定
但是在HelloWorldScene里面的测试场景中还未体现
