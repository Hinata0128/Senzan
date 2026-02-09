#pragma once
#include "../System.h"

class Player;

/**************************************************
*	プレイヤーのポーズのステート(派生).
*	担当:淵脇 未来.
**/

namespace PlayerState {
    class Pause final : public System
    {
    public:
        Pause(Player* owner);
        ~Pause();

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