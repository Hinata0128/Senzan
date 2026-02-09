#pragma once

#include "System//Utility//StateMachine//StateBase.h"
#include "Game\01_GameObject\00_MeshObject\00_Character\02_Boss\Boss.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Utility/FileManager/FileManager.h"
#include "System/Utility/Math/Easing/Easing.h"

#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <filesystem>
#include <algorithm>

/*****************************************************************
*    ボスの攻撃ベースクラス(基底クラス).
**/

//前方宣言.
class Time;
class ColliderBase;
class GameObject;
class Animator;

// 攻撃中の移動（踏み込み）を制御する
struct MovementWindow
{
    float Start = 0.0f;    // 開始秒
    float Duration = 0.2f; // 継続時間
    float Speed = 10.0f;   // 移動速度
    bool IsAct = false;    // 内部フラグ（移動中か）

    MyEasing::Type EasingType = MyEasing::Type::Liner;
    float Distance = 1.0f; // 移動量係数（Speed * Duration に乗算）

    // 追加: このウィンドウで方向オフセットを使うか（XZ 平面）
    bool UseDirectionOffset = false;
    // 方向オフセット（度数法、XZ 平面での回転角）
    float DirectionOffsetDeg = 0.0f;

    // ランタイム用: 開始/終了位置と初期化フラグ（保存対象ではない）
    DirectX::XMFLOAT3 StartPos{ 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 EndPos{ 0.0f, 0.0f, 0.0f };
    bool Initialized = false;
    DirectX::XMFLOAT3 LastEasedPos{ 0.0f, 0.0f, 0.0f };

    void Reset() { IsAct = false; Initialized = false; }
};





// 当たり判定の発生時間を制御する
struct ColliderWindow
{
    std::string BoneName;  // ボーン名（現在未使用、将来のボーン追跡用）
    float Start = 0.0f;    // 開始秒
    float Duration = 0.1f; // 持続秒
    
    // 座標オフセット（Bossのローカル座標系: X=横, Y=上, Z=前方）
    DirectX::XMFLOAT3 Offset{ 0.0f, 0.0f, 0.0f };
    
    // ジャストタイム（開始時間より何秒前からジャスト判定を有効にするか）
    float JustTime = 0.0f;
    
    bool IsAct = false;       // 内部フラグ（判定開始済みか）
    bool IsEnd = false;       // 内部フラグ（判定終了済みか）
    bool IsJustWindow = false; // ジャスト判定フラグ（Start - JustTime ～ Start の間 true）
    bool JustPlayed = false; // 内部フラグ：ジャスト時演出を一度だけ再生する
    
    // 当たり判定を1度だけ有効にするか (true = 到達時に1フレームだけ有効)
    bool OneShot = true;
    // ランタイム用: OneShot アクティベートが行われたことを示す内部フラグ
    bool OneShotTriggered = false;

    void Reset() { IsAct = false; IsEnd = false; IsJustWindow = false; JustPlayed = false; OneShotTriggered = false; }
};

// エフェクト再生のタイミングを制御する
struct EffectWindow
{
    std::string EffectName;  // エフェクトリソース名
    float Start = 0.0f;      // 再生開始秒
    
    // 座標オフセット（Bossのローカル座標系: X=横, Y=上, Z=前方）
    DirectX::XMFLOAT3 Offset{ 0.0f, 0.0f, 0.0f };
    
    float Scale = 1.0f;      // エフェクトのスケール
    
    bool IsPlayed = false;   // 内部フラグ（再生済みか）

    void Reset() { IsPlayed = false; }
};

class BossAttackStateBase
    : public StateBase<Boss>

{
public:
    BossAttackStateBase(Boss* owner);
    virtual ~BossAttackStateBase() = default;

    //最初に入る.
    virtual void Enter() override;
    //動作.
    virtual void Update() override;
    //かかなくていい.
    virtual void LateUpdate() override {};
    //描画.
    virtual void Draw() override {};
    //終わるときに一回だけ入る.
    virtual void Exit() override; 

    // 攻撃ステート調整用UI（必要なステートで呼ぶ）
    virtual void DrawImGui();

    // JSON 設定の読み書き
    virtual void LoadSettings();
    virtual void SaveSettings() const;
    // シリアライズ用（派生が追加フィールドを返すためにオーバーライド可）
    virtual nlohmann::json SerializeSettings() const;
    // 各ステートで上書きしてファイル名を変更する
    virtual std::filesystem::path GetSettingsFileName() const { return std::filesystem::path("BossAttackState_Base.json"); }

    // ステート遷移を許可するか (デバッグ停止時は拒否)
    bool CanChangeState() const override { return !m_IsDebugStop; }

    // PlayerのParry成功時硬直させたいアニメーションとタイミング.
    virtual std::pair<Boss::enBossAnim, float> GetParryAnimPair(); 

    void SetNextAttackCansel();
    // --- Player を見る（Yaw のみ）ヘルパー ---
    // 一度だけ即座にプレイヤーの方を見る
    void FacePlayerInstantYaw();
    // 毎フレームプレイヤーの方を向き続ける（呼び出し側が毎フレーム呼ぶ）
    void FacePlayerYawContinuous();

    // Enter 時に自動でプレイヤーを向くか（デフォルト true）
    bool m_AutoFaceOnEnter = true;

protected:
    // --- 共通タイムベース制御 ---
    float m_CurrentTime = 0.0f;    // ステートに入ってからの経過時間（秒）
    float m_AnimSpeed = 1.0f;      // 実行時の現在アニメ速度（倍率）

    // --- ステート設定値 ---
    float m_EndTime = 2.0f;        // ステート全体の時間（秒）
    float m_ChargeTime = 0.0f;     // 溜め終了時間（秒）
    float m_AttackTime = 0.0f;     // 攻撃終了時間（秒）

    // 当たり判定・移動・エフェクトウィンドウ（可変長）
    std::vector<ColliderWindow>  m_ColliderWindows;  // 当たり判定設定
    std::vector<MovementWindow>  m_MovementWindows;  // 移動（踏み込み）設定
    std::vector<EffectWindow>    m_EffectWindows;    // エフェクト再生設定

    void UpdateBaseLogic(float dt); // 共通更新ロジック
    // 共通のコライダーウィンドウ更新ヘルパー
    // currentTime: 時間基準（秒）
    // windows: 更新対象のウィンドウ配列（m_ColliderWindows など）
    // 戻り値: anyJust フラグ
    bool UpdateColliderWindows(float prevTime, float currentTime, std::vector<ColliderWindow>& windows);

    // 派生クラスが移動の終点を独自に決めたい場合にオーバーライドする
    virtual DirectX::XMFLOAT3 ComputeMovementEndPos(const MovementWindow& mv, const DirectX::XMFLOAT3& startPos, const DirectX::XMFLOAT3& targetPos) const;

protected:
    //メンバ変数を作成.
    //アニメーションに必要なメンバ変数
    DirectX::XMFLOAT3            m_BonePos;      // ボーン座標
    LPD3DXANIMATIONCONTROLLER   AnimCtrl;
    std::shared_ptr<Transform> m_pTransform;

    // --- 共通攻撃パラメータ（設定値） ---
    float m_AttackAmount = 0.0f;   // 攻撃力（ダメージ）
    float m_AttackRange = 0.0f;    // 攻撃レンジ
    float m_ColliderWidth = 0.0f;  // 当たり判定の幅（半径）
    float m_ColliderHeight = 0.0f; // 当たり判定の高さ

    // 統合コライダーに使用するボーン名（派生クラスで設定）
    std::string m_AttackBoneName;

    // フェーズ用アニメ速度（倍率）
    float m_AnimSpeedCharge = 1.0f; // 溜め
    float m_AnimSpeedAttack = 1.0f; // 攻撃中
    float m_AnimSpeedExit = 1.0f;   // 余波

    // 各フェーズの遷移方法フラグ: true = アニメ終了で遷移, false = 時間ベースで遷移
    bool m_TransitionOnAnimEnd_Charge = false; // 溜め -> 攻撃
    bool m_TransitionOnAnimEnd_Attack = false;  // 攻撃 -> 余韻
    bool m_TransitionOnAnimEnd_Exit = false;    // 余韻 -> Idol / 次状態

    bool  m_IsDebugStop = false;         // デバッグ用停止フラグ

    std::shared_ptr<ColliderBase> m_pHitCollider;
    std::vector<GameObject*>      m_HitTargets;

    DirectX::XMFLOAT3 m_MotionVelocity{ 0.0f, 0.0f, 0.0f };

    DirectX::XMFLOAT3 m_StartPos{ 0.0f, 0.0f, 0.0f }; // 攻撃開始位置（共通）
    // (no global registry) instances update Boss::IsJustWindow flag directly
    // 前フレーム時間記録（UpdateBaseLogic 内で使用）
    float m_PrevTime = 0.0f;
};
