#pragma once
#include "../../PlayerStateBase.h"

class Player;
class SingleTrigger;

/**************************************************
*	プレイヤーの操作によるステート(基底).
*   このステート自体がインスタンス化されることはない.
*	担当:淵脇 未来.
**/
namespace PlayerState
{
    class Action 
        : public PlayerStateBase
    {
    public:
        Action(Player* owner);
        ~Action();

        virtual void Enter() override;
        virtual void Update() override;
        virtual void LateUpdate() override;
        virtual void Draw() override;
        virtual void Exit() override;

    protected:
        std::unique_ptr<SingleTrigger> m_AnimSpeedChangedTrigger;

    };
}
