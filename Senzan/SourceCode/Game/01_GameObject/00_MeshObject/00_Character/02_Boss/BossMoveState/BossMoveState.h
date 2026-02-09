#pragma once

//基底クラス.
#include "System/Utility/StateMachine/StateBase.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/Boss.h"
#include "Game\03_Collision\00_Core\01_Capsule\CapsuleCollider.h"

//前方宣言.
class Time;

//---------------------
//ボスの攻撃の前方宣言.
//---------------------
class BossSlashState;
class BossChargeState;
class BossChargeSlashState;
class BossLaserState;
class BossMoveContinueState;
class BossShoutState;
class BossSlashState;
class BossJumpOnlState;
class BossStompState;
class BossThrowingState;

#include <random>
#include <array>

/******************************************************************************
*	ボスの動作(左右移動・プレイヤーを囲むように半円を描く).
**/

class BossMoveState final
	: public StateBase<Boss>
{
public:
	enum class DistanceAttack : byte
	{
		Melee,	//近距離.
		Mid,	//中距離.
		Long,	//遠距離.
		None,	//何もしない.
	};

	enum class MovePhase : byte
	{
		Start,
		Run,
		Stop,
		Strafe,
	};

    // Attack identifiers (order matters for arrays)
    enum AttackId {
        Jump = 0,       // ジャンプ
        Shout = 1,      // 叫び
        Slash = 2,      // 通常
        Spinning = 3,   // 回転
        Stomp = 4,      // とびかかり
        Throwing = 5,   // 岩投げ
        Laser = 6,      // レーザー.
        MoveContinue = 7, // そのまま移動3秒
        Count = 8
    };

    MovePhase m_Phase = MovePhase::Start;
public:
	BossMoveState(Boss* owner);
	~BossMoveState();

	//最初に入る.
	void Enter() override;
	//動作.
	void Update() override;
    //かかなくていい.
    void LateUpdate() override;
	//描画.
	void Draw() override;
	//終わるときに一回だけ入る.
	void Exit() override;

public:
	//初期角度を設定する関数.
	void SetInitialAngle(float angle);
    // 設定の読み書き
    void LoadSettings();
    void SaveSettings() const;
private:
	//現在のボスの回転度を確認する.
	float m_RotationAngle;
	double m_RotationSpeed;
	float m_rotationDirection;	//1.0f:順方向, -1.0:逆方向.

	int							m_AnimNo;	//アニメーション番号.
	double						m_AnimSpeed = 0.0001;// アニメーション速度.
	double						m_AnimTimer;// アニメ―ション経過時間.
	DirectX::XMFLOAT3			m_BonePos;	// ボーン座標.

	float m_BaseAngle;

	DirectX::XMFLOAT3 m_InitBossPos;

	float m_Timer = 0.0f;
	float m_SecondTimer = 120.0f;

	std::unique_ptr<BossJumpOnlState>		m_pAttack;
	std::unique_ptr<BossSlashState>			m_pSlash;
	std::unique_ptr<BossStompState>			m_pStomp;
	std::unique_ptr<BossThrowingState>		m_pThrowing;
	std::unique_ptr<BossShoutState>			m_pShout;

	// === デバッグ用距離設定 ===
	static inline float s_NearRange = 15.0f;      // 近距離の閾値
	static inline float s_MidRange = 35.0f;       // 中距離の閾値
	static inline float s_AttackDelay = 5.0f;     // 攻撃開始までの遅延

	// === 攻撃有効/無効フラグ、重み、クールダウン（配列で管理） ===
	static inline float s_RepeatPenalty = 0.25f;

    // Per-attack settings (indexed by AttackId::Count)
    static inline std::array<bool, Count> s_Enable = { true, true, true, true, true, true, true, true };

    // Distances: Near / Mid (遠距離削除)
    enum DistanceIndex { Near = 0, Mid = 1, DistCount = 2 };

    // weights[distance][attack] - stored/edited as percentages that sum to ~100 per distance
    static inline std::array<std::array<float, Count>, DistCount> s_Weight = { {
        std::array<float, Count>{ 12.5f, 12.5f, 12.5f, 12.5f, 12.5f, 12.5f, 12.5f, 12.5f }, // Near (sum = 100)
        std::array<float, Count>{ 12.5f, 12.5f, 12.5f, 12.5f, 12.5f, 12.5f, 12.5f, 12.5f }  // Mid (sum = 100)
    } };

    static inline std::array<float, Count> s_CooldownDefault = { 2.0f, 4.0f, 2.0f, 3.0f, 2.5f, 2.0f, 4.0f, 1.0f };
    // 表示用日本語ラベル (順序: Jump, Shout, Slash, Spinning, Stomp, Throwing, Laser, MoveContinue)
    static inline const char* s_AttackNames[Count] = { "ジャンプ", "叫び", "通常", "回転", "とびかかり", "岩投げ", "レーザー", "そのまま移動" };
    // 内部保存用ID（英語）。JSONキーなどで使う。
    static inline const char* s_AttackIds[Count] = { "Jump", "Shout", "Slash", "Spinning", "Stomp", "Throwing", "Laser", "MoveContinue" };
    static inline const char* s_DistanceIds[DistCount] = { "Near", "Mid" };

    // === デバッグ強制攻撃選択 ===
    static inline int s_ForceAttackIndex = -1;    // -1: ランダム, 0-7: 強制選択

    // runtime cooldown/last-attack tracking
    std::array<float, Count> m_CooldownRemaining{};
    int m_LastAttackId = -1; // last chosen attack id
};
