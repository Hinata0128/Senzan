#include "BossDeadState.h"

#include "00_MeshObject//00_Character//02_Boss//Boss.h"

static constexpr double AnimSpeed = 3.0;

BossDeadState::BossDeadState(Boss* owner)
	: StateBase<Boss>(owner)
{
}

BossDeadState::~BossDeadState()
{
}

//遷移時に一回だけ入る関数.
void BossDeadState::Enter()
{
	m_pOwner->SetIsLoop(false);
	m_pOwner->SetAnimSpeed(AnimSpeed);
	//死亡アニメションを再生.
	m_pOwner->ChangeAnim(Boss::enBossAnim::Dead);

	if (m_pOwner->IsAnimEnd(Boss::enBossAnim::Dead))
	{
		//ゲームクリアへ遷移させる
		return;
	}
}

void BossDeadState::Update()
{
}

void BossDeadState::LateUpdate()
{
}

void BossDeadState::Draw()
{
}

void BossDeadState::Exit()
{
}
