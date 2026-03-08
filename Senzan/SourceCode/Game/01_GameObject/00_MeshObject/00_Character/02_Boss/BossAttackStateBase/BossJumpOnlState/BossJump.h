#pragma once
#include "..//BossAttackStateBase.h"

/*************************************************************************************************
*	ボスのジャンプ攻撃.
*	ボスが飛んだ時に姿を消し時間経過で上から表示させて落ちてくる.
*	着地判定がわかった時にエフェクトを表示させて実装する.
*	担当者: 西村 日向
**/

class BossIdolState;

class BossJump final
	: public BossAttackStateBase
{
public:
	enum class enJump : byte
	{
		None,
		Charge,
		Jump,
		Attack,
		CoolDown,
		Trans
	};
public:
	BossJump(Boss* owner);
	~BossJump() override;

	void Enter() override;
	void Update() override;
	void LateUpdate() override;
	void Draw() override;
	void DrawImGui() override;
	void Exit() override;

	// PlayerのParry成功時硬直させたいアニメーションとタイミング.
	std::pair<Boss::enBossAnim, float> GetParryAnimPair() override;
private:
	void JumpTime();
	void BossAttack();
	void ChargeTime();
private:
	enJump m_List;
	DirectX::XMFLOAT3 m_Velocity;

	//特殊攻撃中のフラグ.
	bool m_SpecialFrag;
	//着地フラグ.
	bool m_GroundedFrag;


	//高さ上昇量(ジャンプで上昇する量).
	float m_RiseHeight;
	//上昇速度.
	float m_RiseSpeed;
	// 再出現までの待機秒数
	float m_ReappearDelay;
	// 落下速度
	float m_FallSpeed;
	// 内部ターゲットY位置
	float m_RiseTargetY;
	// フラグ: 上昇処理開始済み
	bool m_RiseStarted;
	// フラグ: 現在落下中
	bool m_IsFalling;
	// フラグ: 再出現待ち中
	bool m_WaitingReappear;
	// フラグ: 落下モードに入る1秒前のエフェクトを再生済みか
	bool m_HasPlayedPreFallEffect;
	// 落下モードに入る何秒前にエフェクトを再生するか
	float m_PreFallSeconds;

	//時間のメンバ変数.
	//時間加算に使用する.
	float m_Timer;
	//遷移するときに使用する.
	float m_TransitionTimer;
	//攻撃開始用のタイム変数.
	float m_AttackTimer;

	//上がる速度の設定.
	float m_UpSpeed;

	bool Isfast = false;

	DirectX::XMFLOAT3 m_TargetDirection; // 突進開始時に確定した目標方向
	float m_MaxTrackingAngle;			 // 追尾の限界角度 (度数法)
	float m_AttackMoveSpeed;			 // 突進速度
	float m_AttackDistance;				 // 突進する最大距離
	float m_DistanceTraveled;

};
