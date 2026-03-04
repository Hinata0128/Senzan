#pragma once
#include "Game/01_GameObject/00_MeshObject/MeshObject.h"
#include "Game/03_Collision/00_Core/Ex_CompositeCollider/CompositeCollider.h"

class ColliderBase;

class Ball final
	: public MeshObject
{
public:
	Ball();
	~Ball() override;
	//動作関数.
	void Update() override;
	void LateUpdate() override {};
	//描画関数.
	void Draw() override;

	void Fire(const DirectX::XMFLOAT3 PlayerPos, const DirectX::XMFLOAT3 BossPos);
	//パリィされたかどうか.
	bool IsParried() const { return m_IsParried; }
	//フェード完了後に Manager によって削除されるべきか
	//破壊される.
	bool Destroy() const;

public:
	bool IsAction;
	bool IsVisible;
	//Managerが削除判定を行うためのフラグ
	bool Destroyed;
private:
	void Launch();
	void HandleCollision();  //当たり判定処理.
	void OnParried();        //パリィされた時の処理.

	//ヒット時の視覚・アニメ処理.
	void HandleHitVisual();

private:
	DirectX::XMFLOAT3 Player_Pos_F;  //P2 (着弾点).
	DirectX::XMFLOAT3 Boss_Pos_F;	 //P0 (開始点).
	DirectX::XMFLOAT3 Current_Pos_F; //P1 (制御点).
	DirectX::XMFLOAT3 Init_Pos_F;    //待機位置.

	float ThrowingTime;
	//飛行にかける秒数（インスタンスごとに変更可能）.
	float Totle_ThrowingTime;
	//当たり判定.
	std::unique_ptr<CompositeCollider> m_upColliders;
	ColliderBase* m_pAttackCollider;

	//パリィ状態.
	float m_ParriedAnimTime;
	//内部状態管理.
	enum class enSnowBall : uint8_t
	{
		Idole = 0,
		Launched,
		HitFalling,
		LandedFading,
		Parried
	};
	// 互換用フラグ（既存コード参照のため維持）
	bool m_IsParried;
	bool m_IsHitAnimPlaying;
	//着地のための落下状態と速度.
	bool m_IsFalling;
	float m_FallSpeed;
	//着地後のフェード処理.
	bool m_IsLanded;
	float m_FadeDuration;	//フェードにかける秒数.
	float m_FadeTimer;		//フェード開始からの経過時間.
	float m_GroundY;		//地面のY座標.
	//雪玉の自己消滅用のフラグ.
	bool m_ShouldDestroy;
	//割れた表示を一度だけ行うフラグ.
	bool m_HasBrokenVisual;
	//跳ね返り時の速度.
	float m_BounceSpeed;
	//パリィの開始位置と補間用タイマー.
	DirectX::XMFLOAT3 m_ParryStartPos;	//パリィ開始時の位置.
	float m_ParryElapsed;		//パリィ開始からの経過時間.
	float m_ParryDuration;		//パリィにかける秒数.
};