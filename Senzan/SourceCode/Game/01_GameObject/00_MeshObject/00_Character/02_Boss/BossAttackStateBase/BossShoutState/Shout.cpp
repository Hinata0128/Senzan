#include "Shout.h"

#include "Game/04_Time/Time.h"
#include "00_MeshObject/00_Character/02_Boss/Boss.h"
#include "00_MeshObject/00_Character/01_Player/Player.h"

#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"
#include "System/Singleton/ImGui/CImGuiManager.h"

Shout::Shout(Boss* owner)
	: BossAttackStateBase(owner)

	, m_List(enShout::None)

	, m_ShoutDamage(0.0f)
	, m_ShoutRadius(180.0f)
	, m_KnockBackPower(15.0f)
	
	, m_ShoutExpandTime(1.0f)
	, m_ShoutElapsed(0.0f)
	, m_ShoutStartRadius(0.0f)
	
	, m_EffectPlayed(false)
	, m_EffectOffset(DirectX::XMFLOAT3(0.0f, 15.0f, 0.0f))
	, m_EffectScale(10.0f)
{
}

Shout::~Shout()
{
}

void Shout::Enter()
{
	//初期値のリセット.
	//子レイダーは実際のシャウト開始時に有効化する.
	m_ShoutElapsed = 0.0f;
	m_ShoutStartRadius = 0.0f;
	//ボスの向きの設定.
	FacePlayerYawContinuous();
	//初期位置を保存.
	const DirectX::XMFLOAT3 BossPosF = m_pOwner->GetPosition();
	//アニメーション速度.
	m_pOwner->SetAnimSpeed(1.5);
	m_pOwner->ChangeAnim(Boss::enBossAnim::LaserCharge);
	m_EffectPlayed = false;
}

void Shout::Update()
{
	switch (m_List)
	{
	case Shout::enShout::None:
		m_List = enShout::Shout;
		break;
	case Shout::enShout::Shout:
		if (m_pOwner->IsAnimEnd(Boss::enBossAnim::LaserCharge))
		{
			m_pOwner->SetAnimSpeed(1.0f);
			m_pOwner->ChangeAnim(Boss::enBossAnim::Laser);
			if (!m_EffectPlayed && m_pOwner)
			{
				m_pOwner->PlayEffect("Shout", m_EffectOffset, m_EffectScale);
				m_EffectPlayed = true;
				//音の再生.
				SoundManager::GetInstance().Play("Shout", false);
				SoundManager::GetInstance().SetVolume("Shout", 9400);
			}
			//シャウト発動:コライダーを有効化し
			//半径を徐々に広げる準備.
			if (auto* ShoutCol = m_pOwner->GetShoutCollider())
			{
				//攻撃判定有効.
				ShoutCol->SetActive(true);
				ShoutCol->SetAttackAmount(m_ShoutDamage);
				m_ShoutElapsed = 0.0f;
				m_ShoutStartRadius = 0.0f;
				if (m_ShoutExpandTime <= 0.0f)
				{
					//半径15.0f.
					ShoutCol->SetRadius(m_ShoutRadius);
				}
				else
				{
					//半径0.0f.
					ShoutCol->SetRadius(m_ShoutStartRadius);
				}
			}
			m_List = enShout::ShoutTime;
		}
		break;
	case Shout::enShout::ShoutTime:
		//コライダー半径を徐々に拡大.
		if (auto* ShoutCol = m_pOwner->GetShoutCollider())
		{
			if (ShoutCol->GetActive())
			{
				//デルタタイムの取得.
				float deltaTime = m_pOwner->GetDelta();
				m_ShoutElapsed += deltaTime;
				if (m_ShoutExpandTime > 0.0f)
				{
					float T = m_ShoutElapsed / m_ShoutExpandTime;
					if (T > 1.0f)
					{
						T = 1.0f;
					}
					float Radius = m_ShoutStartRadius + (m_ShoutRadius - m_ShoutStartRadius) * T;
					ShoutCol->SetRadius(Radius);
					ShoutCol->SetHeight(Radius);
				}
				else
				{
					ShoutCol->SetRadius(m_ShoutRadius);
				}
			}
			if (m_pOwner->IsAnimEnd(Boss::enBossAnim::Laser))
			{
				m_pOwner->SetAnimSpeed(1.5);
				m_pOwner->ChangeAnim(Boss::enBossAnim::LaserEnd);
				m_List = enShout::ShoutToIdol;
				if (auto* ShoutCol = m_pOwner->GetShoutCollider())
				{
					//攻撃判定解除.
					ShoutCol->SetActive(false);
				}
			}
		}
		break;
	case Shout::enShout::ShoutToIdol:
		if (m_pOwner->IsAnimEnd(Boss::enBossAnim::LaserEnd))
		{
			m_pOwner->GetStateMachine()->ChangeState(std::make_shared<BossIdolState>(m_pOwner));
		}
		break;
	default:
		break;
	}
}

void Shout::LateUpdate()
{
}

void Shout::Draw()
{
}

void Shout::Exit()
{
}

std::pair<Boss::enBossAnim, float> Shout::GetParryAnimPair()
{
	//パリィしないので0.0fにしている.
	return std::pair(Boss::enBossAnim::Slash, 0.0f);
}

void Shout::DrawImGui()
{
#if _DEBUG
	ImGui::Begin(IMGUI_JP("BossShout State"));
	CImGuiManager::Slider<float>(IMGUI_JP("ダメージ量"), m_ShoutDamage, 0.0f, 50.0f, true);
	CImGuiManager::Slider<float>(IMGUI_JP("範囲半径"), m_ShoutRadius, 5.0f, 60.0f, true);
	CImGuiManager::Slider<float>(IMGUI_JP("ノックバック力"), m_KnockBackPower, 0.0f, 30.0f, true);
	BossAttackStateBase::DrawImGui();
	ImGui::End();
#endif  

}

void Shout::LoadSettings()
{
	BossAttackStateBase::LoadSettings();
	auto srcDir = std::filesystem::path(__FILE__).parent_path();
	auto filePath = srcDir / GetSettingsFileName();
	if (!std::filesystem::exists(filePath)) return;
	json j = FileManager::JsonLoad(filePath);
	if (j.contains("ShoutDamage")) m_ShoutDamage = j["ShoutDamage"].get<float>();
	if (j.contains("ShoutRadius")) m_ShoutRadius = j["ShoutRadius"].get<float>();
	if (j.contains("KnockBackPower")) m_KnockBackPower = j["KnockBackPower"].get<float>();
	// オフセットは ColliderWindow で管理

}

void Shout::SaveSettings() const
{
	json j = SerializeSettings();
	j["ShoutDamage"] = m_ShoutDamage;
	j["ShoutRadius"] = m_ShoutRadius;
	j["KnockBackPower"] = m_KnockBackPower;
	auto srcDir = std::filesystem::path(__FILE__).parent_path();
	auto filePath = srcDir / GetSettingsFileName();
	FileManager::JsonSave(filePath, j);

}
