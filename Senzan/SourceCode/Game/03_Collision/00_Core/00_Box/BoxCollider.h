#pragma once
#include "../ColliderBase.h"

class CollisionDetector;

/**********************************************************************************
* @author    : 淵脇 未来.
* @date      : 2025/10/5.
* @brief     : 四角コリジョン.
**********************************************************************************/

class BoxCollider final
	: public ColliderBase
{
public:
	BoxCollider();
	BoxCollider(std::weak_ptr<const Transform> parentTransform);
	~BoxCollider() override;

	void Update() override;

	// 自身の形状を取得する.
	inline const eShapeType GetShapeType() const noexcept override { return eShapeType::Box; }

	// 辺の長さの取得.
	inline DirectX::XMFLOAT3 GetSize() const noexcept override { return m_Size; }

	// 辺の長さの設定.
	inline void SetSize(DirectX::XMFLOAT3 size) noexcept override { m_Size = size; }

	// 衝突判定.
	CollisionInfo CheckCollision(const ColliderBase& other) const override;

	CollisionInfo DispatchCollision(const SphereCollider& other) const override;
	CollisionInfo DispatchCollision(const CapsuleCollider& other) const override;
	CollisionInfo DispatchCollision(const BoxCollider& other) const override;

private:
	DirectX::XMFLOAT3 m_Size; // 一片の長さ.	

public:

	// デバッグ描画用設定.
	void SetDebugInfo() override;

};