#pragma once

#include "Game/02_Camera/CameraBase.h"

/// 簡易一人称カメラ（Unity の FPS カメラ風）
class FirstPersonCamera final
    : public CameraBase
{
public:
    FirstPersonCamera();
    virtual ~FirstPersonCamera() override;

    // 毎フレーム更新
    virtual void Update() override;

    // 設定
    void SetEyeHeight(float height) noexcept;
    void SetMoveSpeed(float speed) noexcept;
    void SetSprintMultiplier(float mul) noexcept;

private:
    float m_MoveSpeed;         // 歩行速度
    float m_SprintMultiplier;  // スプリント倍率
    float m_EyeHeight;         // カメラ高さ（Y）
    float m_LookDistance;      // 注視点までの距離（内部的に使用）

    // 加速用
    float m_AccelTimerHorizontal; // 水平移動の継続時間
    float m_AccelTimerVertical;   // 垂直移動の継続時間
    float m_AccelRate;            // 加速係数
    float m_MaxAccelMul;         // 最大倍率
};
