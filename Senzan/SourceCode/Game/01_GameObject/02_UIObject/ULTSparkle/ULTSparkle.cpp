#include "ULTSparkle.h"
#include "02_UIObject/UILoader/UILoader.h"

#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Game/04_Time/Time.h"
#include "Math/Easing/Easing.h"
#include "Utility/Math/Random/Random.h"
#include "ImGui/CImGuiManager.h"

ULTSparkle::ULTSparkle(std::shared_ptr<UIObject> pObje)
    : m_pMainSparkle    ( pObje )
    , m_IsAnimating     ( false )
    , m_Timer           ( 0.0f )
    , m_Duration        ( 0.8f )
    , m_StartScale      ( 0.0f )
    , m_PeakScale       ( 0.6f )
    , m_ToPeakTime      ( 0.3f )
    , m_PeakTime        ( 0.6f )
    , m_StartRotSpead   ( 10.0f )
    , m_PeakRotSpead    ( 2.0f )
    , m_EndRotSpead     ( 7.0f )

    , m_IsGaugeMax      ( false )
    , m_SpawnTimer      ( 0.0f )
    , m_SpawnInterval   ( 0.4f )
    , m_ParticlesStartAlpha( 0.7f )
    , m_ParticlesDecAlpha( 1.0f )
    , m_ParticlesSize   ( 0.05f )
    , m_ParticlesAngle  ( 1.6f )
    , m_ParticlesSpead  ( 60.0f )
{
    Create();
}

//----------------------------------------------------------------.

ULTSparkle::~ULTSparkle()
{
}

//----------------------------------------------------------------.

void ULTSparkle::Create()
{
    m_pMainSparkle->SetAlpha(0.0f);
    m_pMainSparkle->SetScale({ 0.0f,0.0f,0.0f });
}

//----------------------------------------------------------------.

void ULTSparkle::DoPeakAnim()
{
    m_IsAnimating = true;
    m_Timer = 0.0f;

}

//----------------------------------------------------------------.

void ULTSparkle::SetULTGaugeStatus(bool isMax, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT2 size)
{
    m_IsGaugeMax = isMax;
    m_GaugeX = { pos.x - (size.x / 2),pos.x + (size.x / 2) };
    m_GaugeY = { pos.y - (size.y / 2),pos.y + (size.y / 2) };
}

//----------------------------------------------------------------.

void ULTSparkle::UpDateParticles(float dt)
{
    if (m_IsGaugeMax)
    {
        m_SpawnTimer += dt;
        if (m_SpawnTimer >= m_SpawnInterval)
        {
            m_SpawnTimer = 0.0f;

            auto pNewUI = std::make_shared<UIObject>();
            pNewUI->AttachSprite(m_pMainSparkle->GetSprite());

            float x = MyRand::GetRandomPercentage(m_GaugeX.x, m_GaugeX.y);
            float y = MyRand::GetRandomPercentage(m_GaugeY.x, m_GaugeY.y);

            pNewUI->SetPosition({ x, y, 0.0f });

            pNewUI->SetScale({ m_ParticlesSize, m_ParticlesSize, 1.0f });
            pNewUI->SetColor(m_pMainSparkle->GetColor());
            pNewUI->SetAlpha(m_ParticlesStartAlpha);

            SparkleParticle particle;
            particle.ui = pNewUI;

            // ランダム方向（-1 ～ 1）
            float dirX = MyRand::GetRandomPercentage(-m_ParticlesAngle, m_ParticlesAngle);
            float dirY = MyRand::GetRandomPercentage(-m_ParticlesAngle, m_ParticlesAngle);

            // 正規化
            float len = sqrtf(dirX * dirX + dirY * dirY);
            if (len != 0.0f)
            {
                dirX /= len;
                dirY /= len;
            }

            // 速度（調整用）
            float speed = MyRand::GetRandomPercentage(0.0f, m_ParticlesSpead);

            particle.velocity = { dirX * speed, dirY * speed };

            m_Particles.push_back(particle);
        }
    }

    auto it = m_Particles.begin();
    while (it != m_Particles.end())
    {
        auto& particle = *it;
        auto& pUI = particle.ui;

        // ---- 移動 ----
        auto pos = pUI->GetPosition();
        pos.x += particle.velocity.x * dt;
        pos.y += particle.velocity.y * dt;
        pUI->SetPosition(pos);

        // ---- フェード ----
        float alpha = pUI->GetAlpha();
        alpha -= m_ParticlesDecAlpha * dt;
        pUI->SetAlpha(alpha);

        if (alpha <= 0.0f)
        {
            it = m_Particles.erase(it);
        }
        else
        {
            pUI->Update();
            ++it;
        }
    }
}

//----------------------------------------------------------------.

void ULTSparkle::Update()
{
    float dt = Time::GetInstance().GetUnscaledDeltaTime();
    UpDateParticles(dt);

    if (m_IsAnimating)
    {
        m_Timer += dt;
        float duration = 1.f; // 全体時間.
        float progress = m_Timer / duration;
        if (progress > 1.0f) progress = 1.0f;

        float currentScale = 0.0f;
        float currentAlpha = 0.0f;
        float rot = m_pMainSparkle->GetRotation().z;

        if (progress < m_ToPeakTime)
        {
            // --- 1. 拡大フェーズ ---.
            float t = progress / m_ToPeakTime;
            // 拡大.
            MyEasing::UpdateEasing(MyEasing::Type::OutCirc, t, 1.0f, 0.0f, m_PeakScale, currentScale);
            MyEasing::UpdateEasing(MyEasing::Type::OutQuint, t, 1.0f, 0.0f, 1.0f, currentAlpha);
            // 回転.
            rot += m_StartRotSpead * dt;
        }
        else if (progress < m_PeakTime)
        {
            // --- 2. 静止維持フェーズ ---.
            currentScale = m_PeakScale;
            currentAlpha = 1.0f;
            // 維持中もゆっくり回し続ける.
            float t = (progress - m_ToPeakTime) / (m_PeakTime - m_ToPeakTime);
            rot += m_PeakRotSpead * dt;
        }
        else
        {
            // --- 3. 急加速縮小フェーズ ---.
            float t = (progress - m_PeakTime) / (1.0f - m_PeakTime);
            // InExpoで吸い込まれるように一瞬で消す.
            MyEasing::UpdateEasing(MyEasing::Type::InCirc, t, 1.0f, m_PeakScale, 0.0f, currentScale);
            MyEasing::UpdateEasing(MyEasing::Type::InExpo, t, 1.0f, 1.0f, 0.0f, currentAlpha);
            rot += m_EndRotSpead * dt;
        }

        m_pMainSparkle->SetRotation({ 0, 0, rot });
        m_pMainSparkle->SetScale({ currentScale, currentScale, 1.0f });
        m_pMainSparkle->SetAlpha(currentAlpha);

        if (progress >= 1.0f)
        {
            m_IsAnimating = false;
            m_pMainSparkle->SetAlpha(0.0f);
        }
    }
    m_pMainSparkle->Update();
}

//----------------------------------------------------------------.

void ULTSparkle::LateUpdate()
{
}

//----------------------------------------------------------------.

void ULTSparkle::Draw()
{
    DirectX11::GetInstance().SetDepth(false);

    // パーティクルを全部描画
    for (auto& part : m_Particles) {
        part.ui->Draw();
    }

    // 主張演出を描画
    m_pMainSparkle->Draw();

    DirectX11::GetInstance().SetDepth(true);
}
