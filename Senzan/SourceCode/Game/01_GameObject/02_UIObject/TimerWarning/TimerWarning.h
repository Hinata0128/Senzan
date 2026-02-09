#pragma once
#include "02_UIObject/UIObject.h"

/*********************************************
*	Timer危険演出クラス.
**/

class TimerWarning
{
public:
    TimerWarning(std::shared_ptr<UIObject> pBaseUI);
    ~TimerWarning();

    void Create();
    void Update(float ratio);
    void Draw();

private:
    // ランダムスケールを再設定する関数
    void SetRandomScales(bool high);
    // ランダムスケールを再設定する関数
    void SetRandomColors();
private:
    struct ChildCircle {
        std::shared_ptr<UIObject> ui;
        float angle         = 0.0f;
        float currentScale  = 0.0f;
        float randomScale   = 0.0f;
    };

    std::shared_ptr<UIObject> m_pBaseUI;
    std::vector<ChildCircle> m_Circles;

    // 演出用定数
    const int m_CircleCount;
    const DirectX::XMFLOAT3 m_TargetPos;
    const DirectX::XMFLOAT2 m_TargetPivot;

    float m_GlobalRotation; // 0.8以降で使用する回転値

    // アニメーション制御用
    float m_Phase1AnimetionTimer;
    float m_Phase2AnimetionTimer;
    float m_Phase3AnimetionTimer;
    float m_Phase1AnimetionTime;
    float m_Phase2AnimetionTime;
    float m_Phase3AnimetionTime;

    float m_Phase1Time;
    float m_Phase2Time;
    float m_Phase3Time;

    // フェーズ切り替わり検知用
    bool m_Phase1Triggered;
    bool m_Phase2Triggered;
    bool m_Phase3Triggered;

    bool m_Phase1Peaked;
    bool m_Phase2Peaked;
    bool m_Phase3Peaked;

};
