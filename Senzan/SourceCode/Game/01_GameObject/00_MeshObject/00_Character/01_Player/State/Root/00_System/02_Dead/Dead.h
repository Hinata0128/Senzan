#pragma once
#include "../System.h"

class Player;

/**************************************************
*	プレイヤーの死亡のステート(派生).
*	担当:淵脇 未来.
**/

namespace PlayerState {
    class Dead final : public System
    {
    public:
        Dead(Player* owner);
        ~Dead();

        // IDの取得.
        constexpr PlayerState::eID GetStateID() const override;

        void Enter() override;
        void Update() override;
        void LateUpdate() override;
        void Draw() override;
        void Exit() override;

    private:
    };
}