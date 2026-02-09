#pragma once
#include "..//BossAttackStateBase.h"

#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"

#include "00_MeshObject/00_Character/03_SnowBall/SnowBall.h"


class BossThrowingState
    : public BossAttackStateBase
{
public:
    enum class enThrowing : byte
    {
        None,
        Anim,
        Attack,
        CoolDown,
        Trans
    };

public:
    BossThrowingState(Boss* owner);
    ~BossThrowingState() override;

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
    std::filesystem::path GetSettingsFileName() const override { return std::filesystem::path("BossThrowingState.json"); }
private:
    void BossAttack();
private:
    enThrowing m_List;
    bool m_IsLaunched;
    float m_BallHeight;   // 発射高さオフセット
    float m_BallSpeed;    // 弾速（SnowBallへ渡す）
};

