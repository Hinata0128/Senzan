#pragma once

#include "..//BossAttackStateBase.h"
#include "Game/01_GameObject/00_MeshObject/MeshObject.h"
#include "Game/03_Collision/00_Core/01_Capsule/CapsuleCollider.h"

/*******************************************************************************
*	ボスの攻撃： 斬る攻撃.
*	このクラスは斬るアニメーションの再生をして攻撃力等をjsonファイルに保存させる.
*   作成日: 2026/03/03
*	担当者: 西村 日向
**/

//前方宣言.
class SkinMesh;
class BossIdolState;
class Boss;
class Player;

class Slash final
    : public BossAttackStateBase
{
public:
    // 斬る攻撃タイミングの取得用
    // 列挙型.
    // メモリの節約のために1バイト0~255に制限している.
    enum class enList : byte
    {
        none,
        ChargeSlash,    // チャージ
        SlashAttack,    // 斬る攻撃
        SlashIdol,      // 待機（後隙）
    };

public:
    Slash(Boss* owner);
    ~Slash();

    // 最初に入る.
    void Enter() override;
    // 動作関数(毎フレーム).
    void Update() override;
    // すべてのUpdate()が終わった時に入るUpdate().
    void LateUpdate() override;
    // 描画.
    void Draw() override;
    // 終了時に入る.
    void Exit() override;

    // PlayerのParry成功時硬直させたいアニメーションとタイミング.
    // パリィ成功時の設定.
    std::pair<Boss::enBossAnim, float> GetParryAnimPair() override;

    // デバッグ中にImGuiでステータスを変更できるように設定する.
    void DrawImGui() override;
    // パラメータ用のjsonファイルの読み込み.
    void LoadSettings() override;
    // パラメータ用のjsonファイルの保存.
    void SaveSettings() const override;
    // 変更させるjsonファイルのパスを設定.
    std::filesystem::path GetSettingsFileName() const override { return std::filesystem::path("Slash.json"); }

private:
    // ホーミング停止秒数
    float m_HomingEndTime = 0.2f;
    // 斬る攻撃の状態管理.
    enList m_State = enList::none;
    // SE再生フラグ
    bool m_IsSwingSoundPlayed = false;
    bool m_IsHitSoundPlayed = false;
};