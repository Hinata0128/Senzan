#include "BossSlashState.h"

#include "Game//04_Time//Time.h"
#include "Game//01_GameObject//00_MeshObject//00_Character//02_Boss//Boss.h"
#include "Game//01_GameObject//00_MeshObject//00_Character//01_Player//Player.h"
#include "..//..//BossMoveState//BossMoveState.h" // 攻撃後の遷移先
#include "Resource\\Mesh\\02_Skin\\SkinMesh.h"

#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Utility/FileManager/FileManager.h"

#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"

constexpr float MY_PI = 3.1415926535f;

BossSlashState::BossSlashState(Boss* owner)
	: BossAttackStateBase(owner)
	, m_List(enList::none)
{
}

BossSlashState::~BossSlashState()
{
}

void BossSlashState::Enter()
{
    BossAttackStateBase::Enter();

    m_CurrentTime = 0.0f;
    m_AnimSpeed = 0.0f;
    m_pOwner->SetIsLoop(false);
    m_pOwner->SetAnimTime(0.0);
    m_pOwner->SetAnimSpeed(m_AnimSpeed);

    // 斬るアニメーションの再生.
    m_pOwner->ChangeAnim(Boss::enBossAnim::Slash);

	// 初期位置を保存.
	const DirectX::XMFLOAT3 BossPosF = m_pOwner->GetPosition();
	DirectX::XMStoreFloat3(&m_StartPos, DirectX::XMLoadFloat3(&BossPosF));

	m_List = enList::ChargeSlash;
}

void BossSlashState::Update()
{
    BossAttackStateBase::Update();

    const float dt = m_pOwner->GetDelta();
    
    UpdateBaseLogic(dt);

    switch (m_List)
    {
    case BossSlashState::enList::ChargeSlash:
        FacePlayerInstantYaw();

        if (m_CurrentTime >= m_ChargeTime)
        {
            m_List = enList::SlashAttack;
            SoundManager::GetInstance().Play("BossSwing", false);
            SoundManager::GetInstance().SetVolume("BossSwing", 9000);
            SoundManager::GetInstance().Play("Throw", false);
            SoundManager::GetInstance().SetVolume("Throw", 8000);
        }
        break;

    case BossSlashState::enList::SlashAttack:
        if (m_TransitionOnAnimEnd_Attack)
        {
            if (m_pOwner->IsAnimEnd(Boss::enBossAnim::Slash))
            {
                m_List = enList::SlashIdol;
                m_pOwner->ChangeAnim(Boss::enBossAnim::SlashToIdol);
            }
        }
        else
        {
            if (m_CurrentTime >= m_ChargeTime + m_AttackTime)
            {
                m_List = enList::SlashIdol;
                m_pOwner->ChangeAnim(Boss::enBossAnim::SlashToIdol);
            }
        }
        break;

    case BossSlashState::enList::SlashIdol:
        // Idol遷移も時間で統一（余韻用に固定）
        if (m_TransitionOnAnimEnd_Exit)
        {
            if (m_pOwner->IsAnimEnd(Boss::enBossAnim::SlashToIdol))
            {
                if (!m_IsDebugStop)
                {
                    m_pOwner->GetStateMachine()->ChangeState(std::make_shared<BossIdolState>(m_pOwner));
                }
                break;
            }
        }
        else
        {
            if (m_CurrentTime >= m_ChargeTime + m_AttackTime + m_EndTime)
            {
                if (!m_IsDebugStop)
                {
                    m_pOwner->GetStateMachine()->ChangeState(std::make_shared<BossIdolState>(m_pOwner));
                }
                break;
            }
        }

    default:
        break;
    }

}

void BossSlashState::LateUpdate()
{
    BossAttackStateBase::LateUpdate();
}

void BossSlashState::Draw()
{
    BossAttackStateBase::Draw();
}

void BossSlashState::Exit()
{
    BossAttackStateBase::Exit();
	// window 制御のコライダーを確実にOFF
	m_pOwner->SetColliderActiveByName("boss_Hand_R", false);
}

std::pair<Boss::enBossAnim, float> BossSlashState::GetParryAnimPair()
{
    return std::pair(Boss::enBossAnim::Slash, 2.360f);
}

void BossSlashState::DrawImGui()
{
#if _DEBUG
    ImGui::Begin(IMGUI_JP("ボス斬撃設定"));
    ImGui::Separator();

    // オフセットは ColliderWindow で管理するため、ここでは基底クラスのImGuiのみ呼ぶ
    BossAttackStateBase::DrawImGui();
    ImGui::End();
#endif
}

void BossSlashState::LoadSettings()
{
    // 基底の読み込みのみ
    BossAttackStateBase::LoadSettings();
}

void BossSlashState::SaveSettings() const
{
    // 基底の保存のみ
    BossAttackStateBase::SaveSettings();
}
