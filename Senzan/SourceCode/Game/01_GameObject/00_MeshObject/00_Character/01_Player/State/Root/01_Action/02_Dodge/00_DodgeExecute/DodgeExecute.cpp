#include "DodgeExecute.h"

#include "00_MeshObject/00_Character/01_Player/Player.h"
#include "../Dodge.h"

#include "Game/05_InputDevice/VirtualPad.h"
#include "System/Singleton/CameraManager/CameraManager.h"
#include "System/Singleton/ImGui/CImGuiManager.h"

#include "System/Utility/SingleTrigger/SingleTrigger.h"
#include "System/Utility/Math/Easing/Easing.h"

// 回避中の速度.
static constexpr float  DODGE_DISTANCE   = 25.0f;	// 目標移動距離.
static constexpr float  DODGE_DURATION   = 1.7f;	// 移動にかける時間（短くしてキビキビ感を出す）.
static constexpr double DODGE_ANIM_SPEED = 4.0;		// アニメーション再生速度.
static constexpr double DODGE_START_TIME = 0.0;		// アニメーションの開始秒数.

namespace PlayerState {
DodgeExecute::DodgeExecute(Player* owner)
	: Dodge(owner)
	, m_IsAnimEnd		( false )
	, m_traveledDistance( 0.0f )
{
}

DodgeExecute::~DodgeExecute()
{
}

// IDの取得.
constexpr PlayerState::eID DodgeExecute::GetStateID() const
{
	return PlayerState::eID::DodgeExecute;
}

void DodgeExecute::Enter()
{
	// TODO : クールタイムの表示.

	Dodge::Enter();

	m_Distance = DODGE_DISTANCE;
	m_MaxTime = DODGE_DURATION;
	m_traveledDistance = 0.0f;

	m_pOwner->SetIsLoop(false);
	m_pOwner->SetAnimSpeed(DODGE_ANIM_SPEED);
	m_pOwner->SetAnimTime(DODGE_START_TIME);
	m_pOwner->ChangeAnim(Player::eAnim::Attack_0);
}

void DodgeExecute::Update()
{
	Dodge::Update();
}

void DodgeExecute::LateUpdate()
{
	Dodge::LateUpdate();

	// 前フレーム時点の目標距離を取得.
	float prev_target_dist = 0.0f;
	float blend_ratio = 0.5f;  // Liner比率を上げて最後の減速を緩やかに.
	{
		float eased = 0.0f, liner = 0.0f;
		// InOutCubic: 最初加速 → 中間最速 → 緩やかに減速.
		MyEasing::UpdateEasing(MyEasing::Type::InOutCubic, m_currentTime, m_MaxTime, 0.0f, m_Distance, eased);
		MyEasing::UpdateEasing(MyEasing::Type::Liner, m_currentTime, m_MaxTime, 0.0f, m_Distance, liner);
		prev_target_dist = eased * (1.0f - blend_ratio) + liner * blend_ratio;
	}

	// デルタタイムの計算.
	double animSpeed = MyMath::IsNearlyEqual(m_pOwner->m_AnimSpeed, 0.0) ? 1.0 : m_pOwner->m_AnimSpeed;
	float deltaTime = static_cast<float>(animSpeed) * m_pOwner->GetDelta();

	// 時間と距離の更新.
	m_currentTime += deltaTime;

	// クランプ.
	if (m_currentTime > m_MaxTime) {
		m_currentTime = m_MaxTime;
	}

	// 移動量を算出.
	float current_target_dist = 0.0f;
	{
		float eased = 0.0f, liner = 0.0f;
		MyEasing::UpdateEasing(MyEasing::Type::InOutCubic, m_currentTime, m_MaxTime, 0.0f, m_Distance, eased);
		MyEasing::UpdateEasing(MyEasing::Type::Liner, m_currentTime, m_MaxTime, 0.0f, m_Distance, liner);
		current_target_dist = eased * (1.0f - blend_ratio) + liner * blend_ratio;
	}
	float move_amount = current_target_dist - prev_target_dist;
	m_traveledDistance = current_target_dist;

	// 座標更新.
	DirectX::XMFLOAT3 moveDirection = { m_InputVec.x, 0.0f, m_InputVec.y };
	DirectX::XMFLOAT3 movement = {
		moveDirection.x * move_amount,
		0.0f,
		moveDirection.z * move_amount
	};
	m_pOwner->AddPosition(movement);

	// 移動しきったら終了.
	if (m_traveledDistance >= m_Distance)
	{
		m_IsAnimEnd = true;
		m_pOwner->ChangeState(PlayerState::eID::Idle);
	}
}

void DodgeExecute::Draw()
{
	Dodge::Draw();
}

void DodgeExecute::Exit()
{
	Dodge::Exit();
}

} // PlayerState.
