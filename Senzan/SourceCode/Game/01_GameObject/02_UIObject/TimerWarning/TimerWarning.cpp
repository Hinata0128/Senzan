#include "TimerWarning.h"
#include "Game/04_Time/Time.h"
#include "Math/Easing/Easing.h"
#include "Utility/Math/Random/Random.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"


TimerWarning::TimerWarning(std::shared_ptr<UIObject> pBaseUI)
    : m_pBaseUI             (pBaseUI)
    , m_CircleCount         (32)
    , m_TargetPos           (547.0f, -130.0f, -10.0f)
    , m_TargetPivot         (0.5f, -0.35f)
    , m_GlobalRotation      (0.0f)
    , m_Phase1AnimetionTimer(0.0f)
    , m_Phase2AnimetionTimer(0.0f)
    , m_Phase3AnimetionTimer(0.0f)
    , m_Phase1AnimetionTime (15.0f * Time::GetInstance().GetDeltaTime())
    , m_Phase2AnimetionTime (15.0f * Time::GetInstance().GetDeltaTime())
    , m_Phase3AnimetionTime (25.0f * Time::GetInstance().GetDeltaTime())
    , m_Phase1Time          (0.333f)
    , m_Phase2Time          (0.666f)
    , m_Phase3Time          (0.95f)
    , m_Phase1Triggered     (false)
    , m_Phase2Triggered     (false)
    , m_Phase3Triggered     (false)
    , m_Phase1Peaked        (false)
    , m_Phase2Peaked        (false)
    , m_Phase3Peaked        (false)
{
}

TimerWarning::~TimerWarning() {}

void TimerWarning::Create()
{
    m_Circles.clear();
    for (int i = 0; i < m_CircleCount; ++i)
    {
        ChildCircle child;
        child.ui = std::make_shared<UIObject>();
        child.ui->AttachSprite(m_pBaseUI->GetSprite());

        child.ui->SetPosition(m_TargetPos);
        // AnchorではなくPivotに設定 (ユーザー指摘に基づき)
        child.ui->SetPivot(m_TargetPivot);

        // ラジアンで均等配置 (0 ～ 6.28)
        child.angle = (6.28f / (float)m_CircleCount) * i;
        child.ui->SetRotation({ 0, 0, child.angle });

        child.ui->SetScale({ 0.0f, 0.0f, 1.0f });
        child.ui->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });

        m_Circles.push_back(child);
    }
}
void TimerWarning::Update(float ratio)
{float dt = Time::GetInstance().GetUnscaledDeltaTime();

    // 1. フェーズ管理 (ループの外でタイマーを更新)
    if (ratio >= m_Phase1Time && ratio <= m_Phase2Time) {
        m_Phase1AnimetionTimer += dt;
        if (!m_Phase1Triggered) {
            SetRandomScales(true);
            SetRandomColors();
            m_Phase1Triggered = true;
            m_Phase1Peaked = false; // 初期化
        }
    } else {
        m_Phase1Triggered = false;
        m_Phase1AnimetionTimer = 0.0f;
    }

    if (ratio >= m_Phase2Time && ratio <= m_Phase3Time) {
        m_Phase2AnimetionTimer += dt;
        if (!m_Phase2Triggered) {
            SetRandomScales(true);
            SetRandomColors();
            m_Phase2Triggered = true;
            m_Phase2Peaked = false;
        }
    } else {
        m_Phase2Triggered = false;
        m_Phase2AnimetionTimer = 0.0f;
    }
    if (ratio >= m_Phase3Time) {
        m_Phase3AnimetionTimer += dt;
        SetRandomScales(false);
        if (!m_Phase3Triggered) {
            SetRandomColors();
            m_Phase3Triggered = true;
        }
    }

    float maxSpeed = 6.28f * 1.5f;
    float minSpeed = 6.28f * 0.0f;
    float accelAlpha = (ratio - m_Phase3Time) / 0.1f;
    float rotSpeed = minSpeed + (maxSpeed - minSpeed) * accelAlpha * dt;
    m_GlobalRotation += rotSpeed;

    // 3. 各サークルの更新
    for (int i = 0; i < m_CircleCount; ++i)
    {
        auto& circle = m_Circles[i];
        DirectX::XMFLOAT3 currentColor = { 1.0f, 1.0f, 1.0f };
        float targetScale = 0.0f;
        float rotOffset = 0.0f;
        float animT = 0.0f;

        // --- Phase 1: 黄色 ---
        if (m_Phase1Time <= ratio && ratio <= m_Phase2Time)
        {
            if (!m_Phase1Peaked) {
                MyEasing::UpdateEasing(MyEasing::Type::OutQuint, m_Phase1AnimetionTimer, m_Phase1AnimetionTime, 0.0f, 1.0f, animT);
                if (m_Phase1AnimetionTimer >= m_Phase1AnimetionTime) {
                    m_Phase1Peaked = true;
                    m_Phase1AnimetionTimer = 0.0f; // 収束用にリセット
                }
            } else {
                MyEasing::UpdateEasing(MyEasing::Type::InQuint, m_Phase1AnimetionTimer, m_Phase1AnimetionTime, 1.0f, 0.0f, animT);
            }

            currentColor = { m_Phase3Time, m_Phase3Time, 0.0f }; // アルファにanimTを反映
            targetScale = circle.randomScale * animT;
            rotOffset = m_GlobalRotation * 0.f;
        }
        // --- Phase 2: 赤色 ---
        else if (m_Phase2Time <= ratio && ratio <= m_Phase3Time)
        {
            if (!m_Phase2Peaked) {
                MyEasing::UpdateEasing(MyEasing::Type::OutQuint, m_Phase2AnimetionTimer, m_Phase2AnimetionTime, 0.0f, 1.0f, animT);
                if (m_Phase2AnimetionTimer >= m_Phase2AnimetionTime) {
                    m_Phase2Peaked = true;
                    m_Phase2AnimetionTimer = 0.0f;
                }
            } else {
                MyEasing::UpdateEasing(MyEasing::Type::InQuint, m_Phase2AnimetionTimer, m_Phase2AnimetionTime, 1.0f, 0.0f, animT);
            }

            currentColor = { m_Phase3Time, 0.0f, 0.0f };
            targetScale = circle.randomScale * animT;
            rotOffset = m_GlobalRotation * 0.f;
        }
        // --- Phase 3: 最終警告 ---
        else if (m_Phase3Time <= ratio && i % 2 == 0)
        {
            MyEasing::UpdateEasing(MyEasing::Type::OutQuint, m_Phase3AnimetionTimer, m_Phase3AnimetionTime, 0.0f, 1.0f, animT);

            // 点滅速度（高いほど速い）
            float flashSpeed = 25.0f;
            // -1.0 ～ 1.0 のサイン波を 0.0 ～ 1.0 に変換
            float colorPulse = (sinf(m_GlobalRotation * flashSpeed) + 1.0f) * 0.5f;

            // 赤(0.9, 0, 0) と ほぼ黒(0.1, 0, 0) の間で補間
            // Lerp(a, b, t) = a + (b - a) * t
            currentColor.x = 0.6f + (0.8f * colorPulse);
            currentColor.y = 0.0f;
            currentColor.z = 0.0f;

            targetScale = circle.randomScale * animT;
            rotOffset = m_GlobalRotation;
        }


        float stretch = 0.f, scaleX = targetScale, scaleY = targetScale;
        if (!m_Phase3Triggered) {
            stretch = 0.8f;
            scaleX = targetScale;
            scaleY = targetScale * (1.0f + animT * (circle.randomScale * stretch));
        }

        circle.ui->SetScale({ scaleX, scaleY, 1.0f });
        circle.ui->SetColor({ currentColor.x, currentColor.y, currentColor.z, circle.ui->GetAlpha() });
        circle.ui->SetRotation({ 0, 0, circle.angle + rotOffset });
        circle.ui->Update();
    }
}

void TimerWarning::Draw()
{
    DirectX11::GetInstance().SetDepth(false);

    for (auto& circle : m_Circles)
    {
        circle.ui->Draw();
    }

    DirectX11::GetInstance().SetDepth(true);
}


void TimerWarning::SetRandomScales(bool high)
{
    for (auto& circle : m_Circles)
    {
        if (high) {
            circle.randomScale = MyRand::GetRandomPercentage(0.6f, 1.4f);
        }
        else {
            circle.randomScale = MyRand::GetRandomPercentage(0.6f, 0.7f);
        }
    }
}


void TimerWarning::SetRandomColors()
{
    for (auto& circle : m_Circles)
    {
        circle.ui->SetAlpha(MyRand::GetRandomPercentage(0.6f, 1.0f));
    }
}
