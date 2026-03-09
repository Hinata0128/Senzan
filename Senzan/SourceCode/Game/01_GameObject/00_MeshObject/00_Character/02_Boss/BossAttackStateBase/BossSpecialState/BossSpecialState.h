#pragma once
#include "..//BossAttackStateBase.h"

class BossIdolState;

class BossSpecialState final : public BossAttackStateBase
{
public:
    enum class enSpecial : byte {
        None,       // 溜め・準備
        Attack,     // 突進・追尾フェーズ
        CoolTime,   // 着地硬直
        Trans       // アイドルへ遷移
    };

public:
    BossSpecialState(Boss* owner);
    ~BossSpecialState() override;

    void Enter() override;
    void Update() override;
    void LateUpdate() override {}
    void Draw() override {}
    void Exit() override;

    std::pair<Boss::enBossAnim, float> GetParryAnimPair() override;

    void DrawImGui() override;
    void LoadSettings() override;
    void SaveSettings() const override;
    std::filesystem::path GetSettingsFileName() const override { return std::filesystem::path("BossSpecialState.json"); }

private:
    void BossAttack(); // 突進移動と追尾、垂直制御

private:
    enSpecial m_List;

    // --- 構造ベース(8割)の変数 ---
    bool  m_IsMoving = false;
    float m_MoveTimer = 0.0f;
    float m_MoveDuration = 1.0f;
    float m_Distance = 0.0f;
    DirectX::XMFLOAT3 m_MoveVec;

    bool  m_UseVerticalEasing = true;
    float m_AscentHeight = 10.0f;
    float m_AscentDuration = 0.3f;
    float m_DescentDuration = 0.6f;
    float m_VerticalTimer = 0.0f;
    float m_StartY = 0.0f;

    float m_WaitSeconds = 0.5f;     // 溜め時間
    float m_SlowAnimSpeed = 0.5f;
    bool  m_AnimSlowed = false;
    float m_SlowDuration = 1.0f;
    float m_SlowElapsed = 0.0f;

    // --- 以前のロジック(2割)の変数 ---
    float m_MaxTrackingAngle = 90.0f; // 突進中の旋回性能
    DirectX::XMFLOAT3 m_CurrentDirection;
};