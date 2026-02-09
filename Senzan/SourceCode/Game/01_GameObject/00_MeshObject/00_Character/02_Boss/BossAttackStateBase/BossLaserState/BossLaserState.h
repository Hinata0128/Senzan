#pragma once

#include "..//BossAttackStateBase.h"

class BossIdolState;
class Boss;

class BossLaserState final : public BossAttackStateBase
{
public:
    enum class enLaser : unsigned char
    {
        None,
        Charge,
        Fire,
        Cool,
        Trans
    };

public:
    BossLaserState(Boss* owner);
    ~BossLaserState() override;

    void Enter() override;
    void Update() override;
    void LateUpdate() override;
    void Draw() override;
    void Exit() override;

    std::pair<Boss::enBossAnim, float> GetParryAnimPair() override;

    void DrawImGui() override;
    void LoadSettings() override;
    void SaveSettings() const override;
    std::filesystem::path GetSettingsFileName() const override { return std::filesystem::path("BossLaserState.json"); }

private:
    enLaser m_State;

    // charge timing
    float m_ChargeDuration;
    float m_ChargeElapsed;

    // fire timing
    float m_FireDuration;
    float m_FireElapsed;

    // laser parameters
    float m_LaserDamage;
    float m_LaserRadius;
    float m_LaserRange;

    // effect played flag
    bool m_EffectPlayed;

    std::shared_ptr<BossIdolState> m_pIdol;
};
