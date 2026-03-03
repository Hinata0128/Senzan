#include "Slash.h"

#include "Resource/Mesh/02_Skin/SkinMesh.h"
#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"
#include "00_MeshObject/00_Character/02_Boss/Boss.h"
#include "00_MeshObject/00_Character/01_Player/Player.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Utility/FileManager/FileManager.h"

Slash::Slash(Boss* owner)
	: BossAttackStateBase	(owner)
{
}

Slash::~Slash()
{
}

void Slash::Enter()
{
}

void Slash::Update()
{
}

void Slash::LateUpdate()
{
}

void Slash::Draw()
{
}

void Slash::Exit()
{
}

std::pair<Boss::enBossAnim, float> Slash::GetParryAnimPair()
{
	return std::pair<Boss::enBossAnim, float>();
}

void Slash::DrawImGui()
{
}

void Slash::LoadSettings()
{
}

void Slash::SaveSettings() const
{
}
