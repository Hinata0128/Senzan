#pragma once
#include <vector>
#include <memory>
#include "../ColliderBase.h"

/**********************************************************************************
* @author    : 淵脇 未来.
* @date      : 2025/11/20.
* @brief     : 複数のColliderをラップし、一つとして扱う.
* @pattern   : Composite.
**********************************************************************************/

class CompositeCollider 
    : public ColliderBase
{
public:

    inline void Update() override{ for (auto& collider : m_Colliders) collider->Update(); }

    // 形状の追加 (所有権を移動するため unique_ptr を推奨).
    void AddCollider(std::unique_ptr<ColliderBase> collider) {
        m_Colliders.push_back(std::move(collider));
    }

    const std::vector<std::unique_ptr<ColliderBase>>& GetInternalColliders() const noexcept {
        return m_Colliders;
    }

    inline void SetDebugInfo() override { for (auto& collider : m_Colliders) { if(collider->GetActive()) collider->SetDebugInfo(); } }

protected:

    CollisionInfo CheckCollision(const ColliderBase& other) const { return CollisionInfo(); };
    CollisionInfo DispatchCollision(const SphereCollider& other) const override { return CollisionInfo(); }
    CollisionInfo DispatchCollision(const CapsuleCollider& other) const override { return CollisionInfo(); }
    CollisionInfo DispatchCollision(const BoxCollider& other) const override { return CollisionInfo(); }

private:
    // 当たり判定リスト.
    std::vector<std::unique_ptr<ColliderBase>> m_Colliders;
};