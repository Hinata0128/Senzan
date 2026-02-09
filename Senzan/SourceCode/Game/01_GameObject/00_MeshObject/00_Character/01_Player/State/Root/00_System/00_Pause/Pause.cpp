#include "Pause.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"

namespace PlayerState {
	Pause::Pause(Player* owner)
		: System(owner)
	{
	}
	Pause::~Pause()
	{
	}

	// ID�̎擾.
	constexpr PlayerState::eID Pause::GetStateID() const
	{
		return PlayerState::eID::Pause;
	}
	void Pause::Enter()
	{
	}
	void Pause::Update()
	{
	}
	void Pause::LateUpdate()
	{
	}
	void Pause::Draw()
	{
	}
	void Pause::Exit()
	{
	}
}
