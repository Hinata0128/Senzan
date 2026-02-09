#pragma once
class ColliderBase;
	
// 衝突結果構造体.
struct CollisionInfo
{
	// 衝突が発生したかどうか.
	bool IsHit = false;

	// 衝突法線.
	DirectX::XMFLOAT3 Normal = {};

	// めり込みの深さ.
	float PenetrationDepth = 0.0f;

	// 接触点.
	DirectX::XMFLOAT3 ContactPoint = {};

	// 攻撃力.
	float AttackAmount = 0.0f;

	// 衝突に関わったコライダーへのポインタ.
	const ColliderBase* ColliderA = nullptr;
	const ColliderBase* ColliderB = nullptr;
};