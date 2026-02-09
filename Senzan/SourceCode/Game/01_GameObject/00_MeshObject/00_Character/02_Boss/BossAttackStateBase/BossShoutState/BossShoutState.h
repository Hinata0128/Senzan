#pragma once
#include "..//BossAttackStateBase.h"
#include "Game\03_Collision\00_Core\01_Capsule\CapsuleCollider.h"

/************************************************************************************
*	叫び攻撃：叫びの攻撃は当たり判定でノックバックさせるのでここでは時間でのIdolの遷移を書く.
**/


class BossIdolState;

class BossShoutState
	: public BossAttackStateBase
{
public:
	enum class enShout : byte
	{
		none,
		Shout,
		ShoutTime,
		ShoutToIdol,
	};
public:
	BossShoutState(Boss* owner);
	~BossShoutState() override;

	void Enter() override;
	void Update() override;
	void LateUpdate() override;
	void Draw() override;
	void Exit() override;

    // PlayerのParry成功時硬直させたいアニメーションとタイミング.
    std::pair<Boss::enBossAnim, float> GetParryAnimPair() override;

private:
    void DrawImGui() override;
    void LoadSettings() override;
    void SaveSettings() const override;
    std::filesystem::path GetSettingsFileName() const override { return std::filesystem::path("BossShoutState.json"); }
private:
	enShout m_List;
	float m_ShoutDamage = 10.0f;     // ダメージ量.
	float m_ShoutRadius = 180.0f;     // 範囲半径.
	float m_KnockBackPower = 15.0f;  // ノックバック力.
    // 叫び判定が広がる時間（秒）。ステート中に半径が 0 -> m_ShoutRadius へ線形補間される
    float m_ShoutExpandTime = 1.0f;
    // ランタイム用: 経過時間と開始半径
    float m_ShoutElapsed = 0.0f;
    float m_ShoutStartRadius = 0.0f;

    // エフェクト関連: 一度だけ発火させるためのフラグと設定
    bool m_EffectPlayed = false;
    DirectX::XMFLOAT3 m_EffectOffset = DirectX::XMFLOAT3(0.0f, 15.0f, 0.0f);
    float m_EffectScale = 10.0f;
};
