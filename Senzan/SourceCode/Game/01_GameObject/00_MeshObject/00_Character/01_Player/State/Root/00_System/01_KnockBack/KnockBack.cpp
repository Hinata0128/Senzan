#include "KnockBack.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"

#include "Game/04_Time/Time.h"
#include "System/Singleton/Debug/Log/DebugLog.h"


static constexpr float INITIAL_JUMP_SPEED   = 6.0f;    // 垂直速度. 
static constexpr float GRAVITY_ACCELERATION = -9.8f * 2.5f;     // 重力.
static constexpr float DAMPING_FACTOR       = 0.99f;    // 減衰率.

namespace PlayerState {
KnockBack::KnockBack(Player* owner)
	: System(owner)
{
}

KnockBack::~KnockBack()
{
}

// IDの取得.
constexpr PlayerState::eID KnockBack::GetStateID() const
{
	return PlayerState::eID::KnockBack;
}

void KnockBack::Enter()
{
    m_pOwner->m_IsKnockBack = true;

    // アニメーション設定.
    m_pOwner->SetIsLoop(false);
    m_pOwner->SetAnimTime(0.0);
    m_pOwner->SetAnimSpeed(1.0);
    m_pOwner->ChangeAnim(Player::eAnim::Dead);

    DirectX::XMVECTOR v_initial_vector = DirectX::XMLoadFloat3(&m_pOwner->m_KnockBackVec);

    // Y成分を0にし、水平方向の初速を計算.
    DirectX::XMVECTOR v_horizontal_vec = DirectX::XMVectorSetY(v_initial_vector, 0.0f);
    v_horizontal_vec = DirectX::XMVector3Normalize(v_horizontal_vec);
    v_horizontal_vec = DirectX::XMVectorScale(v_horizontal_vec, m_pOwner->m_KnockBackPower);

    // 水平速度と垂直速度を組み合わせる.
    DirectX::XMVECTOR v_final_velocity = DirectX::XMVectorSelect(v_horizontal_vec,
        DirectX::XMVectorSet(0.0f, INITIAL_JUMP_SPEED, 0.0f, 0.0f),
        DirectX::XMVectorSelectControl(0, 1, 0, 0));

    m_pOwner->SetPositionY(1.f);
    // 初速を速度変数に格納.
    DirectX::XMStoreFloat3(&m_pOwner->m_KnockBackVec, v_final_velocity);
}

void KnockBack::Update()
{ 
    DirectX::XMVECTOR v_knock_back_speed = DirectX::XMLoadFloat3(&m_pOwner->m_KnockBackVec);
    float deltaTime = m_pOwner->GetDelta();

    // 重力.
    DirectX::XMVECTOR v_gravity = DirectX::XMVectorSet(0.0f, GRAVITY_ACCELERATION * deltaTime, 0.0f, 0.0f);
    v_knock_back_speed = DirectX::XMVectorAdd(v_knock_back_speed, v_gravity);

    // 減速.
    v_knock_back_speed = DirectX::XMVectorScale(v_knock_back_speed, DAMPING_FACTOR);
    DirectX::XMVECTOR v_movement_distance = DirectX::XMVectorScale(v_knock_back_speed, deltaTime);

    // 今フレームの移動距離.
    DirectX::XMStoreFloat3(&m_AppliedMovement, v_movement_distance);

    // 計算後の速度を設定.
    DirectX::XMStoreFloat3(&m_pOwner->m_KnockBackVec, v_knock_back_speed);

    // 着地判定.
    if (m_pOwner->GetPositionY() < 0.f)
    {
        m_pOwner->SetPositionY(0.f); 

        m_pOwner->ChangeState(PlayerState::eID::Idle);
    }
}

void KnockBack::LateUpdate()
{
    // 座標移動.
    m_pOwner->AddPosition(m_AppliedMovement);

    System::LateUpdate();
}

void KnockBack::Draw()
{
}

void KnockBack::Exit()
{
    m_pOwner->m_IsKnockBack = false;
}

} // PlayerState.
