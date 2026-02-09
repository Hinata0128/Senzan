#pragma once
#include "CollisionInfo.h"

struct Transform; // forward declaration to avoid include

class CollisionDetector;
class BoxCollider;
class CapsuleCollider;
class SphereCollider;

// MEMO : ビット演算を定義するためにネームスペースの外に宣言.
// 当たり判定.
enum class eCollisionGroup : uint32_t
{
	None = 0,

	Player_Attack	= 1 << 0,
	Player_Damage	= 1 << 1,
	Player_Parry_Suc= 1 << 2,
	Player_Parry_Noc= 1 << 3,
	Player_Parry_Fai= 1 << 4,
	Player_JustDodge= 1 << 5,
	Enemy_Attack	= 1 << 6,
	Enemy_PreAttack = 1 << 7,
	Enemy_Damage	= 1 << 8,
	Press			= 1 << 9,
	BossPress		= 1 <<10,

	_Max = 0xFFFFFFFF,
};
DEFINE_ENUM_FLAG_OPERATORS(eCollisionGroup)


/**********************************************************************************
* @author    : 淵脇 未来.
* @date      : 2025/10/5.
* @brief     : 当たり判定(基底).
**********************************************************************************/

class ColliderBase
{
public:
	// 形状派生クラスから判定関数(DispatchCollision)を2重ディスパッチするためフレンドに追加.
	friend class BoxCollider;
	friend class CapsuleCollider;
	friend class SphereCollider;

public:
	// 当たり判定の形.
	enum class eShapeType : uint32_t
	{
		Box = 0,
		Sphere,
		Capsule,
		_Max
	};

public:
	ColliderBase();
	ColliderBase(std::weak_ptr<const Transform> parentTransform);
	virtual ~ColliderBase();

	virtual void Update() = 0;

	// 親のトランスフォームを取得する.
	//inline const std::weak_ptr<Transform>& GetParentTransform() const noexcept { return m_wpTransform; }

	// 座標を取得する.
	inline const DirectX::XMFLOAT3 GetPosition() const noexcept; 

	// 自身の形状を取得する.
	inline virtual const eShapeType GetShapeType() const noexcept { return eShapeType::_Max; }

	// 座標のオフセット値を取得する.
	inline const DirectX::XMFLOAT3 GetPositionOffset() const noexcept { return m_PositionOffset; }
	inline void SetPositionOffset(DirectX::XMFLOAT3 NewOffset) noexcept { m_PositionOffset = NewOffset; }
	inline void SetPositionOffset(float NewOffset_x, float NewOffset_y, float NewOffset_z) noexcept { m_PositionOffset = DirectX::XMFLOAT3(NewOffset_x, NewOffset_y, NewOffset_z); }

	// 大きさ取得(派生で実装).
	virtual float GetRadius() const noexcept { return 0.0f; }
	virtual float GetHeight() const noexcept { return 0.0f; }
	virtual DirectX::XMFLOAT3 GetSize() const noexcept { return DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f); }

	// 大きさ設定(派生で実装).
	virtual void SetRadius(float Radius) noexcept {}
	virtual void SetHeight(float Height) noexcept {}
	virtual void SetSize(DirectX::XMFLOAT3 Size) noexcept {}
	
	// 有効か否か.
	void SetActive(bool isActive) noexcept { m_IsActive = isActive; }
	bool GetActive() const noexcept { return m_IsActive; }

	// 色の設定.
	void SetColor(Color::eColor NewColor) noexcept { m_Color = Color::GetColorValue(NewColor); }
	inline void SetColor(DirectX::XMFLOAT4 NewColor) noexcept { m_Color = NewColor; }
	inline void SetColor(float NewR, float NewG, float NewB, float NewA) noexcept { m_Color = DirectX::XMFLOAT4(NewR, NewG, NewB, NewA); }

	void SetAttackAmount(float Amount)noexcept { m_AttackAmount = Amount; }
	float GetAttackAmount() const noexcept { return m_AttackAmount; }

	// 衝突情報を追加する.
	inline void AddCollisionInfo(const CollisionInfo& info) noexcept { m_CollisionEvents.push_back(info); }

	// 衝突情報を取得する.
	inline const std::vector<CollisionInfo>& GetCollisionEvents() const noexcept { return m_CollisionEvents; }

	// 情報をクリアするメソッド.
	inline void ClearCollisionEvents() noexcept { m_CollisionEvents.clear(); }
	
	// 自身のグループを設定.
	inline void SetMyMask(eCollisionGroup MyMask) noexcept { m_MyMask = MyMask; }
	// 衝突を許可するグループを設定.
	inline void SetTarGetTargetMask(eCollisionGroup TarGetTargetMask) noexcept { m_TarGetTargetMask = TarGetTargetMask; }

	inline eCollisionGroup GetMyMask() const noexcept { return m_MyMask; }
	inline eCollisionGroup GetTargetMask() const noexcept { return m_TarGetTargetMask; }

	// 相手と衝突すべきか.
	inline bool ShouldCollide(const ColliderBase& other) const noexcept
	{
		bool A_collides_with_B = (m_TarGetTargetMask & other.m_MyMask) != eCollisionGroup::None;
		bool B_collides_with_A = (other.m_TarGetTargetMask & m_MyMask) != eCollisionGroup::None;
		return A_collides_with_B && B_collides_with_A;
	}

	// 他のColliderとの衝突.
	virtual CollisionInfo CheckCollision(const ColliderBase& other) const = 0;

protected:
	// 形状ごとの衝突処理.
	virtual CollisionInfo DispatchCollision(const SphereCollider& other) const = 0;
	virtual CollisionInfo DispatchCollision(const CapsuleCollider& other) const = 0;
	virtual CollisionInfo DispatchCollision(const BoxCollider& other) const = 0;

protected:

	std::weak_ptr<const Transform> m_wpTransform;	// 持ち主のトランスフォーム.
	DirectX::XMFLOAT3	m_PositionOffset;		    // オフセット位置.
	bool				m_IsActive; 				// アクティブか否か.
	float				m_AttackAmount; 			// 攻撃力.

	// コリジョンフィルター用.
	eCollisionGroup m_MyMask = eCollisionGroup::_Max;		// 自身が所属するグループ.
	eCollisionGroup m_TarGetTargetMask = eCollisionGroup::_Max;	// 衝突対象とするグループ.

	// 検出された衝突情報のリスト.
	std::vector<CollisionInfo> m_CollisionEvents;

public:
	//------------デバッグ描画用-----------.

	virtual void SetDebugInfo() = 0;

protected:
	DirectX::XMFLOAT4	m_Color; 					// 表示色.

};

// 座標を取得する.
inline const DirectX::XMFLOAT3 ColliderBase::GetPosition() const noexcept
{
	if (auto spTransform = m_wpTransform.lock())
	{
		DirectX::XMVECTOR v_position = DirectX::XMLoadFloat3(&spTransform->Position);
		DirectX::XMVECTOR v_offset = DirectX::XMLoadFloat3(&m_PositionOffset);

		// 親の回転（Yaw）を取得し、オフセットを回転させる.
		float yaw = spTransform->Rotation.y;
		DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationY(yaw);
		DirectX::XMVECTOR v_rotated_offset = DirectX::XMVector3TransformCoord(v_offset, rotationMatrix);

		DirectX::XMVECTOR v_result_pos = DirectX::XMVectorAdd(v_position, v_rotated_offset);

		DirectX::XMFLOAT3 result_pos = {};
		DirectX::XMStoreFloat3(&result_pos, v_result_pos);
		return result_pos;
	}
	return m_PositionOffset;
}
