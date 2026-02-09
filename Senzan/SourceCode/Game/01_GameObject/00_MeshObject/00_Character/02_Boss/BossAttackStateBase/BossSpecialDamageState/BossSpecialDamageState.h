#pragma once

#include "..//BossAttackStateBase.h"

class Boss;

class BossSpecialDamageState final : public BossAttackStateBase
{
public:
    enum class Phase : byte { TakeDamage, Down, DownContinue, GetUp, End };
public:
    BossSpecialDamageState(Boss* owner);
    ~BossSpecialDamageState() override;
    void Enter() override;
    void Update() override;
    void LateUpdate() override;
    void Draw() override;
    void Exit() override;
    void DrawImGui() override;
    void LoadSettings() override;
    void SaveSettings() const override;
    std::filesystem::path GetSettingsFileName() const override { return std::filesystem::path("BossSpecialDamageState.json"); }
    
    // 外部からDownフェーズに遷移させる
    void ForceTransitionToDown();
    
private:
    Phase m_Phase = Phase::TakeDamage;
    float m_TakeDamageDuration = 2.0f;
    float m_TakeDamageAnimSpeed = 1.0f;
    float m_TakeDamageReplayTime = 0.5f;
    bool m_TakeDamageReplayEnabled = true;
    float m_DownAnimSpeed = 1.0f;
    float m_DownContinueSpeed = 1.0f;
    float m_DownTransitionTime = 0.3f;
    float m_DownContinueDuration = 3.0f;
    float m_GetUpAnimSpeed = 1.0f;
    float m_GetUpTransitionTime = 0.5f;
    float m_PhaseTimer = 0.0f;
    float m_ReplayTimer = 0.0f;
};
