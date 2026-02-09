#pragma once
#include "../CameraBase.h"

/**********************************************************************************
* @author    : 淵脇 未来.
* @date      : 2026/01/02.
* @brief     : ロックオンカメラクラス.
*              プレイヤーとターゲット（ボス）間の注視制御を提供します.
**********************************************************************************/
class Player;
class Boss;

class LockOnCamera : public CameraBase
{
public:
    // カメラモード.
    enum class eCameraMode
    {
        Normal,     // 通常モード.
        Parry,      // パリィ演出モード(下から潜り込む).
        Special,    // 必殺技.
    };

public:
    LockOnCamera(const Player& player, const Boss& target);
    virtual ~LockOnCamera() = default;

    virtual void Update() override;

    // フォロースピードを設定.
    void SetFollowSpeed(float speed) { m_FollowSpeed = speed; }
    // カメラとプレイヤー間の距離を設定.
    void SetDistance(float dist) { m_Distance = dist; }
    // カメラの高さオフセットを設定.
    void SetHeightOffset(float height) { m_HeightOffset = height; }
    // 注視点の高さオフセットを設定.
    void SetLookOffset(float offset) { m_LookOffset = offset; }
    // 注視点補間値を設定.
    void SetLookLerp(float lerp) { m_LookLerp = lerp; }


    // パリィカメラ演出開始.
    void StartParryCamera();
    // パリィカメラ演出終了（通常モードへ復帰）.
    void EndParryCamera();
    // パリィカメラ演出中か.
    bool IsParryCameraActive() const { return m_CameraMode == eCameraMode::Parry; }

    // 必殺技カメラ演出開始（ボスをロックオン）.
    void StartSpecialCamera();
    // 必殺技カメラ演出終了（通常モードへ復帰）.
    void EndSpecialCamera();
    // 必殺技カメラ演出中か.
    bool IsSpecialCameraActive() const { return m_CameraMode == eCameraMode::Special; }
    // 最終ポーズカメラ開始（プレイヤーの左下から見上げる）
    void StartFinalPoseCamera();

    // 設定の保存/読み込み
    void SaveSettings() const;
    void LoadSettings();
    std::filesystem::path GetSettingsFileName() const { return std::filesystem::path("LockOnCamera.json"); }

private:
    // 通常カメラ更新.
    void UpdateNormalCamera(float dt);
    // パリィカメラ更新.
    void UpdateParryCamera(float dt);
    // 必殺技カメラ更新.
    void UpdateSpecialCamera(float dt);

private:
    std::reference_wrapper<const Player> m_rPlayer;
    std::reference_wrapper<const Boss>   m_rTarget;

    float m_FollowSpeed;  // 追従速度.
    float m_HeightOffset; // 高さオフセット (ワールド単位).
    float m_LookOffset;   // 注視点の高さオフセット (ワールド単位).
    float m_LookLerp;     // 注視点補間 (0.0=プレイヤー, 1.0=ボス).

    // パリィカメラ演出用.
    eCameraMode m_CameraMode = eCameraMode::Normal;
    float m_ParryTime = 0.0f;           // パリィ演出経過時間.
    // パリィ演出時間を3フェーズに分割:
    // m_ParryPhaseInDuration: カメラが目標地点へ移動する時間
    // m_ParryHoldDuration: 目標地点で留まる時間
    // m_ParryOutDuration: 元に戻る時間
    float m_ParryPhaseInDuration = 0.3f;
    float m_ParryHoldDuration = 0.2f;
    float m_ParryOutDuration = 0.3f;
    float m_ParryHeightOffset = 1.0f;   // パリィ時の低い高さ.
    float m_ParryLookOffset = 6.0f;     // パリィ時の注視点高さ（見上げる）.
    float m_ParryDistance = 8.0f;       // パリィ時の距離（近づく）.
    // パリィ時の移動オフセット（ワールド単位）
    // m_ParryHorizontalOffset: プレイヤーの左右方向へのオフセット（正は右）
    // m_ParryForwardOffset: プレイヤーの前方（ボス方向）への追加前進距離（正は前）
    float m_ParryHorizontalOffset = 0.0f;
    float m_ParryForwardOffset = 0.0f;

    // 明示的な目標/注視点モード
    bool m_UseExplicitTarget = false; // true の時は m_ExplicitCameraPosOffset / m_ExplicitLookPosOffset を使用
    DirectX::XMFLOAT3 m_ExplicitCameraPosOffset = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 m_ExplicitLookPosOffset = { 0.0f, 0.0f, 0.0f };

    void SetUseExplicitTarget(bool v) { m_UseExplicitTarget = v; }
    bool IsUsingExplicitTarget() const { return m_UseExplicitTarget; }
    void SetExplicitCameraPosOffset(const DirectX::XMFLOAT3& off) { m_ExplicitCameraPosOffset = off; }
    void SetExplicitLookPosOffset(const DirectX::XMFLOAT3& off) { m_ExplicitLookPosOffset = off; }

    // 必殺技カメラ演出用.
    float m_SpecialLookLerp = 1.0f;     // 必殺技時の注視点補間（1.0=ボス完全ロック）.
    DirectX::XMFLOAT3 m_SpecialFixedCameraPos = { 0.0f, 0.0f, 0.0f }; // 発動時の固定カメラ位置
    DirectX::XMFLOAT3 m_SpecialFixedLookPos = { 0.0f, 0.0f, 0.0f };   // 発動時の固定注視点
    
    // 必殺技カメラ弧移動用
    float m_SpecialArcAngle = 0.0f;       // 現在の弧の角度
    float m_SpecialArcRadius = 20.0f;     // ボスからの距離
    float m_SpecialArcHeight = 8.0f;      // カメラの高さ
    float m_SpecialArcSpeed = 0.8f;       // 弧移動の速度（ラジアン/秒）
    
    // 最終ポーズカメラ用
    bool m_IsFinalPoseCamera = false;     // 最終ポーズカメラ中か
    float m_FinalPoseLeftOffset = 5.0f;   // 左オフセット
    float m_FinalPoseDownOffset = 2.0f;   // 下オフセット
    float m_FinalPoseBackOffset = 3.0f;   // 後ろオフセット
    float m_FinalPoseLookUpOffset = 3.0f; // 注視点高さオフセット

    // FOV control for parry and special effects (radians).
    // Defaults: 50deg and 40deg in radians
    float m_DefaultFOV = 0.872664626f; // 50deg
    float m_ParryFOV = 0.698131700f;   // 40deg
    float m_CurrentFOV = 0.872664626f; // current active FOV
    
    // Getter/Setter for FOV
    void SetDefaultFOV(float radians) { m_DefaultFOV = radians; }
    float GetDefaultFOV() const { return m_DefaultFOV; }
    void SetParryFOV(float radians) { m_ParryFOV = radians; }
    float GetParryFOV() const { return m_ParryFOV; }

    // パリィが雪玉由来かを示すフラグ（雪玉のときはXZを固定してYのみ更新）
    bool m_ParryBySnowball = false;
};
