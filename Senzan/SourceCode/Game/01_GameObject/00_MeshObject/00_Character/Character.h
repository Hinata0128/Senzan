#pragma once
#include "Game/01_GameObject/00_MeshObject/MeshObject.h"
#include <vector>
class CompositeCollider;


/**************************************************
*   キャラクタークラス
**/
class Character
	: public MeshObject
{
public:
	Character();
	virtual ~Character();

	virtual void Update() override;
	virtual void LateUpdate() override;
	virtual void Draw() override;

    // 再生要求を行い、再生中のエフェクトハンドルを管理する
    void PlayEffect(const std::string& effectName, const DirectX::XMFLOAT3& offset = DirectX::XMFLOAT3(0.f,0.f,0.f), float scale = 1.0f, bool isUI = false);

    // ワールド座標を直接指定してエフェクトを再生する
    void PlayEffectAtWorldPos(const std::string& effectName, const DirectX::XMFLOAT3& worldPos, float scale = 1.0f, bool isUI = false);

    // ワールド座標とオイラー回転を指定してエフェクトを再生する
    void PlayEffectAtWorldPos(const std::string& effectName, const DirectX::XMFLOAT3& worldPos, const DirectX::XMFLOAT3& eulerRotation, float scale = 1.0f, bool isUI = false);

    // Play effect in UI (screen-space pixels). screenPos is in pixels with origin at top-left.
    void PlayEffectUIAtScreenPos(const std::string& effectName, const DirectX::XMFLOAT2& screenPos, float scale = 1.0f);

	inline float GetMaxHP() const noexcept { return m_MaxHP; }
	inline float GetHP() const noexcept { return m_HP; }


protected:

	// 衝突応答処理.
	void HandleCollisionResponse();

	// ダメージ適用.
	void ApplyDamage(float damageAmount);

	// 衝突_被ダメージ.
	virtual void HandleDamageDetection() = 0;
	// 衝突_攻撃判定.
	virtual void HandleAttackDetection() = 0;
	// 衝突_回避.
    virtual void HandleDodgeDetection() {};


protected:
	std::unique_ptr<CompositeCollider>	m_upColliders;	// 衝突.

    // 再生中のエフェクトハンドル群（AgeFrames を保持して即時削除を防止）
    struct EffectHandleEntry { int Handle = -1; int AgeFrames = 0; bool IsUI = false; };
    std::vector<EffectHandleEntry> m_EffectHandles;


	float m_MaxHP;							// 最大.
	float m_HP;								// 体力.
};
