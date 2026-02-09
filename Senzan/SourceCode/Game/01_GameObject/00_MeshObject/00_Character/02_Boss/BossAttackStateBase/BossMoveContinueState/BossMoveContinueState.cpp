#include "BossMoveContinueState.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/Boss.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/BossMoveState/BossMoveState.h"

BossMoveContinueState::BossMoveContinueState(Boss* owner)
    : BossAttackStateBase(owner)
{
}

BossMoveContinueState::~BossMoveContinueState()
{
}

void BossMoveContinueState::Enter()
{
    m_Timer = 0.0f;
    m_EndTime = m_Duration;
    // Use Boss API to change anim via state machine owner if allowed
    // Boss::enBossAnim is private; use ChangeAnim by enum index if available
    m_pOwner->ChangeAnim(Boss::enBossAnim::Run); // Changed animation to Run
}

void BossMoveContinueState::Update()
{
    float delta = m_pOwner->GetDelta();
    m_Timer += delta;

    // Follow simplified BossMoveState movement phases for the duration of this state
    DirectX::XMVECTOR vBossPos = DirectX::XMLoadFloat3(&m_pOwner->GetPosition());
    DirectX::XMFLOAT3 playerPosF = m_pOwner->GetTargetPos();
    DirectX::XMVECTOR vTarget = DirectX::XMLoadFloat3(&playerPosF);

    DirectX::XMVECTOR vToPlayer = DirectX::XMVectorSubtract(vTarget, vBossPos);
    vToPlayer = DirectX::XMVectorSetY(vToPlayer, 0.0f);
    float distanceToPlayer = DirectX::XMVectorGetX(DirectX::XMVector3Length(vToPlayer));

    // Use tunable members instead of local constants
    const float STRAFE_RANGE = 20.0f;

    // slightly increase rotation speed for snappier strafing
    m_RotationSpeed = 0.4; // was 0.1

    switch (m_Phase)
    {
    case MovePhase::Start:
        // Immediately enter Run within this continue state
        m_pOwner->ChangeAnim(Boss::enBossAnim::Run);
        m_Phase = MovePhase::Run;
        break;
    case MovePhase::Run:
    {
        // Use configurable forward move speed
        float approachSpeed = m_MoveSpeed;
        DirectX::XMVECTOR vMoveDir = DirectX::XMVector3Normalize(vToPlayer);
        DirectX::XMVECTOR vNewPos = DirectX::XMVectorAdd(vBossPos, DirectX::XMVectorScale(vMoveDir, approachSpeed * delta));

        DirectX::XMFLOAT3 newPosF;
        DirectX::XMStoreFloat3(&newPosF, vNewPos);
        m_pOwner->SetPosition(newPosF);
        vBossPos = vNewPos;

        if (distanceToPlayer <= STRAFE_RANGE)
        {
            m_Phase = MovePhase::Stop;
            m_pOwner->ChangeAnim(Boss::enBossAnim::RunToIdol);
        }
    }
    break;
    case MovePhase::Stop:
        if (m_pOwner->IsAnimEnd(Boss::enBossAnim::RunToIdol))
        {
            m_Phase = MovePhase::Strafe;
            DirectX::XMVECTOR vDirFromPlayer = DirectX::XMVectorSubtract(vBossPos, vTarget);
            m_BaseAngle = atan2f(DirectX::XMVectorGetX(vDirFromPlayer), DirectX::XMVectorGetZ(vDirFromPlayer));
            m_RotationAngle = 0.0f;
        }
        break;
    case MovePhase::Strafe:
    {
        m_RotationAngle += static_cast<float>(m_RotationSpeed) * delta * m_rotationDirection;
        const float MAX_SWAY = DirectX::XM_PIDIV4;
        if (fabsf(m_RotationAngle) > MAX_SWAY)
        {
            m_rotationDirection *= -1.0f;
            m_RotationAngle = std::clamp(m_RotationAngle, -MAX_SWAY, MAX_SWAY);
        }

        float finalAngle = m_BaseAngle + m_RotationAngle;
        DirectX::XMVECTOR vOffset = DirectX::XMVectorSet(
            sinf(finalAngle) * STRAFE_RANGE,
            0.0f,
            cosf(finalAngle) * STRAFE_RANGE,
            0.0f
        );
        DirectX::XMVECTOR vIdealPos = DirectX::XMVectorAdd(vTarget, vOffset);

        constexpr float TRACKING_DELAY = 0.7f;

        DirectX::XMVECTOR vCurrentPos = XMLoadFloat3(&m_pOwner->GetPosition());
        float lerpFactor = TRACKING_DELAY * delta;
        if (lerpFactor > 1.0f) lerpFactor = 1.0f;

        DirectX::XMVECTOR vNextPos = DirectX::XMVectorLerp(vCurrentPos, vIdealPos, lerpFactor);

        DirectX::XMFLOAT3 finalPosF;
        DirectX::XMStoreFloat3(&finalPosF, vNextPos);
        finalPosF.y = m_pOwner->GetPosition().y;
        m_pOwner->SetPosition(finalPosF);

        m_pOwner->SetAnimSpeed(3.0);
        if (m_rotationDirection > 0)
            m_pOwner->ChangeAnim(Boss::enBossAnim::LeftMove);
        else
            m_pOwner->ChangeAnim(Boss::enBossAnim::RightMove);
    }
    break;
    }

    // Face player every frame
    DirectX::XMVECTOR vFinalBossPos = XMLoadFloat3(&m_pOwner->GetPosition());
    DirectX::XMVECTOR vLookAt = DirectX::XMVectorSubtract(vTarget, vFinalBossPos);
    float dx = DirectX::XMVectorGetX(vLookAt);
    float dz = DirectX::XMVectorGetZ(vLookAt);
    float angle = atan2f(dx, dz) + DirectX::XM_PI;
    m_pOwner->SetRotationY(angle);

    // End this continue state when duration exceeded
    if (m_Timer >= m_Duration)
    {
        m_pOwner->GetStateMachine()->ChangeState(std::make_shared<BossMoveState>(m_pOwner));
        return;
    }
}

void BossMoveContinueState::LateUpdate()
{
}

void BossMoveContinueState::Draw()
{
}

void BossMoveContinueState::Exit()
{
}

