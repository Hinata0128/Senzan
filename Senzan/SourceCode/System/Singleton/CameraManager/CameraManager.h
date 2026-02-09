#pragma once
#include "System/Singleton/SingletonTemplate.h"
#include "Game/02_Camera/CameraBase.h"

class LockOnCamera;

/************************************
*   カメラ管理クラス.
************************************/
class CameraManager final
	: public Singleton<CameraManager>
{
private:
	friend class Singleton<CameraManager>;
	CameraManager();
public:
	~CameraManager();

	// カメラの接続.
	inline void SetCamera(std::shared_ptr<CameraBase> pCamera) noexcept {
		if (pCamera == nullptr) { return; }
		m_wpCamera = pCamera;
	}

	// カメラを揺らす.
	void ShakeCamera(float duration, float magnitude);

	// パリィカメラ演出を開始.
	void StartParryCamera();

	// 必殺技カメラ演出を開始（ボスをロックオン）.
	void StartSpecialCamera();
	// 必殺技カメラ演出を終了.
	void EndSpecialCamera();
	// 最終ポーズカメラ演出を開始.
	void StartFinalPoseCamera();

	// カメラの更新.
	void LateUpdate();

	// 座標を取得.
	inline DirectX::XMFLOAT3 GetPosition() const noexcept { return m_wpCamera.lock()->GetPosition(); }

	// 座標を設定.
	 void SetPosition(DirectX::XMFLOAT3 Position);
	 void SetPosition(float x, float y, float z);

	// 注視点を取得.
	inline DirectX::XMFLOAT3 GetLook() const noexcept { return m_wpCamera.lock()->GetLook(); }

	// 注視点を設定.
	void SetLook(DirectX::XMFLOAT3 Position);
	void SetLook(float x, float y, float z);

	// 視線の方向を取得.
	inline DirectX::XMFLOAT3 GetLookDirection() const noexcept { return m_wpCamera.lock()->GetLookDirection(); }

	// 現在のカメラを取得.
	std::shared_ptr<CameraBase> GetCurrentCamera() const noexcept { return m_wpCamera.lock(); }

	// ビュー行列を取得.
	inline const DirectX::XMMATRIX& GetViewMatrix() const noexcept { return m_wpCamera.lock()->GetViewMatrix(); }

	// プロジェクション行列を取得.
	inline const DirectX::XMMATRIX& GetProjMatrix() const noexcept { return m_wpCamera.lock()->GetProjMatrix(); }

	// プロジェクション行列を取得.
	inline const DirectX::XMMATRIX& GetViewProjMatrix() const noexcept { return m_wpCamera.lock()->GetViewProjMatrix(); }

private:

	// ビューとプロジェクションの更新.
	void ViewAndProjectionUpdate();


private:
	std::weak_ptr<CameraBase> m_wpCamera;

	float m_ShakeDuration = 0.0f;       // シェイクの残り時間.
	float m_ShakeMagnitude = 0.0f;      // シェイクの強度.
};
