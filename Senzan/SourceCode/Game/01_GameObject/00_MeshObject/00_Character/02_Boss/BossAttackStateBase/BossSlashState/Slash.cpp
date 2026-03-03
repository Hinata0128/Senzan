#include "Slash.h"

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
