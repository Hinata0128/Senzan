#pragma once
#include "../Action.h"

class Player;
class CapsuleCollider;

/**************************************************
*	プレイヤーの移動の共通処理ステート(基底).
*   このステート自体がインスタンス化されることはない.
*	担当:淵脇 未来.
**/

namespace PlayerState {
    class Movement : public Action
    {
    public:
        Movement(Player* owner);
        ~Movement();

        void Enter() override;
        void Update() override;
        void LateUpdate() override;
        void Draw() override;
        void Exit() override;

    protected:
        std::shared_ptr<CapsuleCollider> m_Collision_BodyHit;

    };
}