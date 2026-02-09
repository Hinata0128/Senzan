#include "Character.h"
#include "Game/03_Collision/00_Core/ColliderBase.h"
#include "Game/03_Collision/00_Core/Ex_CompositeCollider/CompositeCollider.h"
#include "Game/03_Collision/00_Core/00_Box/BoxCollider.h"
#include "Game/03_Collision/00_Core/01_Capsule/CapsuleCollider.h"
#include "Game/03_Collision/00_Core/02_Sphere/SphereCollider.h"
#include "System/Singleton/ResourceManager/EffectManager/EffekseerManager.h"
#include "System/Singleton/CameraManager/CameraManager.h"
#include "Resource/Effect/EffectResource.h"
#include "System/Utility/Math/Math.h"

Character::Character()
	: MeshObject    ()
	, m_upColliders ()
	, m_MaxHP       (100.f)
	, m_HP          (100.f)
{
	m_upColliders = std::make_unique<CompositeCollider>();
}

Character::~Character()
{
}

void Character::Update()
{
	m_upColliders->SetDebugInfo();

    // 再生中のエフェクトハンドルを更新し、存在しなくなったものを削除する
    if (!m_EffectHandles.empty())
    {
        auto mgr = EffekseerManager::GetInstance().GetManager();
        const int MIN_AGE_FRAMES = 5; // 最低フレーム保持して即時削除を防止
        for (auto it = m_EffectHandles.begin(); it != m_EffectHandles.end(); )
        {
            auto &entry = *it;
            int handle = entry.Handle;
            if (handle == -1) { it = m_EffectHandles.erase(it); continue; }

            // Advance effect playback by one frame (1/60s). UpdateHandle expects frames (1 = 1/60s)
            EffekseerManager::GetInstance().UpdateHandle(handle);

            // 年齢を増やす
            entry.AgeFrames += 1;

            // 最低フレーム数に達するまで存在チェックは行わない
            if (entry.AgeFrames < MIN_AGE_FRAMES) { ++it; continue; }

            if (mgr == nullptr || !mgr->Exists(handle))
            {
                std::string msg = std::string("Effect handle ") + std::to_string(handle) + std::string(" expired and removed (Age=") + std::to_string(entry.AgeFrames) + std::string(")");
                Log::GetInstance().LogInfo(msg);
                it = m_EffectHandles.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

void Character::PlayEffect(const std::string& effectName, const DirectX::XMFLOAT3& offset, float scale, bool isUI)
{
    // EffectResource から取得して Effekseer に再生依頼
    auto effect = EffectResource::GetResource(effectName);
    if (effect == nullptr) return;

    DirectX::XMFLOAT3 pos = GetPosition();
    float x = pos.x + offset.x;
    float y = pos.y + offset.y;
    float z = pos.z + offset.z;

    int handle = EffekseerManager::GetInstance().GetManager()->Play(effect, x, y, z);
    if (handle != -1)
    {
        if (!MyMath::IsNearlyEqual(scale, 1.0f))
        {
            EffekseerManager::GetInstance().GetManager()->SetScale(handle, scale, scale, scale);
        }
        m_EffectHandles.push_back({ handle, 0, isUI });
    }
}

void Character::PlayEffectAtWorldPos(const std::string& effectName, const DirectX::XMFLOAT3& worldPos, float scale, bool isUI)
{
    auto effect = EffectResource::GetResource(effectName);
    if (effect == nullptr) return;

    int handle = EffekseerManager::GetInstance().GetManager()->Play(effect, worldPos.x, worldPos.y, worldPos.z);
    if (handle != -1)
    {
        if (!MyMath::IsNearlyEqual(scale, 1.0f))
        {
            EffekseerManager::GetInstance().GetManager()->SetScale(handle, scale, scale, scale);
        }
        m_EffectHandles.push_back({ handle, 0, isUI });
    }
}

void Character::PlayEffectUIAtScreenPos(const std::string& effectName, const DirectX::XMFLOAT2& screenPos, float scale)
{
    auto effect = EffectResource::GetResource(effectName);
    if (effect == nullptr) return;

    // Effekseer: Play with orthographic coordinates requires using RenderHandleUI later.
    int handle = EffekseerManager::GetInstance().GetManager()->Play(effect, screenPos.x, screenPos.y, 0.0f);
    if (handle != -1)
    {
        if (!MyMath::IsNearlyEqual(scale, 1.0f))
        {
            EffekseerManager::GetInstance().GetManager()->SetScale(handle, scale, scale, scale);
        }
        m_EffectHandles.push_back({ handle, 0, true });
    }
}

void Character::PlayEffectAtWorldPos(const std::string& effectName, const DirectX::XMFLOAT3& worldPos, const DirectX::XMFLOAT3& eulerRotation, float scale, bool isUI)
{
    auto effect = EffectResource::GetResource(effectName);
    if (effect == nullptr) return;

    int handle = EffekseerManager::GetInstance().GetManager()->Play(effect, worldPos.x, worldPos.y, worldPos.z);
    if (handle != -1)
    {
        // 回転指定がある場合は Effekseer の SetRotation を使用（ラジアンで渡す）
        EffekseerManager::GetInstance().GetManager()->SetRotation(handle, eulerRotation.x, eulerRotation.y, eulerRotation.z);

        if (!MyMath::IsNearlyEqual(scale, 1.0f))
        {
            EffekseerManager::GetInstance().GetManager()->SetScale(handle, scale, scale, scale);
        }
        m_EffectHandles.push_back({ handle, 0, isUI });
    }
}

void Character::LateUpdate()
{
	HandleCollisionResponse();
}

void Character::Draw()
{
	MeshObject::Draw();

    // 再生中のエフェクトを描画
    if (!m_EffectHandles.empty())
    {
        auto camera = CameraManager::GetInstance().GetCurrentCamera();
        for (auto &entry : m_EffectHandles)
        {
            if (entry.Handle == -1) continue;
            if (entry.IsUI) {
                // UI エフェクトは正射影で描画
                EffekseerManager::GetInstance().RenderHandleUI(entry.Handle);
            }
            else {
                if (camera) EffekseerManager::GetInstance().RenderHandle(entry.Handle, camera.get());
            }
        }
    }
}

// 衝突応答処理.
void Character::HandleCollisionResponse()
{
	if (!m_upColliders) return;

	// このオブジェクトの当たり判定を取得.
	const  std::vector<std::unique_ptr<ColliderBase>>& internal_colliders = m_upColliders->GetInternalColliders();

	// 当たり判定を走査.
	for (const std::unique_ptr<ColliderBase>& collider_ptr : internal_colliders)
	{
		const ColliderBase* current_collider = collider_ptr.get();
		if (!current_collider) { continue; }

		// 自分がPress属性出ないならreturn.
		if ((current_collider->GetMyMask() & eCollisionGroup::Press) == eCollisionGroup::None) {
			continue;
		}

		// 衝突イベントリストを取得.
		const std::vector<CollisionInfo>& events = current_collider->GetCollisionEvents();

		// イベントを走査.
		for (const CollisionInfo& info : events)
		{
			if (!info.IsHit) continue;
			const ColliderBase* otherCollider = info.ColliderB;
			if (!otherCollider) { continue; }

			// 相手のグループが Press であるか (このPressグループとの衝突のみを処理する).
			if ((otherCollider->GetMyMask() & eCollisionGroup::BossPress) == eCollisionGroup::None) { continue; }

			if (info.PenetrationDepth > 0.0f)
			{
				DirectX::XMVECTOR v_normal = DirectX::XMLoadFloat3(&info.Normal);
				// yは無視する.
				v_normal = DirectX::XMVectorSetY(v_normal, 0.0f);

				// 補正量 = 法線 * penetrationDepth.
				DirectX::XMVECTOR v_correction = DirectX::XMVectorScale(v_normal, info.PenetrationDepth);

				// y成分を0にする.
				v_correction = DirectX::XMVectorSetY(v_correction, 0.0f);

				DirectX::XMFLOAT3 correction = {};
				DirectX::XMStoreFloat3(&correction, v_correction);

				// 位置を加算して押し返す.
				AddPosition(correction);
			}
		}
	}
}

void Character::ApplyDamage(float damageAmount)
{
	// すでに死亡している場合は何もしない
	if (0.f >= m_HP) return;

	// 無敵時間中ならダメージを受け付けない (必要であれば)
	// if (m_InvincibleTimer > 0.0f) return;

	// HPを減らす
	m_HP -= damageAmount;
	// 修正後：0.0f と m_MaxHP の間に収める
	m_HP = std::clamp(m_HP - damageAmount, 0.0f, m_MaxHP);

	// HPが0以下になったら死亡処理
	if (m_HP <= 0.0f)
	{
		m_HP = 0.0f;

		// 死亡ステートへ（まだ作っていなければログやフラグのみでもOK）
		// ChangeState(PlayerState::eID::Die); 
		Log::GetInstance().Info("Player", "Player is Dead.");
	}
	else
	{
		// 被ダメージ時の共通処理（SE再生や一瞬だけ赤く光らせるフラグなど）

		// 無敵時間の設定 (例: 1.0秒間無敵)
		// m_InvincibleTimer = 1.0f; 
	}
}
