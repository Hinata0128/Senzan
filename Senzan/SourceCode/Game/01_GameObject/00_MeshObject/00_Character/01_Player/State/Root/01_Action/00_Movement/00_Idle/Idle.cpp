#include "Idle.h"
#include "../../../../../Player.h"


#include "Game/05_InputDevice/Input.h"
#include "Game/05_InputDevice/VirtualPad.h"

#include "System/Singleton/ImGui/CImGuiManager.h"

static constexpr double IDOL_ANIM_SPEED = 1.0;

namespace PlayerState {
Idle::Idle(Player* owner)
	: Movement(owner)
{
}

Idle::~Idle()
{
}

// IDの取得.
constexpr PlayerState::eID Idle::GetStateID() const
{
	return PlayerState::eID::Idle;
}

void Idle::Enter()
{
	Movement::Enter();

	// 待機状態では移動ベクトルをクリア（攻撃後の向きが残らないように）
	m_pOwner->m_MoveVec = {};

	m_pOwner->SetIsLoop(true);
	m_pOwner->SetAnimSpeed(IDOL_ANIM_SPEED);
	m_pOwner->ChangeAnim(Player::eAnim::Idle);
}

void Idle::Update()
{
	// 入力を取得.
	DirectX::XMFLOAT2 input_vec = VirtualPad::GetInstance().GetAxisInput(VirtualPad::eGameAxisAction::Move);

	// 入力があれば移動.
	if (!MyMath::IsVector2NearlyZero(input_vec, 0.f)) {
		m_pOwner->ChangeState(eID::Run);
		return;
	}

	Movement::Update();
}

void Idle::LateUpdate()
{
	Movement::LateUpdate();
}

void Idle::Draw()
{
	Movement::Draw();
}

void Idle::Exit()
{
	Movement::Exit();
}
} // PlayerState.
