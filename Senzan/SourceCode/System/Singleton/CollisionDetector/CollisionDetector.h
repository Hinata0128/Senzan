#pragma once
#include "System/Singleton/SingletonTemplate.h"

class ColliderBase;
class CompositeCollider;
struct CollisionInfo;

class CollisionDetector final
	: public Singleton<CollisionDetector>
{
public:
    friend Singleton<CollisionDetector>;


public:

    // 毎フレーム呼び出され、全ての衝突をチェック・処理する.
    void ExecuteCollisionDetection();

    // コライダーの登録.
    void RegisterCollider(ColliderBase& Collider);
    void RegisterCollider(const CompositeCollider& Colliders);

    // コライダーの解除.
    void UnregisterCollider(ColliderBase* Collider);
    void UnregisterCollider(const CompositeCollider& Collider);

private:
    CollisionDetector() = default;
    ~CollisionDetector() = default;
    CollisionDetector(const CollisionDetector&) = delete;
    CollisionDetector& operator=(const CollisionDetector&) = delete;

private:
    // 登録されたコライダーリスト.
    std::vector<ColliderBase*> m_Colliders;
    
    // 毎フレーム検出された全ての衝突情報を保持するリスト.
    std::vector<CollisionInfo> m_PendingResponses;

};