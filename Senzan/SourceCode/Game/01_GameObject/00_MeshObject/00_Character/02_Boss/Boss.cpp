#include "Boss.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/BossMoveState/BossMoveState.h"

#include "Game/03_Collision/00_Core/01_Capsule/CapsuleCollider.h"
#include "Game/03_Collision/00_Core/Ex_CompositeCollider/CompositeCollider.h"


#include "BossAttackStateBase/BossAttackStateBase.h"
#include "BossAttackStateBase/BossStompState/BossStompState.h"
#include "BossAttackStateBase/BossSlashState/BossSlashState.h"
#include "BossAttackStateBase/BossShoutState/BossShoutState.h"
#include "BossAttackStateBase/BossMoveContinueState/BossMoveContinueState.h"

#include "System/Utility/StateMachine/StateMachine.h"

#include "BossAttackStateBase/BossJumpOnlState/BossJumpOnlState.h"
#include "BossAttackStateBase/BossParryState/BossParryState.h"

#include "Resource/Mesh/02_Skin/SkinMesh.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/BossDeadState/BossDeadState.h"


#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossThrowingState/BossThrowingState.h"
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossSpinningState/BossSpinningState.h"
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossLaserState/BossLaserState.h"
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossSpecialDamageState/BossSpecialDamageState.h"

#include "System/Singleton/CollisionDetector/CollisionDetector.h"
#include "System/Singleton/CameraManager/CameraManager.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Singleton/ResourceManager/EffectManager/EffekseerManager.h"
#include "Resource/Effect/EffectResource.h"
#include "System/Utility/Math/Math.h"

#include <atomic>
#include <chrono>

constexpr float HP_Max = 10000.0f;

Boss::Boss()
	: Character()
	, m_State(std::make_unique<StateMachine<Boss>>(this))
	, m_PlayerPos{}
	, m_TurnSpeed(0.1f)
	, m_MoveSpeed(3.f)
	, m_vCurrentMoveVelocity(0.f, 0.f, 0.f)
	, deleta_time(0.f)

	, m_pSlashCollider(nullptr)

	, m_pStompCollider(nullptr)

	, m_pShoutCollider(nullptr)
{
	AttachMesh(MeshManager::GetInstance().GetSkinMesh("boss"));

	//DirectX::XMFLOAT3 pos = { 0.05f, 10.0f, 20.05f };
	DirectX::XMFLOAT3 pos = { 0.05f, 0.05f, 20.05f };
	DirectX::XMFLOAT3 scale = { 7.0f,7.0f,7.0f};
	DirectX::XMFLOAT3 Rotation = { 0.0f,0.0f,0.0f };
	m_spTransform->SetPosition(pos);
	m_spTransform->SetScale(scale);
	m_spTransform->SetRotationDegrees(Rotation);

	m_MaxHP = 10000.f;
	m_HP = m_MaxHP;

	// 被ダメの追加.
	std::unique_ptr<CapsuleCollider> damage_collider = std::make_unique<CapsuleCollider>(m_spTransform);

	damage_collider->SetColor(Color::eColor::Yellow);
	damage_collider->SetHeight(30.0f);
	damage_collider->SetRadius(10.0f);
	damage_collider->SetPositionOffset(0.f, 1.5f, 0.f);
	damage_collider->SetAttackAmount(10.f);
	damage_collider->SetMyMask(eCollisionGroup::Enemy_Damage);
	damage_collider->SetTarGetTargetMask(eCollisionGroup::Player_Attack);

	m_upColliders->AddCollider(std::move(damage_collider));

	// プレスの追加.
	std::unique_ptr<CapsuleCollider> press_collider = std::make_unique<CapsuleCollider>(m_spTransform);
	press_collider->SetColor(Color::eColor::Cyan);
	press_collider->SetHeight(20.0f);
	press_collider->SetRadius(5.1f);
	press_collider->SetPositionOffset(0.f, 1.5f, 0.f);
	press_collider->SetMyMask(eCollisionGroup::BossPress);
	press_collider->SetTarGetTargetMask(eCollisionGroup::Press);

	m_upColliders->AddCollider(std::move(press_collider));

	//通常攻撃の当たり判定作成.
	auto slashCol = std::make_unique<CapsuleCollider>(m_spTransform);
	m_pSlashCollider = slashCol.get();
	m_pSlashCollider->SetMyMask(eCollisionGroup::Enemy_Attack);
	m_pSlashCollider->SetTarGetTargetMask(
        eCollisionGroup::Player_Damage
        | eCollisionGroup::Player_Parry_Fai
        | eCollisionGroup::Player_JustDodge);
	m_pSlashCollider->SetAttackAmount(10.0f); 
	m_pSlashCollider->SetHeight(40.0f);         
	m_pSlashCollider->SetRadius(15.0f);         
	m_pSlashCollider->SetPositionOffset(0.0f, 0.0f, 0.0f); 
	m_pSlashCollider->SetActive(false);
	m_pSlashCollider->SetColor(Color::eColor::Red);

	m_upColliders->AddCollider(std::move(slashCol));

    // とびかかりの当たり判定.
	auto stompCol = std::make_unique<CapsuleCollider>(m_spTransform);
	m_pStompCollider = stompCol.get();
	m_pStompCollider->SetMyMask(eCollisionGroup::Enemy_Attack);
	m_pStompCollider->SetTarGetTargetMask(
        eCollisionGroup::Player_Damage
        | eCollisionGroup::Player_Parry_Suc
        | eCollisionGroup::Player_JustDodge);
	m_pStompCollider->SetAttackAmount(5.0f);
	m_pStompCollider->SetRadius(30.0f);
	m_pStompCollider->SetHeight(15.0f);
	m_pStompCollider->SetActive(false);
	m_pStompCollider->SetColor(Color::eColor::Red);
	m_upColliders->AddCollider(std::move(stompCol));

    // 咆哮の当たり判定作成.
    auto Shout_collider = std::make_unique<CapsuleCollider>(m_spTransform);
	m_pShoutCollider = Shout_collider.get(); 
	m_pShoutCollider->SetColor(Color::eColor::Cyan);
	m_pShoutCollider->SetHeight(1.0f);
	m_pShoutCollider->SetRadius(1.0f);
	m_pShoutCollider->SetPositionOffset(0.f, 1.5f, 0.f);
	m_pShoutCollider->SetAttackAmount(100.f);
	m_pShoutCollider->SetMyMask(
        eCollisionGroup::BossPress);
	m_pShoutCollider->SetTarGetTargetMask(
        eCollisionGroup::Player_Damage
        | eCollisionGroup::Press);
	m_pShoutCollider->SetActive(false);
	m_upColliders->AddCollider(std::move(Shout_collider));

    // 回転攻撃の当たり判定作成.
    auto spinning_collider = std::make_unique<CapsuleCollider>(m_spTransform);
	m_pSpinningCollider = spinning_collider.get();
    m_pSpinningCollider->SetMyMask(eCollisionGroup::Enemy_Attack);
    m_pSpinningCollider->SetTarGetTargetMask(
        eCollisionGroup::Player_Damage
        | eCollisionGroup::Player_Parry_Fai
        | eCollisionGroup::Player_JustDodge);
    m_pSpinningCollider->SetAttackAmount(10.0f);
    m_pSpinningCollider->SetHeight(40.0f);
    m_pSpinningCollider->SetRadius(15.0f);
    m_pSpinningCollider->SetPositionOffset(0.0f, 0.0f, 0.0f);
    m_pSpinningCollider->SetActive(false);
    m_pSpinningCollider->SetColor(Color::eColor::Red);
	m_upColliders->AddCollider(std::move(spinning_collider));

    // レーザー攻撃の当たり判定作成.
    auto laser_collider = std::make_unique<CapsuleCollider>(m_spTransform);
	m_pLaserCollider = laser_collider.get();
    m_pLaserCollider->SetMyMask(eCollisionGroup::Enemy_Attack );
    m_pLaserCollider->SetTarGetTargetMask(
        eCollisionGroup::Player_Damage
        | eCollisionGroup::Player_JustDodge
        | eCollisionGroup::Player_Parry_Noc);
    m_pLaserCollider->SetAttackAmount(10.0f);
    m_pLaserCollider->SetHeight(40.0f);
    m_pLaserCollider->SetRadius(15.0f);
    m_pLaserCollider->SetPositionOffset(0.0f, 0.0f, 0.0f);
    m_pLaserCollider->SetActive(false);
    m_pLaserCollider->SetColor(Color::eColor::Red);
	m_upColliders->AddCollider(std::move(laser_collider));

    m_State->ChangeState(std::make_shared<BossShoutState>(this));
    /* BossSlashState
 BossChargeState
 BossChargeSlashState
 BossLaserState
 BossShoutState
 BossSlashState
 BossJumpOnlState
 BossStompState
 BossThrowingState*/
	CollisionDetector::GetInstance().RegisterCollider(*m_upColliders);
}

Boss::~Boss()
{
    if (m_upColliders)
    {
        CollisionDetector::GetInstance().UnregisterCollider(*m_upColliders);
    }
}

void Boss::SetHPMultiplier(float multiplier)
{
    m_MaxHP = 20000.f * multiplier;
    m_HP = m_MaxHP;
}

void Boss::GetParryAnimPair()
{
    if (m_State && m_State->m_pCurrentState)
    {
        // BossAttackStateBase へキャストして設定を取得する
        auto attackBase = std::dynamic_pointer_cast<BossAttackStateBase>(m_State->m_pCurrentState);
        if (attackBase)
        {
            auto pair = attackBase->GetParryAnimPair();
            m_ParryPauseSeconds = pair;
        }
    }
}

void Boss::Update()
{
	Character::Update();

	m_State->Update();

#if _DEBUG
    // デバッグ用: ImGui で任意のボスステートに切り替えられるボタン群
    if (ImGui::Begin(IMGUI_JP("Boss Debug")))
    {
        const char* state_labels[] = {
            IMGUI_JP("Idle"),
            IMGUI_JP("Move"),
            IMGUI_JP("Slash"),
            IMGUI_JP("Shout"),
            IMGUI_JP("Spinning"),
            IMGUI_JP("JumpOn"),
            IMGUI_JP("Stomp"),
            IMGUI_JP("Throwing"),
            IMGUI_JP("Parry"),
            IMGUI_JP("Laser"),
            IMGUI_JP("SpecialDamage")
        };
        constexpr int state_count = static_cast<int>(sizeof(state_labels) / sizeof(state_labels[0]));
        const int buttons_per_row = 4;

        for (int i = 0; i < state_count; ++i)
        {
            if (i > 0 && (i % buttons_per_row) != 0)
            {
                ImGui::SameLine();
            }

            if (ImGui::Button(state_labels[i]))
            {
                switch (i)
                {
                case 0: m_State->ChangeState(std::make_shared<BossIdolState>(this)); break;
                case 1: m_State->ChangeState(std::make_shared<BossMoveState>(this)); break;
                case 2: m_State->ChangeState(std::make_shared<BossSlashState>(this)); break;
                case 3: m_State->ChangeState(std::make_shared<BossShoutState>(this)); break;
                case 4: m_State->ChangeState(std::make_shared<BossSpinningState>(this)); break;
                case 5: m_State->ChangeState(std::make_shared<BossJumpOnlState>(this)); break;
                case 6: m_State->ChangeState(std::make_shared<BossStompState>(this)); break;
                case 7: m_State->ChangeState(std::make_shared<BossThrowingState>(this)); break;
                case 8: m_State->ChangeState(std::make_shared<BossParryState>(this)); break;
                case 9: m_State->ChangeState(std::make_shared<BossLaserState>(this)); break;
                case 10: m_State->ChangeState(std::make_shared<BossSpecialDamageState>(this)); break;
                default: break;
                }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button(IMGUI_JP("Enter Slash (Hotkey)")))
        {
            m_State->ChangeState(std::make_shared<BossSpinningState>(this));
        }

        ImGui::End();
    }
#endif
}

void Boss::LateUpdate()
{
	Character::LateUpdate();

	if (!m_State) {
		return;
	}

	// ステートマシーンの最終更新を実行.
	m_State->LateUpdate();

    // 衝突処理.
    HandleDamageDetection();
    HandleAttackDetection();
}

void Boss::Draw()
{
    Character::Draw();
	m_State->Draw();
}

void Boss::Init()
{
}

void Boss::OnParried()
{
    // 既にパリィ状態なら何もしない.
    if (m_IsParried) return;

    m_IsParried = true;

    // 全ての攻撃コライダーを無効化.
    OffAttackCollider();

    // BossParryStateへ遷移.
    m_State->ChangeState(std::make_shared<BossParryState>(this));
}

void Boss::OnParried(bool withDelay)
{
    // 既にパリィ状態なら何もしない.
    if (m_IsParried) return;
    GetParryAnimPair();

    m_IsParried = true;

    // 全ての攻撃コライダーを無効化 (パリィ成功時も攻撃判定を消す)
    OffAttackCollider();

    // BossParryStateへ遷移（遅延フラグと遅延秒を渡す）
    m_State->ChangeState(std::make_shared<BossParryState>(this, withDelay));
}

void Boss::OnSpecial()
{
    // 全ての攻撃コライダーを無効化.
    OffAttackCollider();

    // BossParryStateへ遷移.
    m_State->ChangeState(std::make_shared<BossParryState>(this));
}

StateMachine<Boss>* Boss::GetStateMachine()
{
	return m_State.get();
}

LPD3DXANIMATIONCONTROLLER Boss::GetAnimCtrl() const
{
	return m_pAnimCtrl;
}

void Boss::Hit(float damage)
{
    m_HP -= damage;
    if (m_HP <= 0.0f)
    {
        //死んだときにDeadStateclassに入る.
        m_State->ChangeState(std::make_shared<BossDeadState>(this));
    }

}

void Boss::SetTargetPos(const DirectX::XMFLOAT3 Player_Pos)
{
	m_PlayerPos = Player_Pos;
}


void Boss::OffAttackCollider() {
    m_pSlashCollider->SetActive(false);
    m_pStompCollider->SetActive(false);
    m_pShoutCollider->SetActive(false);
    m_pSpinningCollider->SetActive(false);
    m_pLaserCollider->SetActive(false);
}

void Boss::SetNextAttackCansel()
{
    if (m_State && m_State->m_pCurrentState)
    {
        // BossAttackStateBase へキャストして設定を取得する
        auto attackBase = std::dynamic_pointer_cast<BossAttackStateBase>(m_State->m_pCurrentState);
        if (attackBase)
        {
            attackBase->SetNextAttackCansel();
            return;
        }
    }
}

// 衝突_被ダメージ.
void Boss::HandleDamageDetection()
{
	if (!m_upColliders) return;

	const auto& internal_colliders = m_upColliders->GetInternalColliders();

	for (const auto& collider_ptr : internal_colliders)
	{
		const ColliderBase* current_collider = collider_ptr.get();

		if ((current_collider->GetMyMask() & eCollisionGroup::Enemy_Damage) == eCollisionGroup::None) {
			continue;
		}

		for (const CollisionInfo& info : current_collider->GetCollisionEvents())
		{
			if (!info.IsHit) continue;
			const ColliderBase* otherCollider = info.ColliderB;
			if (!otherCollider) { continue; }

			eCollisionGroup other_group = otherCollider->GetMyMask();

			if ((other_group & eCollisionGroup::Player_Attack) != eCollisionGroup::None)
			{
                bool IsEffectBack = false;

					// プレイヤーがボスの後方（ボスの前方ベクトルとの内積が負）から攻撃していたらダメージを1.4倍にする
					float damageToApply = info.AttackAmount;
					
                    // ボス中心からプレイヤー方向ベクトルを計算（Y成分は無視）
                    DirectX::XMFLOAT3 bossPos = GetPosition();
                    DirectX::XMFLOAT3 playerPos = otherCollider->GetPosition();
                    DirectX::XMVECTOR vBossToPlayer = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&playerPos), DirectX::XMLoadFloat3(&bossPos));
                    vBossToPlayer = DirectX::XMVectorSetY(vBossToPlayer, 0.0f);
                    if (!DirectX::XMVector3NearEqual(vBossToPlayer, DirectX::XMVectorZero(), DirectX::XMVectorReplicate(1e-6f)))
                    {
                        vBossToPlayer = DirectX::XMVector3Normalize(vBossToPlayer);
                        // ボスの前方ベクトル
                        DirectX::XMFLOAT3 fwd = m_spTransform->GetForward();
                        DirectX::XMVECTOR vFwd = DirectX::XMLoadFloat3(&fwd);
                        vFwd = DirectX::XMVectorSetY(vFwd, 0.0f);
                        vFwd = DirectX::XMVector3Normalize(vFwd);
                        float dot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(vFwd, vBossToPlayer));
                        // 内積が負なら後方からの攻撃
                        if (dot < 0.0f)
                        {
                            SoundManager::Play("Damage_Back");
                            SoundManager::SetVolume("Damage_Back", 9000);
                        }
                        else
                        {
                            damageToApply *= 1.4f;
                            SoundManager::Play("Damage");
                            SoundManager::SetVolume("Damage", 9000);
                            IsEffectBack = true;
                        }
                    }

                    static thread_local std::mt19937 s_rng((std::random_device())());
                    for (int i = 0; i < 3; ++i)
                    {
                        // ランダムオフセット.
                        std::uniform_real_distribution<float> dist(-1.2f, 1.2f);
                        std::uniform_real_distribution<float> rotDist(0.0f, DirectX::XM_2PI);

                        DirectX::XMFLOAT3 jitterPos{
                            info.ContactPoint.x + dist(s_rng),
                            info.ContactPoint.y + 1.5f + dist(s_rng),
                            info.ContactPoint.z + dist(s_rng)
                        };

                        // ランダム回転.
                        DirectX::XMFLOAT3 eulerRot{ rotDist(s_rng), rotDist(s_rng), rotDist(s_rng) };
                        if (IsEffectBack)
                        {
                            PlayEffectAtWorldPos("Hit2_Back", jitterPos, eulerRot);
                        }
                        else {
                            PlayEffectAtWorldPos("Hit2", jitterPos, eulerRot);                   
                        }
                    }
                    ApplyDamage(damageToApply);

				Time::GetInstance().SetWorldTimeScale(0.1f, 0.016f * 5);
				CameraManager::GetInstance().ShakeCamera(0.1f, 2.5f); // カメラを少し揺らす.

				// 1フレームに1回.
				return;
			}
		}
	}
}

void Boss::HandleAttackDetection()
{
	if (!m_upColliders) return;

	const auto& internal_colliders = m_upColliders->GetInternalColliders();

	for (const auto& collider_ptr : internal_colliders)
	{
		const ColliderBase* current_collider = collider_ptr.get();

		if ((current_collider->GetMyMask() & eCollisionGroup::Enemy_Attack) == eCollisionGroup::None) {
			continue;
		}

		for (const CollisionInfo& info : current_collider->GetCollisionEvents())
		{
			if (!info.IsHit) continue;
			const ColliderBase* otherCollider = info.ColliderB;
			if (!otherCollider) { continue; }

			eCollisionGroup other_group = otherCollider->GetMyMask();

			if ((other_group & eCollisionGroup::Player_Damage) != eCollisionGroup::None)
			{
                OffAttackCollider();
                SoundManager::Play("Damage");
                SoundManager::SetVolume("Damage", 9000);

				// 1フレームに1回.
				return;
			}
		}
	}
}

ColliderBase* Boss::GetSlashCollider() const
{
	return m_pSlashCollider;
}

ColliderBase* Boss::GetStompCollider() const
{
	return m_pStompCollider;
}

ColliderBase* Boss::GetShoutCollider() const
{
	return m_pShoutCollider;
}

ColliderBase* Boss::GetSpinningCollider() const
{
    return m_pSpinningCollider;
}

ColliderBase* Boss::GetLaserCollider() const
{
    return m_pLaserCollider;
}

void Boss::SetColliderActiveByName(const std::string& name, bool active)
{
	// NOTE: 文字列は typo を避けるため定数化推奨
	if (name == "boss_Hand_R")
	{
		if (auto* col = GetSlashCollider()) col->SetActive(active);
		return;
	}
	if (name == "boss_pSphere28")
	{
		if (auto* col = GetStompCollider()) col->SetActive(active);
		return;
	}
	if (name == "boss_Shout")
	{
		if (auto* col = GetShoutCollider()) col->SetActive(active);
		return;
	}
    if (name == "boss_Spinning")
    {
        if (auto* col = GetSpinningCollider()) col->SetActive(active);
        return;
    }
    if (name == "boss_Laser")
    {
        if (auto* col = GetLaserCollider()) col->SetActive(active);
        return;
    }
}

bool Boss::UpdateColliderFromBone(
    const std::string& boneName,
    ColliderBase* collider,
    Transform& outTransform,
    bool updateRotation,
    const DirectX::XMFLOAT4& rotationOffset)
{
    if (!collider || GetAttachMesh().expired()) return false;
    auto skinMesh = std::dynamic_pointer_cast<SkinMesh>(GetAttachMesh().lock());
    if (!skinMesh) return false;

    DirectX::XMMATRIX bone_local_matrix;
    if (!skinMesh->GetMatrixFromBone(boneName.c_str(), &bone_local_matrix))
    {
        DirectX::XMFLOAT3 boneWorldPos{};
        if (!skinMesh->GetPosFromBone(boneName.c_str(), &boneWorldPos)) return false;
        outTransform.Position = boneWorldPos;
        DirectX::XMMATRIX bossWorldMatrix = m_spTransform->GetWorldMatrix();
        if (updateRotation) {
            DirectX::XMVECTOR b_pos, b_quat, b_scale;
            DirectX::XMMatrixDecompose(&b_scale, &b_quat, &b_pos, bossWorldMatrix);
            // apply rotation offset if provided
            DirectX::XMVECTOR q_bone = b_quat;
            DirectX::XMVECTOR q_offset = DirectX::XMLoadFloat4(&rotationOffset);
            DirectX::XMVECTOR q_result = DirectX::XMQuaternionMultiply(q_offset, q_bone);
            DirectX::XMStoreFloat4(&outTransform.Quaternion, q_result);
            DirectX::XMStoreFloat3(&outTransform.Scale, b_scale);
            outTransform.UpdateRotationFromQuaternion();
        } else {
            outTransform.Quaternion = DirectX::XMFLOAT4{0.0f, 0.0f, 0.0f, 1.0f};
            outTransform.Scale = DirectX::XMFLOAT3{1.0f, 1.0f, 1.0f};
        }
        return true;
    }

    DirectX::XMMATRIX boss_world_matrix = m_spTransform->GetWorldMatrix();
    DirectX::XMMATRIX bone_world_matrix = bone_local_matrix * boss_world_matrix;

    DirectX::XMVECTOR v_final_pos, v_final_quat, v_final_scale;
    DirectX::XMMatrixDecompose(&v_final_scale, &v_final_quat, &v_final_pos, bone_world_matrix);
    DirectX::XMStoreFloat3(&outTransform.Position, v_final_pos);
    if (updateRotation) {
        // apply rotation offset
        DirectX::XMVECTOR q_bone = v_final_quat;
        DirectX::XMVECTOR q_offset = DirectX::XMLoadFloat4(&rotationOffset);
        DirectX::XMVECTOR q_result = DirectX::XMQuaternionMultiply(q_offset, q_bone);
        DirectX::XMStoreFloat4(&outTransform.Quaternion, q_result);
        DirectX::XMStoreFloat3(&outTransform.Scale, v_final_scale);
        outTransform.UpdateRotationFromQuaternion();
    }

    // オフセットは各ステートの ColliderWindow で管理
    return true;
}
