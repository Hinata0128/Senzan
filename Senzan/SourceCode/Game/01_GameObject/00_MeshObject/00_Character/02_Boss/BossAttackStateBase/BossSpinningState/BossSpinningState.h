#pragma once

#include "..//BossAttackStateBase.h"
#include "System/Utility/Math/Easing/Easing.h"

class BossSpinningState
    : public BossAttackStateBase
{
public:
    enum class enSpinning : byte
    {
        None,
        Anim,
        Attack,
        CoolDown,
        Trans
    };

public:
    BossSpinningState(Boss* owner);
    ~BossSpinningState() override;

    void Enter() override;
    void Update() override;
    void LateUpdate() override;
    void Draw() override;
    void Exit() override;

    // PlayerのParry成功時硬直させたいアニメーションとタイミング.
    std::pair<Boss::enBossAnim, float> GetParryAnimPair() override;

    // ImGui と設定の読み書き
    void DrawImGui() override;
    void LoadSettings() override;
    void SaveSettings() const override;
    std::filesystem::path GetSettingsFileName() const override { return std::filesystem::path("BossSpinningState.json"); }

private:
    enSpinning m_List;
    bool m_IsSpun;

    // 回転量制御
    float m_RotateTotalDeg = 360.0f; // 攻撃フェーズで回す合計角度
    MyEasing::Type m_EasingType = MyEasing::Type::InOutSine; // デフォルトのイージング
    float m_LastEasedAngle = 0.0f; // ランタイム: 前フレームのイーズ角度
    float m_SecondSlashTimer;
    float m_SecondSlashTiming;
    bool m_SecondSlashed;
    // アニメは Boss::enBossAnim の列挙を直接使用する
};
