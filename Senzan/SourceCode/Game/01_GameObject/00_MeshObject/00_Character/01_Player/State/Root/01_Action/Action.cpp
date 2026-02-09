#include "Action.h"

#include "00_MeshObject/00_Character/01_Player/Player.h"

#include "Game/05_InputDevice/VirtualPad.h"

#include "System/Utility/SingleTrigger/SingleTrigger.h"
#include "System/Singleton/Debug/Log/DebugLog.h"
static constexpr float JUSTDODGE_MAX_DISTANCE = 65.0f;

namespace PlayerState {
Action::Action(Player* owner)
	: PlayerStateBase(owner)
	, m_AnimSpeedChangedTrigger(std::make_unique<SingleTrigger>())
{
}

Action::~Action()
{
}

void Action::Enter()
{
}

void Action::Update()
{
	// 必殺技ボタンが押されたら(ゲージMAX時のみ).
	if (VirtualPad::GetInstance().IsActionDown(VirtualPad::eGameAction::SpecialAttack))
	{
		if (m_pOwner->m_CurrentUltValue >= m_pOwner->m_MaxUltValue)
		{
			m_pOwner->ChangeState(PlayerState::eID::SpecialAttack);
			return;
		}
	}

    // 回避ボタンが押されたら.
    if (VirtualPad::GetInstance().IsActionDown(VirtualPad::eGameAction::Dodge))
    {
        // ジャスト回避に派生（ボスが近距離にいる場合のみ有効）
        if (m_pOwner->m_IsJustDodgeTiming)
        {

            const DirectX::XMFLOAT3 player_pos = m_pOwner->GetPosition();
            const DirectX::XMFLOAT3 boss_pos = m_pOwner->m_TargetPos;
            const float dx = boss_pos.x - player_pos.x;
            const float dz = boss_pos.z - player_pos.z;
            bool is_true = (dx * dx + dz * dz) <= (JUSTDODGE_MAX_DISTANCE * JUSTDODGE_MAX_DISTANCE);

            if (is_true)
            {
                m_pOwner->ChangeState(PlayerState::eID::JustDodge);
            }
            else
            {
                // 遠距離なら通常回避にフォールバック
                m_pOwner->ChangeState(PlayerState::eID::DodgeExecute);
            }
        }
        // 回避に派生.
        else
        {
            m_pOwner->ChangeState(PlayerState::eID::DodgeExecute);
        }
        return;
    }

    // パリィボタンが押されたら.
	
    if (VirtualPad::GetInstance().IsActionDown(VirtualPad::eGameAction::Parry))
    {
        m_pOwner->ChangeState(PlayerState::eID::Parry);
        return;
    }
}

void Action::LateUpdate()
{
    RotetToFront();
}

void Action::Draw()
{
}

void Action::Exit()
{
}
} // PlayerState.
