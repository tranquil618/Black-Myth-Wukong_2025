#ifndef __TITLE_SCENE_H__
#define __TITLE_SCENE_H__

#include "cocos2d.h"

class TitleScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();

    // “开始游戏”按钮的回调
    void menuStartCallback(cocos2d::Ref* pSender);

    // “退出游戏”按钮的回调
    void menuExitCallback(cocos2d::Ref* pSender);

    CREATE_FUNC(TitleScene);
};

#endif // __TITLE_SCENE_H__