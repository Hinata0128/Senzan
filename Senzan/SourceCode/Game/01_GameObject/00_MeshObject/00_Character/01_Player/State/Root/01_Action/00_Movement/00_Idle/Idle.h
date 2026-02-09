#pragma once
#include "../Movement.h"

class Player;

/**************************************************
*	プレイヤーの待機のステート(派生).
*	担当:淵脇 未来.
**/

namespace PlayerState {
    class Idle final 
        : public Movement
    {
    public:
        Idle(Player* owner);
        ~Idle();

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