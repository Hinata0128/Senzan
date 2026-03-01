#pragma once
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossAttackStateBase.h"

/*******************************************************************
*	ボスのレーザー攻撃.
*	エフェクトでレーザーを表示させてそれに当たり判定を追尾させるように作成していく.
*	担当：　西村 日向
**/

//前方宣言.
class BossIdolState;
class Boss;


class Laser
	: public BossAttackStateBase
{
public:
	//レーザーの攻撃タイミングの取得用
	//列挙型.
	//メモリの節約のために1バイト0~255に制限している.
	enum class enLaser : unsigned char
	{
		None,	//何もしない.
		Charge,	//攻撃をする前の前兆.
		Fire,	//攻撃をする(レーザー).
		Cool,	//攻撃終了.
		Trans,	//レーザー攻撃が終了した際にアイドルへの遷移をする.
	};
public:
	//コンストラクタ.
	Laser(Boss* owner);
	//デストラクタ.
	~Laser() override;
	//最初に入る.
	void Enter() override;
	//動作関数(毎フレーム).
	void Update() override;
	//すべてのUpdate()が終わった時に入るUpdate().
	void LateUpdate() override;
	//描画(レーザーのエフェクトを表示させる).
	void Draw() override;
	//終了時に入る.
	void Exit() override;

	//パリィ成功時の設定(※この攻撃ではパリィが不可能なので0.0fと書いておく).
	std::pair<Boss::enBossAnim, float> GetParryAnimPair() override;
	//デバッグ中にImGuiでLaserのステータスを変更できるように設定する.
	void DrawImGui() override;
	//Laserの攻撃のパラメータ用のjsonファイルの読み込み.
	void LoadSettings() override;
	//Laserの攻撃のパラメータ用のjsonファイルの保存.
	void SaveSettings() const override;
	//変更させるjsonファイルのパスの設定.
	std::filesystem::path GetSettingsFileName() const override
	{
		return std::filesystem::path("Laser.json");
	}
private:
	//レーザー攻撃の列挙.
	enLaser m_State;
	
	//レーザーのため時間.
	float m_ChargeDuration;
	float m_ChargeElapsed;

	//レーザーの発射時間.
	float m_FireDuration;
	float m_FireElapsed;

	//レーザーのパラメータ.
	float m_LaserDamege;
	float m_LaserRadius;
	float m_LaserRange;

	//エフェクト再生.
	bool m_EffectPlayed;

	//攻撃終了時にアイドルに遷移させるために必要.
	std::shared_ptr<BossIdolState> m_pBossIdol;
};
