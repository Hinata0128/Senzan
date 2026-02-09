#include "PlayerStateBase.h"
#include "../Player.h"      

#include "Game/04_Time/Time.h"    

#include "System/Singleton/Debug/Log/DebugLog.h"    

static constexpr float DEFAULT_ROTATION_SPEED = 360.0f * 2.f;

PlayerStateBase::PlayerStateBase(Player* owner)
	: StateBase <Player>(owner)
{
}

Player* PlayerStateBase::GetPlayer() const
{
    return m_pOwner;
}

// 正面へラープ回転.
void PlayerStateBase::RotetToFront()
{
    if(m_pOwner->IsSpecial()) return;
    DirectX::XMFLOAT3 move_vec = m_pOwner->m_MoveVec;

    // 小さなノイズを吸収するためのイプシロン
    constexpr float kMoveEpsilon = 1e-4f;
    if (MyMath::IsVector3NearlyZero(move_vec, kMoveEpsilon)) {
        return;
    }

    // atan2f(x, y) の引数順: 0度 = +Y軸方向
    // もし 0度 = +X軸を期待する場合は atan2f(y, x) に変更してください
    float target_angle_rad = std::atan2f(move_vec.x, move_vec.y);
    float target_angle_deg = target_angle_rad * (180.0f / DirectX::XM_PI);

    m_pOwner->RotetToTarget(target_angle_deg, DEFAULT_ROTATION_SPEED);
}
