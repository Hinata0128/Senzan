#pragma once
#include "System/Singleton/Debug/Log/DebugLog.h"

#include "../ColliderBase.h"

class CollisionDetector;

/**********************************************************************************
* @author    : 淵脇 未来.
* @date      : 2025/10/5.
* @brief     : カプセルコリジョン.
**********************************************************************************/

class CapsuleCollider final
	: public ColliderBase
{
public:
	CapsuleCollider();
	CapsuleCollider(std::weak_ptr<const Transform> parentTransform);
	~CapsuleCollider() override;

	void Update() override;


	// 自身の形状を取得する.
	inline const eShapeType GetShapeType() const noexcept override { return eShapeType::Capsule; }

	// 半径の取得.
	inline float GetRadius() const noexcept override { return m_Radius; }

	// 高さを取得する.
	inline float GetHeight() const noexcept override { return m_Height; }

	// 半径を設定する.
	inline void SetRadius(float Radius) noexcept override
	{
		float max_height = m_Height / 2.0f;

		// 設定されたRadius (Radius) が許容される最大Radiusを超過する場合、最大Radiusに修正.
		if (Radius > max_height)
		{
			m_Radius = max_height;
		}
		else if (Radius < 0.0f)
		{
			// Radiusが負の値にならないようにガード.
			m_Radius = 0.0f;
		}
		else
		{
			m_Radius = Radius;
		}
	}
	// 高さを設定する.
	inline void SetHeight(float Height) noexcept override
	{
		float min_height = 2.0f * m_Radius;

		// 設定された高さ (Height) が最低高さを下回る場合、最低高さに修正.
		if (Height < min_height)
		{
			m_Height = min_height;
		}
		else
		{
			m_Height = Height;
		}
	}

	// 衝突判定.
	CollisionInfo CheckCollision(const ColliderBase& other) const override;

protected:
	CollisionInfo DispatchCollision(const SphereCollider& other) const override;
	CollisionInfo DispatchCollision(const CapsuleCollider& other) const override;
	CollisionInfo DispatchCollision(const BoxCollider& other) const override;

private:
	// カプセルの中心線分の終点 P1 を「計算し」取得.
	static DirectX::XMVECTOR GetCulcCapsuleSegmentStart(const CapsuleCollider* capsule);

	// カプセルの中心線分の終点 P2 を「計算し」取得.
	static DirectX::XMVECTOR GetCulcCapsuleSegmentEnd(const CapsuleCollider* capsule);

	// 点 P から線分 AB への最短距離の二乗を「計算し」取得.
	static float GetCulcClosestPtPointSegmentSq(DirectX::XMVECTOR p, DirectX::XMVECTOR a, DirectX::XMVECTOR b);

	//  線分 AB と線分 CD の最短距離の二乗を「計算し」取得.
	static float GetCulcClosestPtSegmentSegmentSq(DirectX::XMVECTOR a, DirectX::XMVECTOR b, DirectX::XMVECTOR c, DirectX::XMVECTOR d);



private:
	float m_Radius; // 半径.
	float m_Height;	// 高さ.

public:

	// デバッグ描画用設定.
	void SetDebugInfo() override;

};
