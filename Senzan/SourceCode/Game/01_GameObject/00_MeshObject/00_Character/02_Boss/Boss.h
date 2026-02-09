#pragma once

#include "Game/01_GameObject/00_MeshObject/00_Character/Character.h"
#include "System/Utility/Transform/Transform.h"

#include "System/Utility/StateMachine/StateMachine.h"
#include "Game\\01_GameObject\\00_MeshObject\\00_Character\\01_Player\\Player.h"

#include <string>

////ステートマシンクラスの前方宣言.
//template<typename FSM_Owner> class StateMachine;

//=====================================================================
// ボスクラスにプレイヤーの位置を入手させるためにここに前方宣言を書く.
// Update関数の中にプレイヤーのポジションを設定する.
//=====================================================================

class SlashState;        //斬撃ステートクラス.

class SlashCharge;       //チャージ斬撃クラス.
class ChargeSlashState;  //チャージ斬撃ステートクラス.

class Shout;             //叫びクラス.
class ShoutState;        //叫び攻撃ステートクラス.

//ボスの行動関係を書く.
class BossIdolState;        //待機状態.
class BossMoveState;        //左右移動動状態.
class BossAttackStateBase;  //攻撃ベースクラス.
class BossStompState;       //踏みつけ攻撃
class BossSlashState;       //斬る攻撃.
class BossChargeSlashState; //溜め攻撃.
class BossShoutState;       //叫び攻撃.
class BossJumpOnlState;
class BossLaserState;
class BossDeadState;
class BossMoveContinueState;

class BossThrowingState;
class BossSpinningState;

class BossChargeState;
class BossParryState;
class BossSpecialDamageState;

class SkinMesh;

/***********************************************************************
*    ボスキャラクラス.
**/
class Boss
    : public Character
{
    friend BossIdolState;
    friend BossMoveState;
    friend BossStompState;
    friend BossSlashState;
    friend BossAttackStateBase;
    friend BossChargeSlashState;
    friend BossShoutState;
    friend BossJumpOnlState;
    friend BossLaserState;
    friend BossDeadState;
    friend BossChargeState;
    friend BossThrowingState;
    friend BossSpinningState;
    friend BossParryState;
    friend BossMoveContinueState;
    friend BossSpecialDamageState;

    //ボスのアニメーションの列挙.
    enum class enBossAnim : byte
    {
        Idol = 0,        //待機.

        RunToIdol,       //走りから待機.
        Run,             //走り中.
        IdolToRun,       //待機から走り.

        Hit,             //被弾.

        ChargeToIdol,    //ため攻撃から待機.
        ChargeAttack,    //ため攻撃中.
        Charge,          //ためている.

        RightMove,       //右に進.
        LeftMove,        //左に進.

        SpecialToIdol,   //特殊攻撃と踏みつけ終了時に待機.
        Special_1,       //特殊攻撃と踏みつけ中.
        Special_0,       //飛ぶ.

        FlinchToIdol,    //怯みから待機.
        Flinch,          //怯み中.
        FlinchParis,     //怯み(パリィ).

        Dead,            //死亡.

        LaserEnd,        //レーザーから待機.
        Laser,           //レーザー中.
        LaserCharge,     //レーザーのため.

        SlashToIdol,     //斬る攻撃から待機.
        Slash,           //斬る攻撃.

        none,            //何もしない.
    };

public:
Boss();
~Boss() override;

// HPを倍率で設定（コンテニュー時に使用）
void SetHPMultiplier(float multiplier);

void SetAnyAttackJustWindow(bool v) { m_IsAnyAttackJustWindow = v; }
    bool IsAnyAttackJustWindow() const { return m_IsAnyAttackJustWindow; }

    // パリィ被弾通知（外部から呼び出す）。
    // withDelay=true の場合、指定秒数だけパリィ後に次フェーズへ移行する挙動になる。
    void OnParried();
    void OnParried(bool withDelay);

    void OnSpecial();

    // パリィ被弾フラグを取得.
    bool IsParried() const { return m_IsParried; }

    // パリィ時にアニメーションを停止する秒数を取得
    std::pair<Boss::enBossAnim, float> GetParryPauseSeconds() const { return m_ParryPauseSeconds; }

    // パリィで再生するアニメと停止（参照）するアニメ番号を返す
    // first  = 再生開始アニメ
    // second = パリィで停止させたいアニメ（参照用）
    virtual void GetParryAnimPair();

    void Update() override;
    void LateUpdate() override;
    void Draw() override;

    void Init();

    //ステートクラスからStateMachineにアクセスする.
    StateMachine<Boss>* GetStateMachine();

    //アニメーション再生時に必要になるGet関数になっている.
    LPD3DXANIMATIONCONTROLLER GetAnimCtrl() const;

    void Hit(float damage);

    // 文字列でコライダーを操作できるようにする
    void SetColliderActiveByName(const std::string& name, bool active);


public:
    //プレイヤーの位置を取得するためにここにSetPlayer()を作成する.
    void SetTargetPos(const DirectX::XMFLOAT3 Player_Pos);

    DirectX::XMFLOAT3 GetTargetPos() { return m_PlayerPos; }

    // 全攻撃判定オフ.
    void OffAttackCollider();

    // 次の攻撃判定を非アクティブ登録する.
    void SetNextAttackCansel();

    // Playerロックオン.
    void SetLockOnPlayer(bool IslockOnPlayer) { m_IslockOnPlayer = IslockOnPlayer; }
protected:

    // 衝突_被ダメージ.
    void HandleDamageDetection() override;
    // 衝突_攻撃判定.
    void HandleAttackDetection() override;

    //当たり判定を取得する.
    //通常攻撃.
    ColliderBase* GetSlashCollider() const;
    //ジャンプ攻撃.
    ColliderBase* GetStompCollider() const;
    //叫び攻撃.
    ColliderBase* GetShoutCollider() const;
    // 回転攻撃用コライダー
    ColliderBase* GetSpinningCollider() const;
    // レーザー攻撃用コライダー
    ColliderBase* GetLaserCollider() const;

    // 統合された攻撃コライダー（各ステートがボーンを指定して使用）
    ColliderBase* GetAttackCollider() const { return m_spAttackCollider; }

    /*************************************************************
    * @brief	ボスのワールド行列と掛け合わせてボーンのワールド行列を作成し、
    *           指定コライダーの位置オフセットと外部 Transform ポインタを更新.
    *           回転情報が必要ない場合は updateRotation=false を渡してください。
    *           rotationOffset を渡すとボーン回転へ追加の回転を適用できます。
    * @param[in]	boneName	：取得するボーン名.
    * @param[in]	collider	：更新対象のコライダー.
    * @param[in]	outTransform：ワールド Transform を格納するキャッシュ参照.
    * @param[in]	updateRotation：true の場合 outTransform の回転/スケールも更新する（デフォルト true）
    * @param[in]	rotationOffset：ボーン回転に乗算するクォータニオン回転オフセット（デフォルト: 単位クォータニオン）
    * @return	true = 成功, false = 取得失敗または引数不正
    * ************************************************************/
    bool UpdateColliderFromBone(
        const std::string& boneName,
        ColliderBase* collider,
        Transform& outTransform,
        bool updateRotation = true,
        const DirectX::XMFLOAT4& rotationOffset = DirectX::XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f });

protected:
    //ステートマシンのメンバ変数.
    std::unique_ptr<StateMachine<Boss>> m_State;

    DirectX::XMFLOAT3               m_PlayerPos;
    DirectX::XMFLOAT3               m_PlayerVelocity;

    float m_MoveSpped = 0.0f;

    float m_TurnSpeed;
    float m_MoveSpeed;

    D3DXVECTOR3 m_vCurrentMoveVelocity;

    ColliderBase* m_pAttackCollider;     // 攻撃判定.

    float deleta_time;

    //当たり判定のメンバ変数.
    ColliderBase* m_pSlashCollider;
    ColliderBase* m_pStompCollider;
    ColliderBase* m_pShoutCollider;
    ColliderBase* m_pSpinningCollider;
    ColliderBase* m_pLaserCollider;

    // runtime flag indicating any attack's just window is active
    bool m_IsAnyAttackJustWindow = false;

    // パリィ被弾フラグ.
    bool m_IsParried = false;

    // パリィ時にアニメーションを停止する秒数（デザインで手打ち可）
    std::pair<Boss::enBossAnim, float> m_ParryPauseSeconds = {};

    // 統合された攻撃コライダー（各ステートがボーンを指定）
    ColliderBase* m_spAttackCollider = nullptr;
    std::string m_AttackBoneName;  // 現在追従するボーン名
    LPD3DXFRAME m_pAttackBoneFrame = nullptr;  // ボーンフレームキャッシュ
    Transform m_AttackBoneWorldTransform;      // ワールドTransformキャッシュ

    // エフェクトハンドルは Character 側で管理する（Character::PlayEffect を使用）

    bool m_IslockOnPlayer = true;
};

