#include "BossThrowingState.h"
#include "00_MeshObject/00_Character/02_Boss/Boss.h"
#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"
#include "..//04_Time/Time.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Utility/FileManager/FileManager.h"
#include "System/Singleton/SnowBallManager/SnowBallManager.h"

BossThrowingState::BossThrowingState(Boss* owner)
	: BossAttackStateBase(owner)
	, m_List(enThrowing::None)
	, m_IsLaunched(false)
	, m_BallHeight(5.0f)
	, m_BallSpeed(1.0f)
{
}

BossThrowingState::~BossThrowingState()
{
}

void BossThrowingState::Enter()
{
	// プレイヤーの方を向く
	DirectX::XMFLOAT3 BossPosF = m_pOwner->GetPosition();
	DirectX::XMFLOAT3 PlayerPosF = m_pOwner->m_PlayerPos;

	float dx = PlayerPosF.x - BossPosF.x;
	float dz = PlayerPosF.z - BossPosF.z;
	float angle_radian = std::atan2f(dx, dz) + DirectX::XM_PI;
	m_pOwner->SetRotationY(angle_radian);

	// アニメーション開始
	m_pOwner->SetAnimSpeed(2.0);
	m_pOwner->ChangeAnim(Boss::enBossAnim::LaserCharge); // 投擲用の溜め

	m_IsLaunched = false;
	m_List = enThrowing::None;
}

void BossThrowingState::Update()
{
	float deltaTime = m_pOwner->GetDelta();
    m_pOwner->m_AnimTimer += deltaTime;

	switch (m_List)
	{
	case enThrowing::None:
		// 当たり判定を無効化.
		m_List = enThrowing::Anim;
		break;

	case enThrowing::Anim:
		if (m_pOwner->IsAnimEnd(Boss::enBossAnim::LaserCharge))
		{
			m_pOwner->ChangeAnim(Boss::enBossAnim::Laser); // 投げる瞬間のアニメ
			m_List = enThrowing::Attack;
            SoundManager::GetInstance().Play("Throw");
            SoundManager::GetInstance().SetVolume("Throw", 9000);
		}
		break;

	case enThrowing::Attack:
		if (!m_IsLaunched)
		{
			// 発射位置を調整（ボスの手元の高さ）
			DirectX::XMFLOAT3 startPos = m_pOwner->GetPosition();
			startPos.y += m_BallHeight;

			// 雪玉発射
			SnowBallManager::GetInstance().Spawn(m_pOwner->GetTargetPos(), startPos);
			m_IsLaunched = true;
		}

        if (m_pOwner->IsAnimEnd(Boss::enBossAnim::Laser))
        {
            m_pOwner->ChangeAnim(Boss::enBossAnim::LaserEnd);
            m_List = enThrowing::CoolDown;
        }
		
		break;

	case enThrowing::CoolDown:
		if (m_pOwner->IsAnimEnd(Boss::enBossAnim::LaserEnd))
		{
			m_List = enThrowing::Trans;
		}
		break;

	case enThrowing::Trans:
		// 次の行動（アイドル）へ
		m_pOwner->GetStateMachine()->ChangeState(std::make_shared<BossIdolState>(m_pOwner));
		break;
	}
}

void BossThrowingState::LateUpdate()
{
}

void BossThrowingState::Draw()
{
}

void BossThrowingState::Exit()
{
}

std::pair<Boss::enBossAnim, float> BossThrowingState::GetParryAnimPair()
{
    return std::pair(Boss::enBossAnim::Laser, 0.918f);
}

void BossThrowingState::BossAttack()
{
}

void BossThrowingState::DrawImGui()
{
#if _DEBUG
    ImGui::Begin(IMGUI_JP("BossThrowing State"));
    CImGuiManager::Slider<float>(IMGUI_JP("発射高さ"), m_BallHeight, 0.0f, 15.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("弾速"), m_BallSpeed, 0.1f, 5.0f, true);
    BossAttackStateBase::DrawImGui();
    ImGui::End();
#endif
}

void BossThrowingState::LoadSettings()
{
    BossAttackStateBase::LoadSettings();
    auto srcDir = std::filesystem::path(__FILE__).parent_path();
    auto filePath = srcDir / GetSettingsFileName();
    if (!std::filesystem::exists(filePath)) return;
    json j = FileManager::JsonLoad(filePath);
    if (j.contains("BallHeight")) m_BallHeight = j["BallHeight"].get<float>();
    if (j.contains("BallSpeed")) m_BallSpeed = j["BallSpeed"].get<float>();
}

void BossThrowingState::SaveSettings() const
{
    json j = SerializeSettings();
    j["BallHeight"] = m_BallHeight;
    j["BallSpeed"] = m_BallSpeed;
    auto srcDir = std::filesystem::path(__FILE__).parent_path();
    auto filePath = srcDir / GetSettingsFileName();
    FileManager::JsonSave(filePath, j);
}

