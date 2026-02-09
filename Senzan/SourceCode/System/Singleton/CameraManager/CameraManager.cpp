#include <random>     
#include <chrono>

#include "CameraManager.h"
#include "Game/02_Camera/CameraBase.h"
#include "Game/02_Camera/LockOnCamera/LockOnCamera.h"
#include "Game/04_Time/Time.h"

static std::mt19937 s_Engine(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));

CameraManager::CameraManager()
	: m_wpCamera        ()
    , m_ShakeDuration   ()
    , m_ShakeMagnitude  ()
{
}

CameraManager::~CameraManager()
{
}

// カメラを揺らす.
void CameraManager::ShakeCamera(float duration, float magnitude)
{
	// 既に揺れている場合でも、新しいシェイクで上書きする。
	m_ShakeDuration = duration;
	m_ShakeMagnitude = magnitude;
}

// パリィカメラ演出を開始.
void CameraManager::StartParryCamera()
{
	if (m_wpCamera.expired()) { return; }

	// LockOnCameraにキャストしてパリィカメラ演出を開始.
	auto lockOnCamera = std::dynamic_pointer_cast<LockOnCamera>(m_wpCamera.lock());
	if (lockOnCamera)
	{
		lockOnCamera->StartParryCamera();
	}
}

// 必殺技カメラ演出を開始.
void CameraManager::StartSpecialCamera()
{
	if (m_wpCamera.expired()) { return; }

	auto lockOnCamera = std::dynamic_pointer_cast<LockOnCamera>(m_wpCamera.lock());
	if (lockOnCamera)
	{
		lockOnCamera->StartSpecialCamera();
	}
}

// 必殺技カメラ演出を終了.
void CameraManager::EndSpecialCamera()
{
	if (m_wpCamera.expired()) { return; }

	auto lockOnCamera = std::dynamic_pointer_cast<LockOnCamera>(m_wpCamera.lock());
	if (lockOnCamera)
	{
		lockOnCamera->EndSpecialCamera();
	}
}

// 最終ポーズカメラ演出を開始.
void CameraManager::StartFinalPoseCamera()
{
	if (m_wpCamera.expired()) { return; }

	auto lockOnCamera = std::dynamic_pointer_cast<LockOnCamera>(m_wpCamera.lock());
	if (lockOnCamera)
	{
		lockOnCamera->StartFinalPoseCamera();
	}
}

void CameraManager::LateUpdate()
{
    if (m_wpCamera.expired()) { return; }

    std::shared_ptr<CameraBase> camera = m_wpCamera.lock();

    DirectX::XMFLOAT3 shake_offset = { 0.0f, 0.0f, 0.0f };

	// シェイク処理.
    if (m_ShakeDuration > 0.0f)
    {
		// シェイク時間を減少.
        m_ShakeDuration = std::max(0.0f, m_ShakeDuration - Time::GetInstance().GetDeltaTime());

        float currentMagnitude = m_ShakeMagnitude * (m_ShakeDuration / m_ShakeMagnitude);
       
        // 乱数.
        std::uniform_real_distribution<float> dist(-currentMagnitude, currentMagnitude);

        shake_offset.x = dist(s_Engine);
        shake_offset.y = dist(s_Engine);
        shake_offset.z = dist(s_Engine);
    }

    // 揺れをセット.
    camera->SetShakeOffset(shake_offset);

    camera->Update();
}

void CameraManager::SetPosition(DirectX::XMFLOAT3 Position)
{
	if (m_wpCamera.expired()) { return; }
	m_wpCamera.lock()->SetPosition(Position);
}

void CameraManager::SetPosition(float x, float y, float z)
{
	if (m_wpCamera.expired()) { return; }
	m_wpCamera.lock()->SetPosition(DirectX::XMFLOAT3(x,y,z));
}

void CameraManager::SetLook(DirectX::XMFLOAT3 Position)
{
	if (m_wpCamera.expired()) { return; }
	m_wpCamera.lock()->SetLook(Position);
}

void CameraManager::SetLook(float x, float y, float z)
{
	if (m_wpCamera.expired()) { return; }
	m_wpCamera.lock()->SetLook(DirectX::XMFLOAT3(x,y,z));
}

void CameraManager::ViewAndProjectionUpdate()
{
	if (m_wpCamera.expired()) { return; }
	m_wpCamera.lock()->ViewAndProjectionUpdate();
}
