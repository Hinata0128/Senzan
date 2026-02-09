#pragma once
#include "../Dodge.h" 

class Player;

/**************************************************
*	プレイヤーの回避のステート(派生).
*	担当:淵脇 未来.
**/

namespace PlayerState {
    class DodgeExecute final : public Dodge
    {
    public:
        DodgeExecute(Player* owner);
        ~DodgeExecute();

        // IDの取得.
        constexpr PlayerState::eID GetStateID() const override;

        void Enter() override;
        void Update() override;
        void LateUpdate() override;
        void Draw() override;
        void Exit() override;

    private:
        bool m_IsAnimEnd;
        float m_traveledDistance;
    };
}