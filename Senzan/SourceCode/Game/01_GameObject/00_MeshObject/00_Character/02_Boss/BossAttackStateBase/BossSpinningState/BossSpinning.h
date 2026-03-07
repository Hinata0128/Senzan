#pragma once

#include "..//BossAttackStateBase.h"
#include "System/Utility/Math/Easing/Easing.h"

/****************************************************************
*	回転斬りクラス.
	このクラスは回転斬りのアニメーションの再生をしてjsonでダメージ当たり判定当絵を保存している
	このクラスの制御しているjsonの名前 BossSpinningです.
	作成日: 2026/03/07
	担当者: 西村 日向.
**/

//前方宣言.
class BossIdolState;

class BossSpinning final
	: public BossAttackStateBase
{
public:
	//回転斬りの列挙型を作成.
	//状態によっての動作をswitch文で作成させる.
	//メモリの節約のために1バイト0~255に制限している.
	enum class enSpinning : byte
	{
		None,		//何もしない.
		Anim,		//アニメーション.
		Attack,		//攻撃.
		CoolDown,	//攻撃終了(当たり判定の有効化を停止等(false)).
		Trans		//遷移(Idolへ).
	};
public:
	BossSpinning(Boss* owner);
	~BossSpinning() override;

	void Enter() override;
	void Update() override;
	void LateUpdate() override;
	void Draw() override;
	void Exit() override;

	// PlayerのParry成功時硬直させたいアニメーションとタイミング.
	// パリィ成功時の設定.
	std::pair<Boss::enBossAnim, float> GetParryAnimPair() override;
private:
	// デバッグ中にImGuiでステータスを変更できるように設定する.
	void DrawImGui() override;
	// パラメータ用のjsonファイルの読み込み.
	void LoadSettings() override;
	// パラメータ用のjsonファイルの保存.
	void SaveSettings() const override;
	// 変更させるjsonファイルのパスを設定.
	std::filesystem::path GetSettingsFileName() const override { return std::filesystem::path("BossSpinning.json"); }
private:
	enSpinning m_List;
	bool m_IsSpun;
	//回転量制御.
	//攻撃フェーズで回す合計角度.
	//一周させるのでここは360.0fと書く.
	float m_RotateToTalDeg;
	//デフォルトのイージング.
	MyEasing::Type m_EasingTyep;
	//前フレームのイーズ角度.
	float m_LastEasedAndle;
	float m_SecondSlashTimer;
	float m_ScondSlashTiming;
	bool m_ScondSlashed;
};