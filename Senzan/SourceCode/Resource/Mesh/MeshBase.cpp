#include "MeshBase.h"
#include "Graphic/DirectX/DirectX9/DirectX9.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Graphic/Shader/Base/VertexShaderBase.h"
#include "Graphic/Shader/Base/PixelShaderBase.h"

MeshBase::MeshBase()
	: m_pDevice9(nullptr)
	, m_pDevice11(nullptr)
	, m_pContext11(nullptr)
	, m_pVertexShader(std::make_unique<VertexShaderBase>())
	, m_pPixelShader(std::make_unique<PixelShaderBase>())
	, m_pSampleLinear(nullptr)
	, m_pCBufferPerMesh(nullptr)
	, m_pCBufferPerMaterial(nullptr)
	, m_pCBufferPerFrame(nullptr)
	, m_spTransform()
	, m_Quaternion()
	, m_WorldMatrix()
	, m_ResourceName()
	, m_OriginalRadius(0.0f)
	, m_Radius(0.0f)
	, m_IsLight(true)
	, m_IsShadow(true)
{
}

//--------------------------------------------------------------------------------------

MeshBase::~MeshBase()
{
}

//--------------------------------------------------------------------------------------

void MeshBase::SetPosition(const DirectX::XMFLOAT3& Position)
{
	m_spTransform.Position = Position;
}

//--------------------------------------------------------------------------------------

void MeshBase::SetRotation(const DirectX::XMFLOAT3& Rotation)
{
	m_spTransform.Rotation = Rotation;
}

//--------------------------------------------------------------------------------------

void MeshBase::SetQuaternion(const DirectX::XMFLOAT4& quaternion)
{
	m_Quaternion = quaternion;
}

//--------------------------------------------------------------------------------------

void MeshBase::SetScale(const DirectX::XMFLOAT3& Scale)
{
	m_spTransform.Scale = Scale;
}

//--------------------------------------------------------------------------------------

void MeshBase::SetIsLight(const bool& isLight)
{
	m_IsLight = isLight;
}

//--------------------------------------------------------------------------------------

void MeshBase::SetIsShadow(const bool& isShadow)
{
	m_IsShadow = isShadow;
}

//--------------------------------------------------------------------------------------

const std::string& MeshBase::GetResourceName() const
{
	return m_ResourceName;
}

//--------------------------------------------------------------------------------------

void MeshBase::CalcWorldMatrix()
{
	// ワールド行列、スケール行列、回転行列、平行移動行列.
	DirectX::XMMATRIX mScale, mRot, mTran;

	// 拡大縮小行列作成.
	mScale = DirectX::XMMatrixScaling(
		m_spTransform.Scale.x,
		m_spTransform.Scale.y,
		m_spTransform.Scale.z);

	// 平行移動行列作成.
	mTran = DirectX::XMMatrixTranslation(
		m_spTransform.Position.x,
		m_spTransform.Position.y,
		m_spTransform.Position.z);

	// 個別の回転行列を計算 (D3DXからXMへ)
	DirectX::XMMATRIX mYaw, mPitch, mRoll;
	mYaw = DirectX::XMMatrixRotationY(m_spTransform.Rotation.y); // Yaw (Y軸)
	mPitch = DirectX::XMMatrixRotationX(m_spTransform.Rotation.x); // Pitch (X軸)
	mRoll = DirectX::XMMatrixRotationZ(m_spTransform.Rotation.z); // Roll (Z軸)

	// 回転行列を合成 (Yaw * Pitch * Roll の順)
	// 【D3DXからXMへ】行列乗算をXMMatrixMultiplyに置き換え
	mRot = DirectX::XMMatrixMultiply(mYaw, mPitch);
	mRot = DirectX::XMMatrixMultiply(mRot, mRoll);

	DirectX::XMMATRIX mWorldTemp = DirectX::XMMatrixMultiply(mScale, mRot);
	m_WorldMatrix = DirectX::XMMatrixMultiply(mWorldTemp, mTran);
}