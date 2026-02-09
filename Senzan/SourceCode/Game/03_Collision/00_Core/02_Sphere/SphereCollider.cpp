#include "System/Singleton/Debug/CollisionVisualizer/CollisionVisualizer.h"

#include "SphereCollider.h"


namespace {
    // 球体の分割数.
    static constexpr int DIVIDE_X = 16;
    static constexpr int DIVIDE_Y = 16;
}

SphereCollider::SphereCollider()
	: ColliderBase	()
	, m_Radius		( 0.5f )
{
}

SphereCollider::SphereCollider(std::weak_ptr<const Transform> parentTransform)
	: ColliderBase	(parentTransform)
	, m_Radius		( 0.5f )
{
}

SphereCollider::~SphereCollider()
{
}

void SphereCollider::Update()
{
  
}

CollisionInfo SphereCollider::CheckCollision(const ColliderBase& other) const
{	
    // フィルター判断.
    if (!ShouldCollide(other)) {
        return {};
    }

	// 形状ごとの衝突判定へディスパッチ.
    return other.DispatchCollision(*this);
}

CollisionInfo SphereCollider::DispatchCollision(const SphereCollider& other) const
{
    return {};
}

CollisionInfo SphereCollider::DispatchCollision(const CapsuleCollider& other) const
{
    return {};
}

CollisionInfo SphereCollider::DispatchCollision(const BoxCollider& other) const
{
    return {};
}

// デバッグ描画用設定.
void SphereCollider::SetDebugInfo()
{
    CollisionVisualizer& visualizer = CollisionVisualizer::GetInstance();
    DebugColliderInfo info = {};


    auto spParentTransform = m_wpTransform.lock();
    // 親の単位行列を作成.
    DirectX::XMMATRIX mat_parent_world = spParentTransform
        ? spParentTransform->GetWorldMatrix()
        : DirectX::XMMatrixIdentity();


    // オフセット行列.
    DirectX::XMMATRIX mat_offset = DirectX::XMMatrixTranslation(
        m_PositionOffset.x, m_PositionOffset.y, m_PositionOffset.z
    );

    // 親のワールド行列にオフセットを乗算してデバッグ用ワールド行列を作成.
    DirectX::XMMATRIX mat_debug_world = DirectX::XMMatrixMultiply(mat_offset, mat_parent_world);

    info.ShapeType = eShapeType::Sphere;
    info.Color = m_Color;

    DirectX::XMStoreFloat4x4(&info.WorldMatrix, mat_debug_world);

    // 半径を Data0.x に設定.
    // (デバッグメッシュが単位半径を想定している場合、この値でスケーリングを行う)
    info.Data0 = DirectX::XMFLOAT4(m_Radius, 0.0f, 0.0f, 0.0f);
    info.Data1 = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

    visualizer.RegisterCollider(info);
}
