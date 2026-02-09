#include "JustDodgeEffect.h"
#include "System/Singleton/ResourceManager/EffectManager/EffekseerManager.h"
#include "Resource/Effect/EffectResource.h"
#include "System/Singleton/CameraManager/CameraManager.h"
#include "System/Utility/Math/Math.h"
#include <algorithm>

JustDodgeEffect::JustDodgeEffect()
    : m_Handle(-1)
    , m_StartPos{0.0f, 0.0f, 0.0f}
    , m_TargetPos{0.0f, 0.0f, 0.0f}
    , m_CurrentPos{0.0f, 0.0f, 0.0f}
    , m_Scale(1.0f)
    , m_Duration(1.0f)
    , m_ElapsedTime(0.0f)
    , m_IsFinished(false)
{
}

JustDodgeEffect::~JustDodgeEffect()
{
    Stop();
}

void JustDodgeEffect::Start(const std::string& effectName, const DirectX::XMFLOAT3& worldStart, const DirectX::XMFLOAT3& worldTarget, float scale, float duration)
{
    Stop();
    m_EffectName = effectName;
    m_StartPos = worldStart;
    m_TargetPos = worldTarget;
    m_CurrentPos = worldStart;
    m_Scale = scale;
    m_Duration = (duration > 0.0f) ? duration : 1.0f;
    m_ElapsedTime = 0.0f;
    m_IsFinished = false;

    auto effect = EffectResource::GetResource(effectName);
    if (effect == nullptr) return;

    // Play at start position
    m_Handle = EffekseerManager::GetInstance().GetManager()->Play(effect, m_StartPos.x, m_StartPos.y, m_StartPos.z);
    if (m_Handle != -1)
    {
        if (!MyMath::IsNearlyEqual(scale, 1.0f))
        {
            EffekseerManager::GetInstance().GetManager()->SetScale(m_Handle, scale, scale, scale);
        }
        // 向きを目標方向へ向ける (XZ 平面)
        float dx = m_StartPos.x - m_TargetPos.x;
        float dz = m_StartPos.z - m_TargetPos.z;
        if (std::fabs(dx) > 1e-6f || std::fabs(dz) > 1e-6f)
        {
            float yaw = std::atan2f(dx, dz) + DirectX::XM_PI;
            EffekseerManager::GetInstance().GetManager()->SetRotation(m_Handle, 0.0f, yaw, 0.0f);
        }
    }
}

void JustDodgeEffect::Update(float deltaTime)
{
    if (m_Handle == -1) return;
    
    // 経過時間を更新
    m_ElapsedTime += deltaTime;
    
    // 進行度を計算 (0.0 ~ 1.0)
    float t = m_ElapsedTime / m_Duration;
    t = std::clamp(t, 0.0f, 1.0f);
    
    // 線形補間で現在位置を計算: CurrentPos = StartPos + (TargetPos - StartPos) * t
    m_CurrentPos.x = m_StartPos.x + (m_TargetPos.x - m_StartPos.x) * t;
    m_CurrentPos.y = m_StartPos.y + (m_TargetPos.y - m_StartPos.y) * t;
    m_CurrentPos.z = m_StartPos.z + (m_TargetPos.z - m_StartPos.z) * t;
    
    // エフェクトの位置を更新
    auto mgr = EffekseerManager::GetInstance().GetManager();
    if (mgr != nullptr && mgr->Exists(m_Handle))
    {
        mgr->SetLocation(m_Handle, m_CurrentPos.x, m_CurrentPos.y, m_CurrentPos.z);
    }
    
    // エフェクトのシミュレーションを進める
    EffekseerManager::GetInstance().UpdateHandle(m_Handle);

    // 移動完了チェック
    if (t >= 1.0f)
    {
        m_IsFinished = true;
    }
    
    // エフェクトが終了したかチェック
    if (mgr == nullptr || !mgr->Exists(m_Handle))
    {
        m_Handle = -1;
    }
}

void JustDodgeEffect::Stop()
{
    if (m_Handle == -1) return;
    auto mgr = EffekseerManager::GetInstance().GetManager();
    if (mgr != nullptr) {
        mgr->SetShown(m_Handle, false);
    }
    m_Handle = -1;
    m_IsFinished = true;
}

void JustDodgeEffect::Draw()
{
    if (m_Handle == -1) return;
    auto* camera = CameraManager::GetInstance().GetCurrentCamera().get();
    EffekseerManager::GetInstance().RenderHandle(m_Handle, camera);
}
