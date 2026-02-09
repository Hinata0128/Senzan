#include "CameraBase.h"
#include "Game/05_InputDevice/Input.h"
#include <DirectXMath.h>

namespace {
	static constexpr DirectX::XMFLOAT2 MOUSE_SENSITIVITY = DirectX::XMFLOAT2(0.001f, 0.001f);
	static constexpr DirectX::XMFLOAT2 CONTROLLER_SENSITIVITY = DirectX::XMFLOAT2(0.2f, 0.2f);
	static constexpr float PITCH_LIMIT_RAD = D3DXToRadian(60.0f); 
	static constexpr float CAMERA_SPEED = 1.0f;
	static constexpr float NEAR_CLIP = 1.0f;
	static constexpr float FAR_CLIP = FLT_MAX;
	static constexpr float FOV_ANGLE_RAD = DirectX::XMConvertToRadians(50.0f);
	static constexpr float ASPECT = WND_WF / WND_HF;
}

// Module-scope FOV that can be changed at runtime by cameras.
static float g_FOV_ANGLE_RAD = FOV_ANGLE_RAD;

CameraBase::CameraBase()
	: m_spTransform			()
	, m_LookPos				({ 0.f, 0.f, 0.f })
	, m_ShakeOffset			({ 0.f, 0.f, 0.f })
	, m_Distance			(5.f)
	, m_ForwardVec			()
	, m_RightVec			()
	, m_View				()
	, m_Proj				()
	, m_MouseSensitivity	(MOUSE_SENSITIVITY)
	, m_ControllerSensitivity(CONTROLLER_SENSITIVITY)
	, m_Yaw					()
	, m_Pitch				()
{
}

CameraBase::~CameraBase()
{
}

//---------------------------------------------------------------------.

// ビューとプロジェクションの更新.
void CameraBase::ViewAndProjectionUpdate()
{
	ViewUpdate();
	ProjectionUpdate();
}

//---------------------------------------------------------------------.

bool CameraBase::IsPointInFrustum(const DirectX::XMFLOAT3& point)
{
	std::vector<DirectX::XMFLOAT4> frustum = CalcFrustum();
	DirectX::XMVECTOR vPoint = DirectX::XMLoadFloat3(&point);

	for (const auto& plane : frustum)
	{
		DirectX::XMVECTOR vPlane = DirectX::XMLoadFloat4(&plane);
		float distance = DirectX::XMVectorGetX(DirectX::XMPlaneDotCoord(vPlane, vPoint));
		if (distance < 0.0f) { return false; }
	}

	return true;
}

//---------------------------------------------------------------------.

bool CameraBase::VFCulling(const DirectX::XMFLOAT3& Position, const float radius)
{
	DirectX::XMVECTOR vPosition = DirectX::XMLoadFloat3(&Position);
	std::vector<DirectX::XMFLOAT4> frustum = CalcFrustum();

	for (const auto& plane : frustum)
	{
		DirectX::XMVECTOR vPlane = DirectX::XMLoadFloat4(&plane);
		float distance = DirectX::XMVectorGetX(DirectX::XMPlaneDotCoord(vPlane, vPosition));
		if (distance < -radius) // 半径を考慮して視錐台の外かチェック
		{
			return false;
		}
	}
	return true;
}

//---------------------------------------------------------------------.

const DirectX::XMFLOAT3& CameraBase::GetPosition() const
{
	return m_spTransform.Position;
}

//---------------------------------------------------------------------.

void CameraBase::SetPosition(const DirectX::XMFLOAT3& Position)
{
	m_spTransform.Position = Position;
}

//---------------------------------------------------------------------.

const DirectX::XMFLOAT3& CameraBase::GetLook() const
{
	return m_LookPos;
}

//---------------------------------------------------------------------.

void CameraBase::SetLook(const DirectX::XMFLOAT3& look)
{
	m_LookPos = look;
}

//---------------------------------------------------------------------.

const DirectX::XMFLOAT3 CameraBase::GetLookDirection()
{
	// XMVector3NormalizeはXMVECTORを返すため、XMStoreFloat3でXMFLOAT3に変換
	DirectX::XMVECTOR vLook = DirectX::XMLoadFloat3(&m_LookPos);
	DirectX::XMVECTOR vPosition = DirectX::XMLoadFloat3(&m_spTransform.Position);
	DirectX::XMVECTOR vDirection = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(vLook, vPosition));

	DirectX::XMFLOAT3 direction;
	DirectX::XMStoreFloat3(&direction, vDirection);
	return direction;
}

//---------------------------------------------------------------------.

const DirectX::XMMATRIX& CameraBase::GetViewMatrix() const
{
	return m_View;
}

//---------------------------------------------------------------------.

const DirectX::XMMATRIX& CameraBase::GetProjMatrix() const
{
	return m_Proj;
}

//---------------------------------------------------------------------.

const DirectX::XMMATRIX CameraBase::GetViewProjMatrix() const
{
	return m_View * m_Proj;
}

//---------------------------------------------------------------------.

const DirectX::XMFLOAT3 CameraBase::GetForwardVec() const
{
	return m_spTransform.GetForward();
}

//---------------------------------------------------------------------.

const DirectX::XMFLOAT3 CameraBase::GetRightVec() const
{
	return m_spTransform.GetRight();
}

//---------------------------------------------------------------------.

const float& CameraBase::GetYaw() const
{
	return m_Yaw;
}

//---------------------------------------------------------------------.

const float& CameraBase::GetPitch() const
{
	return m_Pitch;
}

// シェイクオフセットの設定.
void CameraBase::SetShakeOffset(const DirectX::XMFLOAT3& offset)
{
	m_ShakeOffset = offset;
}


//---------------------------------------------------------------------.

void CameraBase::ViewUpdate()
{
	DirectX::XMVECTOR v_position = DirectX::XMLoadFloat3(&m_spTransform.Position);
	DirectX::XMVECTOR v_look = DirectX::XMLoadFloat3(&m_LookPos);
	DirectX::XMVECTOR v_up = Axis::UP_VECTOR;
	DirectX::XMVECTOR v_shake_offset = DirectX::XMLoadFloat3(&m_ShakeOffset);

	v_position = DirectX::XMVectorAdd(v_position, v_shake_offset);

	m_View = DirectX::XMMatrixLookAtLH(v_position, v_look, v_up);

	// シェイクオフセットをリセット.
	m_ShakeOffset = { 0.0f, 0.0f, 0.0f };
}

//---------------------------------------------------------------------.

void CameraBase::ProjectionUpdate()
{
	// DirectXMathの関数でプロジェクション行列を更新
	m_Proj = DirectX::XMMatrixPerspectiveFovLH(
        g_FOV_ANGLE_RAD,
		ASPECT,
		NEAR_CLIP,
		FAR_CLIP);
}

void CameraBase::SetFOV(float fovRadians)
{
    g_FOV_ANGLE_RAD = fovRadians;
}

float CameraBase::GetFOV() const
{
    return g_FOV_ANGLE_RAD;
}

//---------------------------------------------------------------------.

void CameraBase::CalculateMoveVector()
{
	// クォータニオンの更新.
	DirectX::XMVECTOR v_Quaternion =
		DirectX::XMQuaternionRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f);
	DirectX::XMStoreFloat4(&m_spTransform.Quaternion, v_Quaternion);

	// 前方ベクトルの更新.
	DirectX::XMVECTOR forward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	DirectX::XMVECTOR v_rotated_forward = DirectX::XMVector3Rotate(forward, v_Quaternion);
	DirectX::XMStoreFloat3(&m_ForwardVec, v_rotated_forward);

	// 右方ベクトルの更新.
	DirectX::XMVECTOR right = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR v_rotated_right = DirectX::XMVector3Rotate(right, v_Quaternion);
	DirectX::XMStoreFloat3(&m_RightVec, v_rotated_right);

	// 座標の更新.
	DirectX::XMVECTOR v_look_pos = DirectX::XMLoadFloat3(&m_LookPos);
	DirectX::XMVECTOR v_back_vec = DirectX::XMVectorScale(v_rotated_forward, -m_Distance);
	DirectX::XMVECTOR v_new_camera_pos = DirectX::XMVectorAdd(v_look_pos, v_back_vec);
	DirectX::XMStoreFloat3(&m_spTransform.Position, v_new_camera_pos);
}

//---------------------------------------------------------------------.

std::vector<DirectX::XMFLOAT4> CameraBase::CalcFrustum()
{
	// 6つの平面を格納するためのvectorを初期化.
	std::vector<DirectX::XMFLOAT4> frustum(6);

	// ビュー行列とプロジェクション行列を乗算し、ビュープロジェクション行列を取得.
	DirectX::XMMATRIX viewProj = GetViewProjMatrix();

	// ビュープロジェクション行列の各列（ベクトル）を抽出.
	DirectX::XMVECTOR v_col1 = viewProj.r[0]; // 第1列.
	DirectX::XMVECTOR v_col2 = viewProj.r[1]; // 第2列.
	DirectX::XMVECTOR v_col3 = viewProj.r[2]; // 第3列.
	DirectX::XMVECTOR v_col4 = viewProj.r[3]; // 第4列.

	// 行列の列ベクトルを組み合わせて、視錐台の6つの平面を抽出.
	DirectX::XMVECTOR v_plane_left = DirectX::XMVectorAdd(v_col4, v_col1);
	DirectX::XMVECTOR v_plane_right = DirectX::XMVectorSubtract(v_col4, v_col1);
	DirectX::XMVECTOR v_plane_bottom = DirectX::XMVectorAdd(v_col4, v_col2);
	DirectX::XMVECTOR v_plane_top = DirectX::XMVectorSubtract(v_col4, v_col2);
	DirectX::XMVECTOR v_plane_near = v_col3;
	DirectX::XMVECTOR v_plane_far = DirectX::XMVectorSubtract(v_col4, v_col3);

	// 各平面ベクトルを正規化
	v_plane_left = DirectX::XMPlaneNormalize(v_plane_left);
	v_plane_right = DirectX::XMPlaneNormalize(v_plane_right);
	v_plane_bottom = DirectX::XMPlaneNormalize(v_plane_bottom);
	v_plane_top = DirectX::XMPlaneNormalize(v_plane_top);
	v_plane_near = DirectX::XMPlaneNormalize(v_plane_near);
	v_plane_far = DirectX::XMPlaneNormalize(v_plane_far);

	// 最後に、SIMDレジスタ内のXMVECTORをXMFLOAT4構造体に格納.
	DirectX::XMStoreFloat4(&frustum[0], v_plane_left);
	DirectX::XMStoreFloat4(&frustum[1], v_plane_right);
	DirectX::XMStoreFloat4(&frustum[2], v_plane_bottom);
	DirectX::XMStoreFloat4(&frustum[3], v_plane_top);
	DirectX::XMStoreFloat4(&frustum[4], v_plane_near);
	DirectX::XMStoreFloat4(&frustum[5], v_plane_far);

	return frustum;
}

void CameraBase::ApplyCameraShake()
{
}
