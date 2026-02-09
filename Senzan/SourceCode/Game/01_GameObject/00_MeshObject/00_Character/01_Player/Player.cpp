#include "Player.h"
#include "System/Utility/StateMachine/StateMachine.h"

#include "State/PlayerStateID.h"
#include "State/PlayerStateBase.h"

#include "State/Root/Root.h"

#include "State/Root/00_System/System.h"
#include "State/Root/00_System/00_Pause/Pause.h"
#include "State/Root/00_System/01_KnockBack/KnockBack.h"
#include "State/Root/00_System/02_Dead/Dead.h"
#include "State/Root/00_System/03_SpecialAttack/SpecialAttack.h"


#include "State/Root/01_Action/Action.h"
#include "State/Root/01_Action/00_Movement/Movement.h"
#include "State/Root/01_Action/00_Movement/00_Idle/Idle.h"
#include "State/Root/01_Action/00_Movement/01_Run/Run.h"

#include "State/Root/01_Action/01_Combat/Combat.h"
#include "State/Root/01_Action/01_Combat/00_AttackCombo_0/AttackCombo_0.h"
#include "State/Root/01_Action/01_Combat/01_AttackCombo_1/AttackCombo_1.h"
#include "State/Root/01_Action/01_Combat/02_AttackCombo_2/AttackCombo_2.h"

#include "State/Root/01_Action/02_Dodge/Dodge.h"
#include "State/Root/01_Action/02_Dodge/00_DodgeExecute/DodgeExecute.h"
#include "State/Root/01_Action/02_Dodge/01_JustDodge/JustDodge.h"
#include "State/Root/01_Action/02_Dodge/01_JustDodge/JustDodgeEffect.h"

#include "Game/03_Collision/00_Core/01_Capsule/CapsuleCollider.h"
#include "Game/03_Collision/00_Core/Ex_CompositeCollider/CompositeCollider.h"

#include "Game/04_Time/Time.h"
#include "Game/05_InputDevice/Input.h"

#include "System/Singleton/CollisionDetector/CollisionDetector.h"
#include "System/Singleton/CameraManager/CameraManager.h"
#include "System/Singleton/CombatCoordinator/CombatCoordinator.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Resource/Effect/EffectResource.h"
#include <random>

//#if _DEBUG
#include "System/Singleton/ImGui/CImGuiManager.h"
//#endif

Player::Player()
	: Character         ()
	, m_RootState       ( std::make_unique<PlayerState::Root>(this) )
	, m_StateRefMap     ( )
	, m_NextStateID     ( PlayerState::eID::None )
	, m_IsStateChangeRequest    ( false )
	, m_MoveVec         ( { 0.f,0.f,0.f } )
	, m_Combo           ( 0 )
	, m_CurrentUltValue ( 0.0f )
	, m_MaxUltValue     ( 10000.0f )
	, m_IsKnockBack     ( false )
	, m_KnockBackVec    ( { 0.f,0.f,0.f } )
	, m_KnockBackPower  ( 0.f )
	, m_IsDead          ( false )
	, m_RunMoveSpeed    ( 50.f )
	, m_IsSuccessParry  ( false )
	, m_pAttackCollider ( nullptr )
	, m_pPressCollider  ( nullptr )
	, m_IsJustDodgeTiming( false )
	, m_TargetPos        ( { 0.f,0.f,0.f } )
	, m_DebugForcedState ( PlayerState::eID::None )
	, m_DebugRepeatOnExit( false )
	, m_DebugWasInForcedState( false )
{
	// ステートの初期化.
	InitializeStateRefMap();

	// メッシュのアタッチ.
	auto mesh = ResourceManager::GetSkinMesh("player");
	_ASSERT_EXPR(mesh != nullptr, "メッシュの取得に失敗");
	AttachMesh(mesh);

	//デバック確認のため.
	DirectX::XMFLOAT3 pos = { 0.f, 0.f, -20.f };
	m_spTransform->SetPosition(pos);

	DirectX::XMFLOAT3 scale = { 3.f, 3.f, 3.f };
	m_spTransform->SetScale(scale);

	m_MaxHP = 100.f;
	m_HP = m_MaxHP;

	// 被ダメの追加.
	std::unique_ptr<CapsuleCollider> damage_collider
        = std::make_unique<CapsuleCollider>(m_spTransform);

	m_pDamageCollider = damage_collider.get();

	damage_collider->SetColor(Color::eColor::Yellow);
	damage_collider->SetHeight(2.0f);
	damage_collider->SetRadius(0.5f);
	damage_collider->SetPositionOffset(0.f, 1.5f, 0.f);
	damage_collider->SetMyMask(eCollisionGroup::Player_Damage);
	damage_collider->SetTarGetTargetMask(eCollisionGroup::Enemy_Attack);

	m_upColliders->AddCollider(std::move(damage_collider));

	// パリィの追加.
	std::unique_ptr<CapsuleCollider> parry_collider = std::make_unique<CapsuleCollider>(m_spTransform);

	m_pParryCollider = parry_collider.get();

	parry_collider->SetActive(false);
	parry_collider->SetColor(Color::eColor::Green);
	parry_collider->SetHeight(2.0f);
	parry_collider->SetRadius(0.5f);
	parry_collider->SetPositionOffset(0.f, 1.5f, 0.f);
	parry_collider->SetMyMask(eCollisionGroup::Player_Parry_Fai | eCollisionGroup::Player_Parry_Suc| eCollisionGroup::Player_Parry_Noc);
	parry_collider->SetTarGetTargetMask(eCollisionGroup::Enemy_Attack);
	m_upColliders->AddCollider(std::move(parry_collider));

	// ジャスト回避の追加.
	std::unique_ptr<CapsuleCollider> justdodge_collider = std::make_unique<CapsuleCollider>(m_spTransform);

	justdodge_collider->SetColor(Color::eColor::Gray);
	justdodge_collider->SetHeight(45.0f);
	justdodge_collider->SetRadius(45.0f);
	justdodge_collider->SetPositionOffset(0.f, 1.5f, 0.f);
	justdodge_collider->SetMyMask(eCollisionGroup::Player_JustDodge);
	justdodge_collider->SetTarGetTargetMask(eCollisionGroup::Enemy_Attack);

	m_upColliders->AddCollider(std::move(justdodge_collider));

	// 押し戻しの追加.
	std::unique_ptr<CapsuleCollider> pressCollider = std::make_unique<CapsuleCollider>(m_spTransform);

	m_pPressCollider = pressCollider.get();  // ポインタを保存

	pressCollider->SetColor(Color::eColor::Cyan);
	pressCollider->SetHeight(3.0f);
	pressCollider->SetRadius(1.0f);
	pressCollider->SetPositionOffset(0.f, 1.5f, 0.f);
	pressCollider->SetMyMask(eCollisionGroup::Press);
	pressCollider->SetTarGetTargetMask(eCollisionGroup::BossPress);

	m_upColliders->AddCollider(std::move(pressCollider));

	// 攻撃の追加.
	std::unique_ptr<CapsuleCollider> attackCollider = std::make_unique<CapsuleCollider>(m_spTransform);

	m_pAttackCollider = attackCollider.get();

	attackCollider->SetActive(false);
	attackCollider->SetColor(Color::eColor::Red);
	attackCollider->SetAttackAmount(25.0f);
	attackCollider->SetHeight(3.0f);
	attackCollider->SetRadius(1.0f);
	attackCollider->SetPositionOffset(0.f, 1.5f, 2.f);
	attackCollider->SetMyMask(eCollisionGroup::Player_Attack);
	attackCollider->SetTarGetTargetMask(eCollisionGroup::Enemy_Damage);

	m_upColliders->AddCollider(std::move(attackCollider));

	CollisionDetector::GetInstance().RegisterCollider(*m_upColliders);

	// 各ステートの初期化.
	m_RootState.get()->Enter();
}

Player::~Player()
{
    if (m_upColliders)
    {
        CollisionDetector::GetInstance().UnregisterCollider(*m_upColliders);
    }
}

void Player::Update()
{
	Character::Update();

	// ステート遷移のチェック.
	if (m_NextStateID != PlayerState::eID::None)
	{
		m_RootState->ChangeState(m_NextStateID);
		m_NextStateID = PlayerState::eID::None;
	}
	if (!m_RootState) {
		return;
	}
	m_RootState->Update();

	m_IsJustDodgeTiming = false;

    EffekseerManager::GetInstance().GetManager()->Update();

    // ジャスト回避エフェクトの更新（ステートに依存しない）
    if (m_pJustDodgeEffect && m_pJustDodgeEffect->IsPlaying())
    {
        m_pJustDodgeEffect->Update(GetDelta());
    }

#if _DEBUG
    ImGui::Begin("PlayerStateDebug");
    if (ImGui::Button("SpecialCharge")) {
        m_CurrentUltValue = m_MaxUltValue;
    }
    ImGui::End();
#endif

}

void Player::LateUpdate()
{
	Character::LateUpdate();

	if (!m_RootState) {
		return;
	}

	// ステートマシーンの最終更新を実行.
	m_RootState->LateUpdate();

	// 押し戻し.
	HandleCollisionResponse();

    // 衝突イベント処理を実行
    if (IsSpecial()){return;}

    HandleParry_NocDetection();
    HandleParry_SuccessDetection();
    HandleParry_FailDetection();
	HandleDamageDetection();
	HandleAttackDetection();
	HandleDodgeDetection();
}

void Player::Draw()
{
    // モデルの関係で前後反転.
    // X/Z 軸の回転が描画時に変わらないよう、元の回転を保持して
    // 一時的に Y 軸のみを変更して描画を行い、その後に復元する。
    DirectX::XMFLOAT3 originalRot = m_spTransform->Rotation;
    DirectX::XMFLOAT3 tempRot = originalRot;
    tempRot.x = 0.0f; // X軸回転を無効化
    tempRot.z = 0.0f; // Z軸回転を無効化
    tempRot.y = originalRot.y + D3DXToRadian(180.0f);
    m_spTransform->SetRotation(tempRot);

    m_RootState->Draw();
    Character::Draw();

    // ジャスト回避エフェクトの描画
    if (m_pJustDodgeEffect && m_pJustDodgeEffect->IsPlaying())
    {
        m_pJustDodgeEffect->Draw();
    }

    // 回転を元に戻す
    m_spTransform->SetRotation(originalRot);
}

// ノック中か.
bool Player::IsKnockBack() const noexcept
{
	return m_IsKnockBack;
}

// 死亡中か.
bool Player::IsDead() const noexcept
{
	return m_IsDead;
}

bool Player::IsParry() const noexcept
{
	return m_IsSuccessParry;
}

// ポーズ中か.
bool Player::IsPaused() const noexcept
{
	return false;
}

bool Player::IsSpecial() const noexcept
{
    return m_IsSpecial;
}

// ステートの変更.
void Player::ChangeState(PlayerState::eID id)
{
    // 遷移ステートがより優先度の高いほうを設定.
    auto IsSystemState = [](PlayerState::eID s) {
        return s == PlayerState::eID::Pause ||
               s == PlayerState::eID::KnockBack ||
               s == PlayerState::eID::Dead ||
               s == PlayerState::eID::SpecialAttack;
    };

    // まだ予約がない場合はそのままセット
    if (m_NextStateID == PlayerState::eID::None)
    {
        m_NextStateID = id;
        return;
    }

    // 新しい遷移がSystem側なら優先して上書き
    if (IsSystemState(id))
    {
        m_NextStateID = id;
        return;
    }

    // 既に予約されている遷移がSystem側ならそれを優先（上書きしない）
    if (IsSystemState(m_NextStateID))
    {
        return;
    }

    // それ以外は新しい遷移で上書きする
    m_NextStateID = id;
}

std::reference_wrapper<PlayerStateBase> Player::GetStateReference(PlayerState::eID id)
{
	try
	{
		auto it = m_StateRefMap.find(id);

		if (it == m_StateRefMap.end())
		{
			throw std::out_of_range("PlayerStateID not mapped (ID: " + std::to_string(static_cast<int>(id)) + ")");
		}

		return it->second();
	}
	catch (const std::exception& e)
	{
		std::string message = "Player::GetStateReference failed (ID: " + std::to_string(static_cast<int>(id)) + "): " + e.what();
		//Debug::Warning(message.c_str());
		throw;
	}
}

// マッピングを初期化するヘルパー関数.
void Player::InitializeStateRefMap()
{
	// --- Systemステート ---.
	m_StateRefMap[PlayerState::eID::Pause]          = [this] { return m_RootState->GetPauseStateRef(); };
	m_StateRefMap[PlayerState::eID::KnockBack]      = [this] { return m_RootState->GetKnockBackStateRef(); };
	m_StateRefMap[PlayerState::eID::Dead]           = [this] { return m_RootState->GetDeadStateRef(); };
	m_StateRefMap[PlayerState::eID::SpecialAttack]  = [this] { return m_RootState->GetSpecialAttackStateRef(); };

	// --- Movementステート ---.
	m_StateRefMap[PlayerState::eID::Idle]           = [this] { return m_RootState->GetIdleStateRef(); };
	m_StateRefMap[PlayerState::eID::Run]            = [this] { return m_RootState->GetRunStateRef(); };

	// --- Dodgeステート ---.
	m_StateRefMap[PlayerState::eID::DodgeExecute]   = [this] { return m_RootState->GetDodgeExecuteStateRef(); };
	m_StateRefMap[PlayerState::eID::JustDodge]      = [this] { return m_RootState->GetJustDodgeStateRef(); };

	// --- Combatステート ---.
	m_StateRefMap[PlayerState::eID::AttackCombo_0]  = [this] { return m_RootState->GetCombo0StateRef(); };
	m_StateRefMap[PlayerState::eID::AttackCombo_1]  = [this] { return m_RootState->GetCombo1StateRef(); };
	m_StateRefMap[PlayerState::eID::AttackCombo_2]  = [this] { return m_RootState->GetCombo2StateRef(); };
	m_StateRefMap[PlayerState::eID::Parry]          = [this] { return m_RootState->GetParryStateRef(); };
}

// 衝突_被ダメージ.
void Player::HandleDamageDetection()
{
	if (!m_upColliders) return;

	const auto& internal_colliders = m_upColliders->GetInternalColliders();

	for (const auto& collider_ptr : internal_colliders)
	{
		const ColliderBase* current_collider = collider_ptr.get();

		if ((current_collider->GetMyMask() & eCollisionGroup::Player_Damage) == eCollisionGroup::None) {
			continue;
		}

		for (const CollisionInfo& info : current_collider->GetCollisionEvents())
		{
			if (!info.IsHit) continue;
			const ColliderBase* otherCollider = info.ColliderB;
			if (!otherCollider) { continue; }

			eCollisionGroup other_group = otherCollider->GetMyMask();

			if ((other_group & eCollisionGroup::Enemy_Attack) != eCollisionGroup::None)
			{
				SoundManager::GetInstance().Play("Damage");
				SoundManager::GetInstance().SetVolume("Damage",7000);

				// 既にスタン中や無敵時間であれば処理を中断
				if (IsKnockBack() || IsDead()) { continue; }

				m_Combo = 0;

				// ダメージを適用 
				ApplyDamage(info.AttackAmount);

				// ボスからPlayerへのベクトルを計算.
				DirectX::XMFLOAT3 bossPos = m_TargetPos;
				DirectX::XMFLOAT3 playerPos = GetPosition();
				DirectX::XMVECTOR vBossToPlayer = DirectX::XMVectorSubtract(
					DirectX::XMLoadFloat3(&playerPos),
					DirectX::XMLoadFloat3(&bossPos)
				);
				vBossToPlayer = DirectX::XMVector3Normalize(vBossToPlayer);
				DirectX::XMStoreFloat3(&m_KnockBackVec, vBossToPlayer);

				m_KnockBackPower = 40.f;

				// 状態をノックバックに遷移させる
				ChangeState(PlayerState::eID::KnockBack);

				CameraManager::GetInstance().ShakeCamera(0.5f, 4.5f); // カメラを少し揺らす.

				// 1フレームに1回.
				return;
			}
		}
	}
}

void Player::HandleAttackDetection()
{
	if (!m_upColliders) return;

	const auto& internal_colliders = m_upColliders->GetInternalColliders();

	for (const auto& collider_ptr : internal_colliders)
	{
		const ColliderBase* current_collider = collider_ptr.get();

		if ((current_collider->GetMyMask() & eCollisionGroup::Player_Attack) == eCollisionGroup::None) {
			continue;
		}

		for (const CollisionInfo& info : current_collider->GetCollisionEvents())
		{
			if (!info.IsHit) continue;
			const ColliderBase* otherCollider = info.ColliderB;
			if (!otherCollider) { continue; }

			eCollisionGroup other_group = otherCollider->GetMyMask();

            if ((other_group & eCollisionGroup::Enemy_Damage) != eCollisionGroup::None)
            {
                static thread_local std::mt19937 s_rng((std::random_device())());
               
                SoundManager::GetInstance().Play("Hit1");
                SoundManager::GetInstance().SetVolume("Hit1", 9000);

                float upper = 6.0f * (static_cast<float>(m_Combo) / 100.0f);
                if (upper < 3.0f) upper = 3.0f; 

                std::uniform_real_distribution<float> randomUp(3.0f, upper);
                m_Combo += static_cast<int>(randomUp(s_rng));
                m_CurrentUltValue = std::clamp(m_CurrentUltValue + static_cast<float>(m_Combo), 0.0f, m_MaxUltValue);
                SetAttackColliderActive(false);

                // 一フレーム1回.
                return;
            }
		}
	}
}

void Player::HandleDodgeDetection()
{
	if (!m_upColliders) return;

	const auto& internal_colliders = m_upColliders->GetInternalColliders();

	for (const auto& collider_ptr : internal_colliders)
	{
		const ColliderBase* current_collider = collider_ptr.get();

		if ((current_collider->GetMyMask() & eCollisionGroup::Player_JustDodge) == eCollisionGroup::None) {
			continue;
		}

		for (const CollisionInfo& info : current_collider->GetCollisionEvents())
		{

            if (!info.IsHit) continue;
            const ColliderBase* otherCollider = info.ColliderB;
            if (!otherCollider) { continue; }

            eCollisionGroup other_group = otherCollider->GetMyMask();

            if ((other_group & eCollisionGroup::Enemy_Attack) != eCollisionGroup::None)
            {
                // ジャスト回避タイミング.
                m_IsJustDodgeTiming = true;
            }
		}
	}
}

void Player::HandleParry_SuccessDetection()
{
	if (!m_upColliders) return;

	const auto& internal_colliders = m_upColliders->GetInternalColliders();

	for (const auto& collider_ptr : internal_colliders)
	{
		const ColliderBase* current_collider = collider_ptr.get();

		if ((current_collider->GetMyMask() & eCollisionGroup::Player_Parry_Suc) == eCollisionGroup::None) {
			continue;
		}

		for (const CollisionInfo& info : current_collider->GetCollisionEvents())
		{
			if (!info.IsHit) continue;
			const ColliderBase* otherCollider = info.ColliderB;
			if (!otherCollider) { continue; }

			eCollisionGroup other_group = otherCollider->GetMyMask();
            eCollisionGroup other_target_group = otherCollider->GetTargetMask();

			if ((other_group & eCollisionGroup::Enemy_Attack) != eCollisionGroup::None
                && (other_target_group & eCollisionGroup::Player_Parry_Suc) != eCollisionGroup::None)
			{
				SoundManager::GetInstance().Play("Parry");
				SoundManager::GetInstance().SetVolume("Parry",9000);
				m_IsSuccessParry = true;
				
				// パリィ成功時のゲージ増加
				m_CurrentUltValue += 100.0f * ComboMultiplier();

				// パリィ成功時のカメラ演出（シェイク）
				CameraManager::GetInstance().ShakeCamera(0.15f, 0.3f);

                CombatCoordinator::GetInstance().OnParrySuccess(true);

            // パリィエフェクトを衝突点に出す（小さなランダムオフセットを追加）
                for(int i = 0 ; i < 3; ++i)
            {
                static thread_local std::mt19937 s_rng((std::random_device())());
                std::uniform_real_distribution<float> dist(-1.15f, 1.15f);
                std::uniform_real_distribution<float> rotDist(0.0f, DirectX::XM_2PI);
                DirectX::XMFLOAT3 jitterPos{
                    info.ContactPoint.x + dist(s_rng),
                    info.ContactPoint.y + 3.5f+ dist(s_rng),
                    info.ContactPoint.z + dist(s_rng)
                };
                DirectX::XMFLOAT3 eulerRot{ rotDist(s_rng), rotDist(s_rng), rotDist(s_rng) };
                PlayEffectAtWorldPos("Spark", jitterPos, eulerRot, 3.f);
            }
				// 一フレーム1回.
				return;
			}
		}
	}
}


void Player::HandleParry_FailDetection()
{
	if (!m_upColliders) return;

	const auto& internal_colliders = m_upColliders->GetInternalColliders();

	for (const auto& collider_ptr : internal_colliders)
	{
		const ColliderBase* current_collider = collider_ptr.get();

		if ((current_collider->GetMyMask() & eCollisionGroup::Player_Parry_Fai) == eCollisionGroup::None) {
			continue;
		}

		for (const CollisionInfo& info : current_collider->GetCollisionEvents())
		{
			if (!info.IsHit) continue;
			const ColliderBase* otherCollider = info.ColliderB;
			if (!otherCollider) { continue; }

			eCollisionGroup other_group = otherCollider->GetMyMask();
			eCollisionGroup other_target_group = otherCollider->GetTargetMask();

			if ((other_group & eCollisionGroup::Enemy_Attack) != eCollisionGroup::None
             && (other_target_group & eCollisionGroup::Player_Parry_Fai) != eCollisionGroup::None)
			{
				SoundManager::GetInstance().Play("Parry");
				SoundManager::GetInstance().SetVolume("Parry",7000);
				m_IsSuccessParry = true;

                // パリィ成功時のゲージ増加
                m_CurrentUltValue += 100.0f * ComboMultiplier();

				// パリィ成功時のカメラ演出（シェイク）
				CameraManager::GetInstance().ShakeCamera(0.15f, 0.3f);
			    // Boss に通知（アニメ再生のみ）
			    CombatCoordinator::GetInstance().OnParrySuccess(false);

                // パリィエフェクトを衝突点に出す（ランダム回転）
                {
                    static thread_local std::mt19937 s_rng((std::random_device())());
                    std::uniform_real_distribution<float> rotDist(0.0f, DirectX::XM_2PI);
                    DirectX::XMFLOAT3 eulerRot{ rotDist(s_rng), rotDist(s_rng), rotDist(s_rng) };
                    PlayEffectAtWorldPos("Spark", info.ContactPoint, eulerRot);
                }

			// 一フレーム1回.
			return;
			}
		}
	}
}

void Player::HandleParry_NocDetection()
{
    if (!m_upColliders) return;

    const auto& internal_colliders = m_upColliders->GetInternalColliders();

    for (const auto& collider_ptr : internal_colliders)
    {
        const ColliderBase* current_collider = collider_ptr.get();

        if ((current_collider->GetMyMask() & eCollisionGroup::Player_Parry_Noc) == eCollisionGroup::None) {
            continue;
        }

        for (const CollisionInfo& info : current_collider->GetCollisionEvents())
        {
            if (!info.IsHit) continue;
            const ColliderBase* otherCollider = info.ColliderB;
            if (!otherCollider) { continue; }

            eCollisionGroup other_group = otherCollider->GetMyMask();
            eCollisionGroup other_target_group = otherCollider->GetTargetMask();

            if ((other_group & eCollisionGroup::Enemy_Attack) != eCollisionGroup::None
                && (other_target_group & eCollisionGroup::Player_Parry_Noc) != eCollisionGroup::None)
            {
                SoundManager::GetInstance().Play("Damage");
                SoundManager::GetInstance().SetVolume("Damage", 7000);

                // 既にスタン中や無敵時間であれば処理を中断
                if (IsKnockBack() || IsDead()) { continue; }

                m_Combo = 0;

                // ダメージを適用 
                ApplyDamage(info.AttackAmount);

                // ボスからPlayerへのベクトルを計算.
                DirectX::XMFLOAT3 bossPos = m_TargetPos;
                DirectX::XMFLOAT3 playerPos = GetPosition();
                DirectX::XMVECTOR vBossToPlayer = DirectX::XMVectorSubtract(
                    DirectX::XMLoadFloat3(&playerPos),
                    DirectX::XMLoadFloat3(&bossPos)
                );
                vBossToPlayer = DirectX::XMVector3Normalize(vBossToPlayer);
                DirectX::XMStoreFloat3(&m_KnockBackVec, vBossToPlayer);

                m_KnockBackPower = 40.f;

                // 状態をノックバックに遷移させる
                ChangeState(PlayerState::eID::KnockBack);

                CameraManager::GetInstance().ShakeCamera(0.5f, 4.5f); // カメラを少し揺らす.

                // 1フレームに1回.
                return;
            }
        }
    }
}

float Player::ComboMultiplier() const noexcept
{
    float handledCombo = static_cast<float>(m_Combo) / 100.f; 

    return 1.f + (handledCombo * 0.05f);
}

void Player::StartJustDodgeEffect(const DirectX::XMFLOAT3& startPos, const DirectX::XMFLOAT3& targetPos, float scale, float duration, float extraDistance)
{
    if (!m_pJustDodgeEffect)
    {
        m_pJustDodgeEffect = std::make_unique<JustDodgeEffect>();
    }
    // Extend the target position in the boss forward direction by extraDistance (if provided)
    DirectX::XMFLOAT3 finalTarget = targetPos;
    if (extraDistance != 0.0f)
    {
        // compute direction from start to target (XZ) and extend
        float dx = targetPos.x - startPos.x;
        float dz = targetPos.z - startPos.z;
        float len = sqrtf(dx*dx + dz*dz);
        if (len > 1e-6f)
        {
            float nx = dx / len;
            float nz = dz / len;
            finalTarget.x += nx * extraDistance;
            finalTarget.z += nz * extraDistance;
        }
    }
    m_pJustDodgeEffect->Start("JustDodge_Attack", startPos, finalTarget, scale, duration);
}

void Player::SetSpecial(bool enable)
{
    m_IsSpecial = enable;
}
