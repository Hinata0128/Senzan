#include "CollisionDetector.h"
#include "Game/03_Collision/00_Core/ColliderBase.h" 
#include "Game/03_Collision/00_Core/Ex_CompositeCollider/CompositeCollider.h" 

void CollisionDetector::ExecuteCollisionDetection()
{
    // 前フレームの情報をクリア.
    for (const auto& collider : m_Colliders)
    {
        if (collider)
        {
            collider->ClearCollisionEvents();
        }
    }
    m_PendingResponses.clear();

    // 2. コライダーリストの総当たりチェック
    for (size_t i = 0; i < m_Colliders.size(); ++i)
    {
        for (size_t j = i + 1; j < m_Colliders.size(); ++j)
        {
            ColliderBase* colliderA = m_Colliders[i];
            ColliderBase* colliderB = m_Colliders[j];
            if (!colliderA || !colliderB) { continue; }
            if (!colliderA->GetActive() || !colliderB->GetActive()) { continue; }
#if 1

            eCollisionGroup group_a = colliderA->GetMyMask();
            eCollisionGroup group_b = colliderB->GetMyMask();

            bool is_pattern1 = (group_a & eCollisionGroup::Press) != eCollisionGroup::None &&
                (group_b & eCollisionGroup::BossPress) != eCollisionGroup::None;

            bool is_pattern2 = (group_a & eCollisionGroup::BossPress) != eCollisionGroup::None &&
                (group_b & eCollisionGroup::Press) != eCollisionGroup::None;

            // パターン1 または パターン2 の時に実行
            if (is_pattern1 || is_pattern2)
            {
                // 衝突後の処理
                int i = 0;
                i++;
            }
#endif
            // フィルタの判断と接触の判断.
            CollisionInfo info = colliderA->CheckCollision(*colliderB);
            if (!info.IsHit) { continue; }

            info.AttackAmount = colliderB->GetAttackAmount();

            // Detector側の記録リストに追加.
            m_PendingResponses.push_back(info);

            // Collider A への情報追加.
            colliderA->AddCollisionInfo(info);

            // Collider B への情報追加.
            CollisionInfo info_reverse = info;

            // 法線ベクトルを反転 (反対方向を与える).
            DirectX::XMVECTOR v_normal_reverse = DirectX::XMLoadFloat3(&info.Normal);
            v_normal_reverse = DirectX::XMVectorNegate(v_normal_reverse);
            DirectX::XMStoreFloat3(&info_reverse.Normal, v_normal_reverse);

            // ポインタを入れ替え.
            const ColliderBase* temp_collider = info.ColliderA;
            info_reverse.ColliderA = info.ColliderB;
            info_reverse.ColliderB = temp_collider; 
            info_reverse.AttackAmount = colliderA->GetAttackAmount();

            // 接触点はワールド座標なのでそのまま使用.
            colliderB->AddCollisionInfo(info_reverse);
        }
    }
}

// コライダーの登録.
void CollisionDetector::RegisterCollider(ColliderBase& Collider)
{
    m_Colliders.push_back(&Collider);
}

// コライダーの登録.
void CollisionDetector::RegisterCollider(const CompositeCollider& Collider)
{
    const std::vector<std::unique_ptr<ColliderBase>>& internal_colliders = Collider.GetInternalColliders();

    for (const std::unique_ptr<ColliderBase>& collider_ptr : internal_colliders)
    {
        if (collider_ptr)
        {
            m_Colliders.push_back(collider_ptr.get());
        }
    }
}

// コライダーの解除.
void CollisionDetector::UnregisterCollider(ColliderBase* Collider)
{
    if (!Collider) { return; }
    if (m_Colliders.empty()) { return; }

    auto it = std::remove(m_Colliders.begin(), m_Colliders.end(), Collider);
    if (it != m_Colliders.end())
    {
        m_Colliders.erase(it, m_Colliders.end());
    }
}

// コライダーの解除.
void CollisionDetector::UnregisterCollider(const CompositeCollider& Collider)
{
    const std::vector<std::unique_ptr<ColliderBase>>& internal_colliders = Collider.GetInternalColliders();

    for (const std::unique_ptr<ColliderBase>& collider_ptr : internal_colliders)
    {
        if (collider_ptr)
        {
            UnregisterCollider(collider_ptr.get());
        }
    }
}
