#pragma once
#include "../Dodge.h"
#include "AfterImage.h"
#include <memory>

class Player;

/**************************************************
*	プレイヤーのジャスト回避のステート(派生).
*	担当:淵脇 未来.
**/

namespace PlayerState {
class JustDodge final : public Dodge
{
public:
    JustDodge(Player* owner);
    ~JustDodge();

    // IDの取得.
    constexpr PlayerState::eID GetStateID() const override;

    void Enter() override;
    void Update() override;
    void LateUpdate() override;
    void Draw() override;
    void Exit() override;

protected:
    // 残像エフェクト.
    std::unique_ptr<AfterImage> m_pAfterImage;
    float m_AfterImageTimer;        // 残像生成タイマー.
    float m_AfterImageInterval;     // 残像生成間隔 (秒).
    float m_AfterImageLifeTime;     // 残像の生存時間 (秒).
    bool  m_IsEffectStart;          // 一度だけエフェクトを再生.
};
}
