#pragma once
#include "Game/01_GameObject/00_MeshObject/MeshObject.h"
#include "Game/03_Collision/00_Core/Ex_CompositeCollider/CompositeCollider.h"
#include <DirectXMath.h>

class ColliderBase;

class SnowBall
    : public MeshObject
{
public:
	SnowBall();
	~SnowBall() override;

    void Update() override;
    void LateUpdate() override {};
	void Draw() override;

	void Fire(const DirectX::XMFLOAT3 PlayerPos, const DirectX::XMFLOAT3 BossPos);

    // パリィされたかどうか.
	bool IsParried() const { return m_IsParried; }

	// フェード完了後に Manager によって削除されるべきか
	bool ShouldDestroy() const;

public:
	bool IsAction = false;
	bool IsVisible = false;
    // Manager が削除判定を行うためのフラグ
    bool Destroyed = false;

private:
	void Launch();
	void HandleCollision();  // 当たり判定処理.
	void OnParried();        // パリィされた時の処理.

	// ヒット時の視覚・アニメ処理
	void HandleHitVisual();

private:
	DirectX::XMFLOAT3 Player_Pos; // P2 (着弾点)
	DirectX::XMFLOAT3 Boss_Pos;   // P0 (開始点)
	DirectX::XMFLOAT3 Current_Pos; // P1 (制御点)
	DirectX::XMFLOAT3 Init_Pos;    // 待機位置

    float ThrowingTime;
    // 飛行にかける秒数（インスタンスごとに変更可能）
    float Totle_ThrowingTime = 0.5f; // デフォルトはコンストラクタで設定

	// 当たり判定.
    std::unique_ptr<CompositeCollider> m_upColliders;
    ColliderBase* m_pAttackCollider;

    // パリィ状態.
    float m_ParriedAnimTime;

    // 内部状態管理.
    enum class State : uint8_t { Idle = 0, Launched, HitFalling, LandedFading, Parried };
    State m_State;
    // 互換用フラグ（既存コード参照のため維持）
    bool m_IsParried;
    bool m_IsHitAnimPlaying;
    // 着地のための落下状態と速度
    bool m_IsFalling;
    float m_FallSpeed; // 単位: world units / sec
    // 着地後のフェード処理
    bool m_IsLanded;
    float m_FadeDuration; // フェードにかける秒数
    float m_FadeTimer;
    float m_GroundY; // 地面の Y 座標（環境に合わせて調整）
    // 自己消滅フラグ（フェード完了後に Manager により削除される）
    bool m_ShouldDestroy;
    // 割れた表示を一度だけ行うフラグ
    bool m_HasBrokenVisual;

    // 跳ね返り時の速度
    float m_BounceSpeed;
    // パリィ時にボスへ到達させるための補助データ
    DirectX::XMFLOAT3 m_ParryStartPos; // パリィ開始時の位置
    float m_ParryElapsed;              // 経過時間
    float m_ParryDuration;             // 到達にかける時間（秒）
};
