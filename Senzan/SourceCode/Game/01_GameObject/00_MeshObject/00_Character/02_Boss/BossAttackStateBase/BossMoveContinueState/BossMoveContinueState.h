#pragma once

#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossAttackStateBase.h"

class BossMoveContinueState : public BossAttackStateBase
{
public:
    BossMoveContinueState(Boss* owner);
    ~BossMoveContinueState() override;

    void Enter() override;
    void Update() override;
    void LateUpdate() override;
    void Draw() override;
    void Exit() override;

private:
    float m_Duration = 5.0f; 
    float m_Timer = 0.0f;
    float m_MoveSpeed = 10.0f; // forward move speed
    // Movement logic copied from BossMoveState (phases: Start/Run/Stop/Strafe)
    enum class MovePhase : unsigned char
    {
        Start,
        Run,
        Stop,
        Strafe,
    };

    MovePhase m_Phase = MovePhase::Start;
    float m_RotationAngle = 0.0f;
    double m_RotationSpeed = 0.1;
    float m_rotationDirection = 1.0f;
    // Lerp multiplier used when tracking strafe ideal position. Larger = faster tracking
    float m_StrafeLerp = 3.5f;
    float m_BaseAngle = 0.0f;
};
