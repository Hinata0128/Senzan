#pragma once
#include "System/Singleton/SingletonTemplate.h"
#include <DirectXMath.h>

class Player;
class Boss;

/**********************************************************************************
* @author    : 淵脇 未来.
* @date      : 2026/01/23.
* @brief     : パリィ管理クラス.
*              PlayerとBoss間のパリィ成功処理を仲介する.
**********************************************************************************/
class CombatCoordinator final
    : public Singleton<CombatCoordinator>
{
private:
    friend class Singleton<CombatCoordinator>;
    CombatCoordinator();

public:
    ~CombatCoordinator();

	// PlayerとBossの参照を設定（シーン初期化時に呼び出す）.
	void Initialize(Player* player, Boss* boss);

	// 参照をクリア（シーン終了時に呼び出す）.
	void Clear();

    // パリィ成功を通知（Playerから呼び出される）.
    // withDelay = true の場合、ボスのパリィはアニメ再生後に delay 秒待機する挙動になる。
    void OnParrySuccess(bool withDelay = false);
    // Called when player enters SpecialAttack state to coordinate boss behavior
    void Enter();

	// パリィ成功時のカメラ演出が完了したか.
	bool IsParryCameraFinished() const;
    void JustCancelAttackCollider();

    // ジャスト回避開始.
    // @param TimeScale: 使用されるワールド時間スケール (0.0 ~ 1.0 の値推奨).
    // Note: この関数はボス側の攻撃判定を無効化し、
    //       ワールド時間スケールを変更します (Time::SetWorldTimeScale を使用).
    void StartJustDodge(float TimeScale);
    // ジャスト回避終了: ワールド時間スケールを 1.0 に戻し、ロックオンを復帰します.
    void EndJustDodge();
    // ボスの攻撃判定を全てオフにする（プレイヤー側からのユーティリティ）
    void DisableBossAttackColliders();

    // ボスへダメージ.
    void DamageToBoss(float DamageAmount);
    // Notification that the last parried object was a snowball
    void NotifyParriedBySnowball();
    bool WasLastParriedBySnowball() const;
    void ClearLastParriedFlag();

    void HitSpecialAttackToBoss();
    // ボスの前方向を取得
    DirectX::XMFLOAT3 GetBossForward() const;
    // ボスをダウンさせる（SpecialAttack終了時）
    void ForceBossDown();
    // Entrypoints to control Player from external callers (Manager-level hooks)
    // These provide a simple interface to trigger player-side effects from CombatCoordinator.
    void StartPlayerJustDodge(float TimeScale);
    void EndPlayerJustDodge();
    void CancelPlayerAttackCollider();
private:
	Player* m_pPlayer = nullptr;
	Boss* m_pBoss = nullptr;
    bool m_LastParriedBySnowball = false;
};
