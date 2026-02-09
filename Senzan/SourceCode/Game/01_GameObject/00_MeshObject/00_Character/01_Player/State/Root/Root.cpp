#include "Root.h"

#include "../../Player.h"

#include "00_System/System.h"	
#include "00_System/00_Pause/Pause.h"	
#include "00_System/01_KnockBack/KnockBack.h"	
#include "00_System/02_Dead/Dead.h"	
#include "00_System/03_SpecialAttack/SpecialAttack.h"	

#include "01_Action/Action.h"	
#include "01_Action/00_Movement/Movement.h"	
#include "01_Action/00_Movement/00_Idle/Idle.h"	
#include "01_Action/00_Movement/01_Run/Run.h"	

#include "01_Action/01_Combat/Combat.h"	
#include "01_Action/01_Combat/00_AttackCombo_0/AttackCombo_0.h"	
#include "01_Action/01_Combat/01_AttackCombo_1/AttackCombo_1.h"	
#include "01_Action/01_Combat/02_AttackCombo_2/AttackCombo_2.h"
#include "01_Action/01_Combat/03_Parry/Parry.h"

#include "01_Action/02_Dodge/Dodge.h"	
#include "01_Action/02_Dodge/00_DodgeExecute/DodgeExecute.h"	
#include "01_Action/02_Dodge/01_JustDodge/JustDodge.h"

#include "Game/04_Time/Time.h"

#include "System/Singleton/Debug/Log/DebugLog.h"



namespace PlayerState {
Root::Root(Player* owner)
    : PlayerStateBase(owner)
    , m_pPause          ( nullptr )
    , m_pKnockBack      ( nullptr )
    , m_pDead           ( nullptr )
    , m_pSpecialAttack  ( nullptr )
    , m_pIdle           ( nullptr )
    , m_pRun            ( nullptr )
    , m_pCombo_0        ( nullptr )
    , m_pCombo_1        ( nullptr )
    , m_pCombo_2        ( nullptr )
    , m_pParry          ( nullptr )
    , m_pDodgeExecute   ( nullptr )
    , m_pJustDodge      ( nullptr )
	, m_CurrentActiveState(std::ref(*m_pRun.get())) 
{
}

Root::~Root()
{
}

constexpr PlayerState::eID Root::GetStateID() const
{
    // 現在有効なステートを取得.
    return m_CurrentActiveState.get().GetStateID();
}

void Root::Enter()
{
    m_pPause = std::make_unique<Pause>(m_pOwner);
    m_pKnockBack = std::make_unique<KnockBack>(m_pOwner);
    m_pDead = std::make_unique<Dead>(m_pOwner);
    m_pSpecialAttack = std::make_unique<SpecialAttack>(m_pOwner);
    m_pIdle = std::make_unique<Idle>(m_pOwner);
    m_pRun = std::make_unique<Run>(m_pOwner);
    m_pCombo_0 = std::make_unique<AttackCombo_0>(m_pOwner);
    m_pCombo_1 = std::make_unique<AttackCombo_1>(m_pOwner);
    m_pCombo_2 = std::make_unique<AttackCombo_2>(m_pOwner);
    m_pParry = std::make_unique<Parry>(m_pOwner);
    m_pDodgeExecute = std::make_unique<DodgeExecute>(m_pOwner);
    m_pJustDodge = std::make_unique<JustDodge>(m_pOwner);
    m_CurrentActiveState = std::ref(*m_pIdle.get());
}

void Root::Update()
{
    m_CurrentActiveState.get().Update();
}

void Root::LateUpdate()
{
    m_CurrentActiveState.get().LateUpdate();
}

void Root::Draw()
{
    m_CurrentActiveState.get().Draw();
}
void Root::Exit()
{
    m_CurrentActiveState.get().Exit();
}

// ステートの変更.
void Root::ChangeState(PlayerState::eID id)
{
    // 強制割り込みステート.
    if (id == PlayerState::eID::KnockBack ||
        id == PlayerState::eID::Pause ||
        id == PlayerState::eID::Dead ||
        id == PlayerState::eID::SpecialAttack)
    {
        std::reference_wrapper<PlayerStateBase> newStateRef = GetPlayer()->GetStateReference(id);

        // 現在と違うステートなら変更.
        if (&m_CurrentActiveState.get() != &newStateRef.get())
        {
            m_CurrentActiveState.get().Exit();
            m_CurrentActiveState = newStateRef;
            m_CurrentActiveState.get().Enter();
            return;
        }
        return;
    }

    try {
        std::reference_wrapper<PlayerStateBase> newStateRef = GetPlayer()->GetStateReference(id);

        // 現在と違うステートなら変更.
        if (&m_CurrentActiveState.get() != &newStateRef.get())
        {
            m_CurrentActiveState.get().Exit();
            m_CurrentActiveState = newStateRef;
            m_CurrentActiveState.get().Enter();
            return;
        }
        return;
    }
    catch (const std::exception& e) {
        std::string message = "Root::ChangeState failed (ID: " + std::to_string(static_cast<int>(id)) + "): " + e.what();
        Log::GetInstance().Warning("Pllayerステートの変更エラー", message.c_str());
        throw;
    }
}

#pragma region GetStateRef

//----------System----------.
// ポーズステートの取得.
std::reference_wrapper<PlayerStateBase> Root::GetPauseStateRef()
{
    return std::ref(*m_pPause.get());
}

// スタンステートの取得.
std::reference_wrapper<PlayerStateBase> Root::GetKnockBackStateRef()
{
    return std::ref(*m_pKnockBack.get());
}

// 死亡ステートの取得.
std::reference_wrapper<PlayerStateBase> Root::GetDeadStateRef()
{
    return std::ref(*m_pDead.get());
}

// 必殺技ステートの取得.
std::reference_wrapper<PlayerStateBase> Root::GetSpecialAttackStateRef()
{
    return std::ref(*m_pSpecialAttack.get());
}

//----------Movement----------.
// 待機ステートの取得.
std::reference_wrapper<PlayerStateBase> Root::GetIdleStateRef()
{
    return std::ref(*m_pIdle.get());
}

// 走りステートの取得.
std::reference_wrapper<PlayerStateBase> Root::GetRunStateRef()
{
    return std::ref(*m_pRun.get());
}

//----------Combat----------.
// 攻撃1段目ステートの取得.
std::reference_wrapper<PlayerStateBase> Root::GetCombo0StateRef()
{
    return std::ref(*m_pCombo_0.get());
}

// 攻撃2段目ステートの取得.
std::reference_wrapper<PlayerStateBase> Root::GetCombo1StateRef()
{
    return std::ref(*m_pCombo_1.get());
}

// 攻撃3段目ステートの取得.
std::reference_wrapper<PlayerStateBase> Root::GetCombo2StateRef()
{
    return std::ref(*m_pCombo_2.get());
}

// パリィステートの取得.
std::reference_wrapper<PlayerStateBase> Root::GetParryStateRef()
{
    return std::ref(*m_pParry.get());
}

//----------Dodge----------.
// 回避ステートの取得.
std::reference_wrapper<PlayerStateBase> Root::GetDodgeExecuteStateRef()
{
    return std::ref(*m_pDodgeExecute.get());
}

// ジャスト回避ステートの取得.
std::reference_wrapper<PlayerStateBase> Root::GetJustDodgeStateRef()
{
    return std::ref(*m_pJustDodge.get());
}

#pragma endregion

} // PlayerState.
