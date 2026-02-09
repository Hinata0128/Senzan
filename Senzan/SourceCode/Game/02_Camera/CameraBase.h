#pragma once

/******************************
*	CameraBaseクラス
******************************/
class CameraBase
{
public:
	CameraBase();
	virtual ~CameraBase();

	virtual void Update() = 0;

	/****************************************************
	* @breif ビューとプロジェクションの更新.
	****************************************************/
	void ViewAndProjectionUpdate();


	/****************************************************
	* @breif 指定座標が視錐体内にあるか.
	* @param point：判定したい座標.
	****************************************************/
	bool IsPointInFrustum(const DirectX::XMFLOAT3& point);

	bool VFCulling(const DirectX::XMFLOAT3& Position, const float radius);


public: // Getter、Setter.

	/****************************************************
	* @breif 視点を取得・設定.
	* @note GetPosition：取得.
	* @note SetPosition：設定.
	****************************************************/
	const DirectX::XMFLOAT3& GetPosition() const;
	void SetPosition(const DirectX::XMFLOAT3& Position);


	/****************************************************
	* @breif  注視点を取得・設定.
	****************************************************/
	const DirectX::XMFLOAT3& GetLook() const;
	void SetLook(const DirectX::XMFLOAT3& look);


	/****************************************************
	* @breif  視線の方向を取得.
	****************************************************/
	const DirectX::XMFLOAT3 GetLookDirection();


	/****************************************************
	* @breif  ビュー行列を取得
	****************************************************/
	const DirectX::XMMATRIX& GetViewMatrix() const;


	/****************************************************
	* @breif プロジェクション行列を取得.
	****************************************************/
	const DirectX::XMMATRIX& GetProjMatrix() const;


	/****************************************************
	* @breif ビュー・プロジェクションの合成行列を取得.
	****************************************************/
	const DirectX::XMMATRIX GetViewProjMatrix() const;

	/****************************************************
	* @breif 前方向ベクトルを取得.
	****************************************************/
	const DirectX::XMFLOAT3 GetForwardVec() const;


	/****************************************************
	* @breif 右方向ベクトルを取得.
	****************************************************/
	const DirectX::XMFLOAT3 GetRightVec() const;


	/****************************************************
	* @breif Yawを取得.
	****************************************************/
	const float& GetYaw() const;


	/****************************************************
	* @breif Pitchを取得.
	****************************************************/
	const float& GetPitch() const;

	// シェイクオフセットの設定.
	void SetShakeOffset(const DirectX::XMFLOAT3& offset);

	// FOVの設定・取得（ラジアン）.
	void SetFOV(float fovRadians);
	float GetFOV() const;

protected:

	/****************************************************
	* @breif ビュー(カメラ)変換.
	****************************************************/
	void ViewUpdate();


	/****************************************************
	* @breif プロジェクション(射影)変換.
	****************************************************/
	void ProjectionUpdate();


	/****************************************************
	* @breif 移動方向ベクトルを計算する.
	****************************************************/
	void CalculateMoveVector();

	/****************************************************
	* @breif 視錐体を計算する.
	****************************************************/
	std::vector<DirectX::XMFLOAT4> CalcFrustum();

	// カメラシェイクを適用する.
	void ApplyCameraShake();

protected:
	Transform		  m_spTransform;
	DirectX::XMFLOAT3 m_LookPos;	// 注視点.
	DirectX::XMFLOAT3 m_ShakeOffset; 
	float			  m_Distance;

	DirectX::XMFLOAT3 m_ForwardVec; // カメラの前方ベクトル
	DirectX::XMFLOAT3 m_RightVec;   // カメラの右方ベクトル

	DirectX::XMMATRIX m_View;		//　ビュー(カメラ)行列.
	DirectX::XMMATRIX m_Proj;		//　プロジェクション(射影)行列.

	DirectX::XMFLOAT2 m_MouseSensitivity;// マウス感度.
	DirectX::XMFLOAT2 m_ControllerSensitivity;// マウス感度.
	float			  m_Yaw;
	float			  m_Pitch;
};
