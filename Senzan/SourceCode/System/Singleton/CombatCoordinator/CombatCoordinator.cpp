#include "CombatCoordinator.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/Boss.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossSpecialDamageState/BossSpecialDamageState.h"
#include "System/Singleton/CameraManager/CameraManager.h"
#include "System/Singleton/PostEffectManager/PostEffectManager.h"
#include "Game/02_Camera/LockOnCamera/LockOnCamera.h"
#include "System/Singleton/ResourceManager/SoundManager/SoundManager.h"

CombatCoordinator::CombatCoordinator()
	: m_pPlayer(nullptr)
	, m_pBoss(nullptr)
{
}

void CombatCoordinator::Enter()
{
    if (m_pBoss)
    {
        m_pBoss->GetStateMachine()->ChangeState(std::make_shared<BossSpecialDamageState>(m_pBoss));
    }
}

CombatCoordinator::~CombatCoordinator()
{
	Clear();
}

void CombatCoordinator::Initialize(Player* player, Boss* boss)
{
	m_pPlayer = player;
	m_pBoss = boss;
}

void CombatCoordinator::Clear()
{
	m_pPlayer = nullptr;
	m_pBoss = nullptr;
}

void CombatCoordinator::OnParrySuccess(bool withDelay)
{
    // カメラ演出(シェイク + パリィカメラ).
    CameraManager::GetInstance().ShakeCamera(0.40f, 0.5f);
    CameraManager::GetInstance().StartParryCamera();

    // 円状グレースケールエフェクト開始.
    PostEffectManager::GetInstance().StartCircleGrayEffect(0.0f, 0.016f * 10, 0.016f * 2);

    // BGMのフェードアウト.
    SoundManager::LowerCurrentBGMVolumeTemporarily(6500, 0.2f, 0.4f, 0.4f);

    // Bossにパリィ被弾を通知（パリィステートへ遷移）.
    // withDelay フラグは Boss 側で解釈して遅延挙動を組み込む.
    if (m_pBoss)
    {
        m_pBoss->OnParried(withDelay);
        m_pBoss->Hit(250.f);
    }
}

void CombatCoordinator::NotifyParriedBySnowball()
{
    m_LastParriedBySnowball = true;
}

bool CombatCoordinator::WasLastParriedBySnowball() const
{
    return m_LastParriedBySnowball;
}

void CombatCoordinator::ClearLastParriedFlag()
{
    m_LastParriedBySnowball = false;
}

void CombatCoordinator::HitSpecialAttackToBoss()
{
    if (m_pBoss)
    {
        m_pBoss->OnSpecial();
    }
}

void CombatCoordinator::StartJustDodge(float TimeScale)
{
    if (m_pBoss)
    {
        JustCancelAttackCollider();
        m_pBoss->OffAttackCollider();
        m_pBoss->SetTimeScale(TimeScale);
        m_pBoss->SetLockOnPlayer(false);
    }
}

void CombatCoordinator::StartPlayerJustDodge(float TimeScale)
{
    if (m_pPlayer)
    {
        // Forward to Player: set just dodge timing and start effect
        m_pPlayer->SetIsJustDodgeTiming(true);
        // Player owns the effect; call StartJustDodgeEffect with default params
        m_pPlayer->StartJustDodgeEffect(m_pPlayer->GetPosition(), m_pPlayer->GetPosition(), 1.0f, 1.0f);
        // Also reduce world time scale to create the slow-mo feel
        // Player state may expect Time::SetWorldTimeScale, but CombatCoordinator shouldn't include Time header here;
        // callers can still use CombatCoordinator to coordinate. We set player's internal flag only.
    }
}

void CombatCoordinator::EndPlayerJustDodge()
{
    if (m_pPlayer)
    {
        m_pPlayer->SetIsJustDodgeTiming(false);
    }
}

void CombatCoordinator::CancelPlayerAttackCollider()
{
    if (m_pPlayer)
    {
        m_pPlayer->CancelAttackCollider();
    }
}

void CombatCoordinator::EndJustDodge()
{
    if (m_pBoss)
    {
        m_pBoss->OffAttackCollider();
        m_pBoss->Hit(50.f);
        m_pBoss->SetTimeScale(-1.f);
        m_pBoss->SetLockOnPlayer(true);
    }
}

void CombatCoordinator::DisableBossAttackColliders()
{
    if (m_pBoss)
    {
        m_pBoss->OffAttackCollider();
    }
}

void CombatCoordinator::DamageToBoss(float DamageAmount)
{
    if (m_pBoss)
    {
        m_pBoss->Hit(DamageAmount);
    }
}

bool CombatCoordinator::IsParryCameraFinished() const
{
	auto camera = CameraManager::GetInstance().GetCurrentCamera();
	if (!camera) return true;

	auto lockOnCamera = std::dynamic_pointer_cast<LockOnCamera>(camera);
	if (lockOnCamera)
	{
		return !lockOnCamera->IsParryCameraActive();
	}
	return true;
}

void CombatCoordinator::JustCancelAttackCollider()
{
    if (m_pBoss)
    {
        m_pBoss->SetNextAttackCansel();
    }
}

DirectX::XMFLOAT3 CombatCoordinator::GetBossForward() const
{
    if (m_pBoss)
    {
        return m_pBoss->GetTransform()->GetForward();
    }
    return DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
}

void CombatCoordinator::ForceBossDown()
{
    if (m_pBoss)
    {
        // ボスの現在のステートがBossSpecialDamageStateの場合、Downに遷移させる
        auto currentState = m_pBoss->GetStateMachine()->m_pCurrentState;
        auto specialDamageState = std::dynamic_pointer_cast<BossSpecialDamageState>(currentState);
        if (specialDamageState)
        {
            specialDamageState->ForceTransitionToDown();
        }
    }
}
