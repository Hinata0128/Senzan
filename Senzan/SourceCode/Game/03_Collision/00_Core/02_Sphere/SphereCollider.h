#pragma once
#include "../ColliderBase.h"


/**********************************************************************************
* @author    : 淵脇 未来.
* @date      : 2025/10/5.
* @brief     : 球体コリジョン.
**********************************************************************************/

class SphereCollider final
	: public ColliderBase
{
public:
	SphereCollider();
	SphereCollider(std::weak_ptr<const Transform> parentTransform);
	~SphereCollider() override;

	void Update() override;


	// 自身の形状を取得する.
	inline const eShapeType GetShapeType() const noexcept override { return eShapeType::Sphere; }

	// 半径の取得.
	inline float GetRadius() const noexcept override { return m_Radius; }

	// 半径を設定する.
	inline void SetRadius(float Radius) noexcept override { m_Radius = Radius; }

	// 衝突判定.
	CollisionInfo CheckCollision(const ColliderBase& other) const override;

	CollisionInfo DispatchCollision(const SphereCollider& other) const override;
	CollisionInfo DispatchCollision(const CapsuleCollider& other) const override;
	CollisionInfo DispatchCollision(const BoxCollider& other) const override;

private:
	float m_Radius; // 半径.

public:

	// デバッグ描画用設定.
	void SetDebugInfo() override;

};

