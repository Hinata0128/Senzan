#include "System/Singleton/Debug/CollisionVisualizer/CollisionVisualizer.h"

#include "BoxCollider.h"


BoxCollider::BoxCollider()
	: ColliderBase	()
	, m_Size		( 1.0f, 1.0f, 1.0f )	
{
}

BoxCollider::BoxCollider(std::weak_ptr<const Transform> parentTransform)
	: ColliderBase	(parentTransform)
	, m_Size		( 1.0f, 1.0f, 1.0f )	
{
}

BoxCollider::~BoxCollider()
{
}

void BoxCollider::Update()
{

}

CollisionInfo BoxCollider::CheckCollision(const ColliderBase& other) const
{
    // フィルター判断.
    if (!ShouldCollide(other)) {
        return {};
    }

    return other.DispatchCollision(*this);
}

CollisionInfo BoxCollider::DispatchCollision(const SphereCollider& other) const
{
    return {};
}

CollisionInfo BoxCollider::DispatchCollision(const CapsuleCollider& other) const
{
    return {};
}

CollisionInfo BoxCollider::DispatchCollision(const BoxCollider& other) const
{
    return {};
}

// デバッグ描画用設定.
void BoxCollider::SetDebugInfo()
{
	CollisionVisualizer& visualizer = CollisionVisualizer::GetInstance();

    // Visualizerに渡す情報.
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

    info.ShapeType = eShapeType::Box;
    info.Color = m_Color;

    DirectX::XMStoreFloat4x4(&info.WorldMatrix, mat_debug_world);

    // Boxのサイズを Data0 に設定 (Boxのメッシュは単位サイズを想定).
    info.Data0 = DirectX::XMFLOAT4(m_Size.x, m_Size.y, m_Size.z, 1.0f);
    info.Data1 = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

    visualizer.RegisterCollider(info);
}

