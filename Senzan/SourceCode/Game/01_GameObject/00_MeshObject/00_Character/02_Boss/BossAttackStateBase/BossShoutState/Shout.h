#pragma once
#include "..//BossAttackStateBase.h"
#include "Game/03_Collision/00_Core/01_Capsule/CapsuleCollider.h"

/***************************************************************************
*	叫び攻撃
*	叫び攻撃はノックバックを設定して当たり判定で押し出す仕組みにする
*	ダメージはなし.
**/

class BossIdolState;

class Shout final
	: public BossAttackStateBase
{
public:
	enum class enShout : byte
	{
		None,
		Shout,
		ShoutTime,
		ShoutToIdol,
	};
public:
	//コンストラクタ
	Shout(Boss* owner);
	//デストラクタ.
	~Shout() override;

	//最初に入る.
	void Enter() override;
	//動作.
	void Update() override;
	//Update()の後に入る.
	void LateUpdate() override;
	//描画.
	void Draw() override;
	//終わるときに一回だけ入る.
	void Exit() override;

	//PlayerのParry成功時硬直させたいアニメーションとタイミング.
	std::pair<Boss::enBossAnim, float> GetParryAnimPair() override;
private:
	void DrawImGui() override;
	void LoadSettings() override;
	void SaveSettings() const override;
	std::filesystem::path GetSettingsFileName() const override { return std::filesystem::path("BossShoutState.json"); }
private:
	//叫びの状態を管理する列挙型.
	enShout m_List;
	//ダメージ量.
	float m_ShoutDamage;
	//範囲半径.
	float m_ShoutRadius;
	//ノックバックの力.
	float m_KnockBackPower;
	//叫び攻撃判定が広がる時間(秒).
	//ステート中に半径が 0->m_ShoutRadiusへ放線補間される.
	float m_ShoutExpandTime;
	//ランタイム用:経過時間と開始半径.
	float m_ShoutElapsed;
	float m_ShoutStartRadius;

	//エフェクト関連:一度だけ発火させるためのフラグと設定.
	bool m_EffectPlayed;
	DirectX::XMFLOAT3 m_EffectOffset;
	float m_EffectScale;
};

