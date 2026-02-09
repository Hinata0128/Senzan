#include "Movement.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"

#include "Game/03_Collision/00_Core/01_Capsule/CapsuleCollider.h"

#include "Game/05_InputDevice/VirtualPad.h"    
#include "Game/04_Time/Time.h"   

namespace PlayerState {

// ----------------------------------------------------
// コンストラクタ
// ----------------------------------------------------
Movement::Movement(Player* owner)
	: Action(owner)
	, m_Collision_BodyHit(std::make_shared<CapsuleCollider>(owner->GetTransform()))
{
}

Movement::~Movement()
{
}

void Movement::Enter()
{
	Action::Enter();
}

void Movement::Update()
{
    Action::Update();

	if (VirtualPad::GetInstance().IsActionPress(VirtualPad::eGameAction::Attack))
	{
		m_pOwner->ChangeState(PlayerState::eID::AttackCombo_0);
		return;
	}
	
	if (VirtualPad::GetInstance().IsActionDown(VirtualPad::eGameAction::Parry))
	{
		m_pOwner->ChangeState(PlayerState::eID::Parry);
		return;
	}
}

void Movement::LateUpdate()
{
	Action::LateUpdate();
}

void Movement::Draw()
{
	Action::Draw();
}

void Movement::Exit()
{
	Action::Exit();
}
} // namespace PlayerState
