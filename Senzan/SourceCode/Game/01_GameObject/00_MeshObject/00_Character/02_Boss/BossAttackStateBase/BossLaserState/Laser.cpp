#include "Laser.h"

#include "Game//04_Time//Time.h"
#include "Game//01_GameObject//00_MeshObject//00_Character//02_Boss//Boss.h"
#include "Game//01_GameObject//00_MeshObject//00_Character//01_Player//Player.h"
#include "..//..//BossMoveState//BossMoveState.h"


#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"


Laser::Laser(Boss* owner)
    : BossAttackStateBase(owner)
    , m_pBossIdol()

    , m_AnimChange(enAnimChange::none)
{
}

Laser::~Laser()
{
}

void Laser::Enter()
{

}

void Laser::Update()
{
}

void Laser::LateUpdate()
{
}

void Laser::Draw()
{
}

void Laser::Exit()
{
}

