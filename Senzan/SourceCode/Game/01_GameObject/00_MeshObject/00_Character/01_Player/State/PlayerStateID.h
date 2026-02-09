#pragma once
/**************************************************
*	プレイヤーのステートIDクラス.
*	担当 : 淵脇 未来.
**/
namespace PlayerState {
enum class eID
{
    None = 0, // 未初期化、または無効なステートID.

    // System.
    Pause,         // System/Pause          : 一時停止.
    KnockBack,     // System/KnockBack      : 被ダメノックバッグ.
    Dead,          // System/Dead           : 死亡
    SpecialAttack, // System/SpecialAttack  : 必殺技.

    // Gameplay/Movement.
    Idle,          // Gameplay/Movement/Idle: 無操作.
    Run,           // Gameplay/Movement/Run : 走り.

    // Gameplay/Dodge.
    DodgeExecute,  // Gameplay/Dodge/DodgeExecute   : 通常回避.
    JustDodge,     // Gameplay/Dodge/JustDodge      : ジャスト回避.

    // Gameplay/Combat.
    AttackCombo_0, // Gameplay/Combat/AttackCombo_0 : 攻撃一段階目.
    AttackCombo_1, // Gameplay/Combat/AttackCombo_1 : 攻撃二段階目.
    AttackCombo_2, // Gameplay/Combat/AttackCombo_2 : 攻撃三段階目.
    Parry,         // Gameplay/Combat/Parry         : ジャストガード.

    _Max
};
}