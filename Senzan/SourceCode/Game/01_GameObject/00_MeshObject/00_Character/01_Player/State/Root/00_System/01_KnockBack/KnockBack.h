#pragma once
#include "../System.h"

class Player;

/**************************************************
*	プレイヤーのノックバックのステート(派生).
*	担当:淵脇 未来.
**/

namespace PlayerState {
    class KnockBack final : public System
    {
    public:
        KnockBack(Player* owner);
        ~KnockBack();

        // IDの取得.
        constexpr PlayerState::eID GetStateID() const override;

        void Enter() override;
        void Update() override;
        void LateUpdate() override;
        void Draw() override;
        void Exit() override;

    private:
        DirectX::XMFLOAT3 m_AppliedMovement = {};
    };
}