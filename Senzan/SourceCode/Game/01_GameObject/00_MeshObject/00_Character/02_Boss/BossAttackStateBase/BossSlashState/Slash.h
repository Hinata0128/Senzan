#pragma once

#include "..//BossAttackStateBase.h"
#include "Game/01_GameObject/00_MeshObject/MeshObject.h"
#include "Game/03_Collision/00_Core/01_Capsule/CapsuleCollider.h"

/*******************************************************************************
*	ボスの攻撃： 斬る攻撃.
*	このクラスは斬るアニメーションの再生をして攻撃力等をjsonファイルに保存させる.
*	担当者: 西村 日向
**/

//前方宣言.
class SkinMesh;
class BossIdolState;

class Slash final
	: public BossAttackStateBase
{
public:
    //斬る攻撃タイミングの取得用
    //列挙型.
    //メモリの節約のために1バイト0~255に制限している.
    enum class enSlash : unsigned char
    {
        none,			//何もしない.
        ChargeSlash,	//チャージ.
        SlashAttack,	//斬る攻撃.
        SlashIdol,		//斬る攻撃から待機.
    };

public:
	Slash(Boss* owner);
	~Slash();

    //最初に入る.
    void Enter() override;
    //動作関数(毎フレーム).
    void Update() override;
    //すべてのUpdate()が終わった時に入るUpdate().
    void LateUpdate() override;
    //描画(レーザーのエフェクトを表示させる).
    void Draw() override;
    //終了時に入る.
    void Exit() override;

    // PlayerのParry成功時硬直させたいアニメーションとタイミング.
    //パリィ成功時の設定.
    std::pair<Boss::enBossAnim, float> GetParryAnimPair() override;

    //デバッグ中にImGuiでLaserのステータスを変更できるように設定する.
    void DrawImGui() override;
    //Laserの攻撃のパラメータ用のjsonファイルの読み込み.
    void LoadSettings() override;
    //Laserの攻撃のパラメータ用のjsonファイルの保存.
    void SaveSettings() const override;
    //変更させるjsonファイルのポスを設定.
    std::filesystem::path GetSettingsFileName() const override { return std::filesystem::path("Slash.json"); }


private:
    // ホーミング停止秒数（m_StateTimer がこの値を超えるまでだけ追尾）
    float m_HomingEndTime = 0.2f;

    enSlash m_State;

};
