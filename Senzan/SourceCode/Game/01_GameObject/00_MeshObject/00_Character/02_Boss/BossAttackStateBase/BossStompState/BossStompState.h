#pragma once

#include "..//BossAttackStateBase.h"	//基底クラス.

class BossIdolState;
class Boss;

class BossStompState final
	: public BossAttackStateBase
{
public:
	enum class enAttack : byte
	{
		None,		//何もしない.
		Stomp,		//踏みつけ攻撃をする.
		CoolTime,	//クールタイム.
		Trans		//Idolに状態遷移.
	};

public:
	BossStompState(Boss* owner);
	~BossStompState() override;

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
    std::filesystem::path GetSettingsFileName() const override { return std::filesystem::path("BossStompState.json"); }
private:
    void BossAttack();
    // (use base timing/windows)
private:
	enAttack m_List;

	std::shared_ptr<BossIdolState> m_pIdol;
	//====================================================
	// 踏みつけに必要になるメンバ変数を書く.
	//====================================================
	DirectX::XMFLOAT3 m_Velocity;

	//ジャンプの初速度.
	float m_JumpPower;
	//重力加速度.
	float m_Gravity;
	//ジャンプ中のフラグ.
	bool m_JumpFrag;
	// 前フレームの Y 値を保持してスムーズに移動させる
	// float m_LastY; // 一時無効化: Y 制御オフ
	//着地フラグ.
	//この時に時間を取得して3秒後にIdolに遷移等を書く.
	bool m_GroundedFrag;
	//===================================================
	// タイマー変数.
	//===================================================
	//タイマーの初期化変数.
	float m_Timer;
	//遷移させるタイミングの変数.
	float TransitionTimer;

	//上がる速度のスピード設定用.
	float m_UpSpeed;

	// State timers / flags
	float m_WaitSeconds = 0.4f;    // seconds before slowing animation
	float m_SlowAnimSpeed = 0.5f;  // slowed animation speed value
	bool  m_AnimSlowed = false;    // whether animation was slowed
	bool  m_IsMoving = false;      // whether boss is currently moving (stomp phase)
    // use m_CurrentTime from base for timing
    // slow duration control: how long animation stays slowed after delay
    float m_SlowDuration = 1.5f;  // seconds to keep animation slowed
    float m_SlowElapsed = 0.0f;   // elapsed time since slow started
    // stomp collider settings (persisted)
    float m_StompRadius = 30.0f;
    float m_StompDamage = 15.0f;
    bool  m_StompActive = false;

	// movement easing params
	float m_MoveDuration = 0.8f;   // duration of the movement/easing (seconds)
	float m_MoveTimer = 0.0f;      // elapsed time since movement started
	float m_Distance = 0.0f;       // total horizontal distance to move
	DirectX::XMFLOAT3 m_MoveVec;   // normalized horizontal direction vector
    // vertical control for ascent/descent
    bool m_UseVerticalEasing = true;
    float m_AscentHeight = 10.0f;       // peak additional height relative to start
    float m_AscentDuration = 0.25f;     // seconds
    float m_DescentDuration = 0.55f;    // seconds
    MyEasing::Type m_AscentEasing = MyEasing::Type::OutSine;
    MyEasing::Type m_DescentEasing = MyEasing::Type::InQuad;
    float m_VerticalTimer = 0.0f;       // timer for vertical easing
    float m_StartY = 0.0f;

	//====================================================
	// 飛びかかり（前方移動）用メンバ変数.
	//====================================================
	float m_ForwardSpeed;           // 前方移動速度.
	DirectX::XMFLOAT3 m_TargetPos;  // 目標位置（プレイヤー位置）.
	DirectX::XMFLOAT3 m_StartPos_Stomp;   // 開始位置.
    bool m_HasLanded;               // 着地済みフラグ（ダメージ1回のみ）
    // using base's m_ColliderWindows

    float m_JumpedSoundTiming;
    bool m_JumpedSoundPlayed;
    bool m_LandedSoundPlayed;
};
