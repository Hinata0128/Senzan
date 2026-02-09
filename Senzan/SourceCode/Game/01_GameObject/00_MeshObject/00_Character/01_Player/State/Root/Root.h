#pragma once

#include "../PlayerStateBase.h"
#include "../PlayerStateID.h"

class Player;

namespace PlayerState {
class Pause;
class KnockBack;
class Dead;
class SpecialAttack;
class Idle;
class Run;
class AttackCombo_0;
class AttackCombo_1;
class AttackCombo_2;
class Parry;
class DodgeExecute;
class JustDodge;
}

namespace PlayerState {
/**************************************************
*	プレイヤーの状態の最上位(Root).
*   各ステートの所有権と使用ステート切り替えをする.
*	担当:淵脇 未来.
**/

class Root final 
    : public PlayerStateBase
{
public:
    Root(Player* owner);
    ~Root();

    // ステートIDの取得.
    constexpr PlayerState::eID GetStateID() const override;

    void Enter() override;
    void Update() override;
    void LateUpdate() override;
    void Draw() override;
    void Exit() override;

    // ステートの変更.
    void ChangeState(PlayerState::eID id);

#pragma region GetStateRef

    //---System---.
    std::reference_wrapper<PlayerStateBase> GetPauseStateRef();         // ポーズステートの取得.
    std::reference_wrapper<PlayerStateBase> GetKnockBackStateRef();     // スタンステートの取得.
    std::reference_wrapper<PlayerStateBase> GetDeadStateRef();          // 死亡ステートの取得.
    std::reference_wrapper<PlayerStateBase> GetSpecialAttackStateRef(); // 必殺技ステートの取得.

    //---Movement---.
    std::reference_wrapper<PlayerStateBase> GetIdleStateRef();          // 待機ステートの取得.
    std::reference_wrapper<PlayerStateBase> GetRunStateRef();           // 移動ステートの取得.

    //---Combat---.
    std::reference_wrapper<PlayerStateBase> GetCombo0StateRef();        // 攻撃1段目ステートの取得.
    std::reference_wrapper<PlayerStateBase> GetCombo1StateRef();        // 攻撃2段目ステートの取得.
    std::reference_wrapper<PlayerStateBase> GetCombo2StateRef();        // 攻撃3段目ステートの取得.
    std::reference_wrapper<PlayerStateBase> GetParryStateRef();         // パリィステートの取得.

    //---Dodge---.
    std::reference_wrapper<PlayerStateBase> GetDodgeExecuteStateRef();  // 回避ステートの取得.
    std::reference_wrapper<PlayerStateBase> GetJustDodgeStateRef();     // ジャスト回避ステートの取得.
#pragma endregion
    
private:
    std::unique_ptr<Pause>          m_pPause;         // ポーズ状態.  
    std::unique_ptr<KnockBack>      m_pKnockBack;     // スタン状態.
    std::unique_ptr<Dead>           m_pDead;          // 死亡状態.
    std::unique_ptr<SpecialAttack>  m_pSpecialAttack; // 必殺技状態.
    std::unique_ptr<Idle>           m_pIdle;		  // 待機状態.
    std::unique_ptr<Run>            m_pRun;		      // 走り状態.
    std::unique_ptr<AttackCombo_0>  m_pCombo_0;       // 攻撃1段目.
    std::unique_ptr<AttackCombo_1>  m_pCombo_1;       // 攻撃2段目.
    std::unique_ptr<AttackCombo_2>  m_pCombo_2;       // 攻撃3段目.
    std::unique_ptr<Parry>          m_pParry;         // パリィ.
    std::unique_ptr<DodgeExecute>   m_pDodgeExecute;  // 回避状態.
    std::unique_ptr<JustDodge>      m_pJustDodge;     // ジャスト回避状態.

    // 現在の状態を保持.
    std::reference_wrapper<PlayerStateBase> m_CurrentActiveState;
};
}