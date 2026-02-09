#pragma once
#include "../Combat.h"

class Player;

/**************************************************
*	プレイヤーのパリィのステート(派生).
*	担当:淵脇 未来.
**/

namespace PlayerState {
    class Parry final : public Combat
    {
    public:
        Parry(Player* owner);
        ~Parry();

        // IDの取得.
        constexpr PlayerState::eID GetStateID() const override;

        void Enter() override;
        void Update() override;
        void LateUpdate() override;
        void Draw() override;
        void Exit() override;

    private:
        // パリィ成功を待つための一時停止制御
        float m_ElapsedTime = 0.0f;
        float m_PauseThreshold = 1.12f; // この時間経過後にアニメ速度を0にする
        // パリィ判定が成功しなかった場合にステートを抜けるまでの最大待機時間
        float m_MaxWaitTime = 1.5f; // 秒
        bool m_IsPaused = false;
        bool m_IsAnimEndStart = false;
        bool m_IsFastTime = false;
        // 強制位置移動の設定
        bool m_UseForceOffset = false;
        DirectX::XMFLOAT3 m_ForceOffset = { 0.0f, 0.0f, 0.0f };

    public:
        // パリィ開始時にターゲット位置からのオフセットでプレイヤー位置を強制設定する機能
        void EnableForceOffset(bool v) { m_UseForceOffset = v; }
        void SetForceOffset(const DirectX::XMFLOAT3& off) { m_ForceOffset = off; }
    };
}
