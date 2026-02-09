#include "EffekseerManager.h"
#include "Graphic\DirectX\DirectX11\DirectX11.h"
#include "Game/04_Time/Time.h"   
#include "Game/02_Camera/CameraBase.h"

namespace
{
	static constexpr int RENDER_SPRITE_MAX = 80000;

	//エフェクト管理用インスタンス最大数.
	static constexpr int EFFECT_INSTANCE_MAX = 8000;
}

EffekseerManager::EffekseerManager()
	: m_pManager	( nullptr )
	, m_pRenderer	( nullptr )
{
	// マネージャー、レンダラの初期化.
	Initialize();
}

//-----------------------------------------------------------------------------------.

EffekseerManager::~EffekseerManager()
{
	// マネージャー解放.
	m_pManager.Reset();

	// レンダラ解放.
	m_pRenderer.Reset();
}

//-----------------------------------------------------------------------------------.

void EffekseerManager::UpdateHandle(::Effekseer::Handle handle)
{
	// 更新.	
	if (handle == -1) {
		return; // ハンドルまたはカメラがない場合は描画しない.
	}
	m_pManager->BeginUpdate();
	m_pManager->UpdateHandle(handle, Time::GetInstance().GetDeltaTime() * 60.0f);
	m_pManager->EndUpdate();
}

//-----------------------------------------------------------------------------------.

void EffekseerManager::RenderHandle(::Effekseer::Handle handle, CameraBase* pUseCamera)
{
	if (handle == -1 || !pUseCamera) {
		return; // ハンドルまたはカメラがない場合は描画しない.
	}

	// ビュー行列を設定.
	m_pRenderer->SetCameraMatrix(ConvertToEfkMatrix(pUseCamera->GetViewMatrix()));

	// プロジェクション行列を設定.
	m_pRenderer->SetProjectionMatrix(ConvertToEfkMatrix(pUseCamera->GetProjMatrix()));
	
	// 描画.	
	m_pRenderer->BeginRendering();
	m_pManager->DrawHandle(handle);
	m_pRenderer->EndRendering();
}

//-----------------------------------------------------------------------------------.

void EffekseerManager::RenderHandleUI(::Effekseer::Handle handle)
{
	if (handle == -1) {
		return;
	}

	// 正射影行列（スクリーン座標系）を作成.
	// 左上原点、右下が (WND_W, WND_H) となる座標系.
	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX projMatrix = DirectX::XMMatrixOrthographicOffCenterLH(
		0.0f, static_cast<float>(WND_W),
		static_cast<float>(WND_H), 0.0f,
		-1000.0f, 1000.0f
	);

	// 行列を設定.
	m_pRenderer->SetCameraMatrix(ConvertToEfkMatrix(viewMatrix));
	m_pRenderer->SetProjectionMatrix(ConvertToEfkMatrix(projMatrix));

	// 描画.
	m_pRenderer->BeginRendering();
	m_pManager->DrawHandle(handle);
	m_pRenderer->EndRendering();
}

//-----------------------------------------------------------------------------------.

::Effekseer::Vector3D EffekseerManager::ConvertToEfkVector3(const DirectX::XMFLOAT3& vector3Dx)
{
	return ::Effekseer::Vector3D(vector3Dx.x, vector3Dx.y, vector3Dx.z);
}

//-----------------------------------------------------------------------------------.

DirectX::XMFLOAT3 EffekseerManager::ConvertToXMFloat3(const::Effekseer::Vector3D& vector3Efk)
{
	return DirectX::XMFLOAT3(vector3Efk.X, vector3Efk.Y, vector3Efk.Z);
}

//-----------------------------------------------------------------------------------.

::Effekseer::Matrix44 EffekseerManager::ConvertToEfkMatrix(const DirectX::XMMATRIX& matrixDx)
{
	::Effekseer::Matrix44 outMatrix;

	for (int i = 0;i < 4;i++)
	{
		for (int j = 0;j < 4;j++)
		{
			DirectX::XMVECTOR row = matrixDx.r[i];

			// XMFLOAT4に変換.
			DirectX::XMFLOAT4 rowFloat;
			DirectX::XMStoreFloat4(&rowFloat, row);

			outMatrix.Values[i][0] = rowFloat.x;
			outMatrix.Values[i][1] = rowFloat.y;
			outMatrix.Values[i][2] = rowFloat.z;
			outMatrix.Values[i][3] = rowFloat.w;
		}
	}

	return outMatrix;
}

//-----------------------------------------------------------------------------------.

DirectX::XMMATRIX EffekseerManager::ConvertToXMMatrix(const::Effekseer::Matrix44& matrixEfk)
{
	// XMMATRIX の各行を構成するために XMVECTOR を作成
	DirectX::XMVECTOR r0 = DirectX::XMVectorSet(
		matrixEfk.Values[0][0], matrixEfk.Values[0][1],
		matrixEfk.Values[0][2], matrixEfk.Values[0][3]);

	DirectX::XMVECTOR r1 = DirectX::XMVectorSet(
		matrixEfk.Values[1][0], matrixEfk.Values[1][1],
		matrixEfk.Values[1][2], matrixEfk.Values[1][3]);

	DirectX::XMVECTOR r2 = DirectX::XMVectorSet(
		matrixEfk.Values[2][0], matrixEfk.Values[2][1],
		matrixEfk.Values[2][2], matrixEfk.Values[2][3]);

	DirectX::XMVECTOR r3 = DirectX::XMVectorSet(
		matrixEfk.Values[3][0], matrixEfk.Values[3][1],
		matrixEfk.Values[3][2], matrixEfk.Values[3][3]);

	return DirectX::XMMATRIX(r0, r1, r2, r3);
}

//-----------------------------------------------------------------------------------.

const::Effekseer::ManagerRef EffekseerManager::GetManager() const
{
	return m_pManager;
}

//-----------------------------------------------------------------------------------.

void EffekseerManager::SetBackgroundTexture(ID3D11ShaderResourceView* pSRV)
{
	if (pSRV == nullptr || m_pRenderer == nullptr) {
		return;
	}

	// DX11 専用の SetBackground を使用（SRV を直接渡す）
	// deprecated警告を抑制
#pragma warning(push)
#pragma warning(disable: 4996)
	m_pRenderer->SetBackground(pSRV);
#pragma warning(pop)
}

//-----------------------------------------------------------------------------------.

void EffekseerManager::ClearBackgroundTexture()
{
	if (m_pRenderer != nullptr) {
#pragma warning(push)
#pragma warning(disable: 4996)
		m_pRenderer->SetBackground(static_cast<ID3D11ShaderResourceView*>(nullptr));
#pragma warning(pop)
	}
}

//-----------------------------------------------------------------------------------.

void EffekseerManager::Initialize()
{
	// マネージャーの作成.
	m_pManager = ::Effekseer::Manager::Create(RENDER_SPRITE_MAX);

	// レンダラの作成.
	m_pRenderer = ::EffekseerRendererDX11::Renderer::Create(
		DirectX11::GetInstance().GetDevice(),
		DirectX11::GetInstance().GetContext(),
		EFFECT_INSTANCE_MAX);

	// 描画モジュールの設定.
	m_pManager->SetSpriteRenderer(m_pRenderer->CreateSpriteRenderer());
	m_pManager->SetRibbonRenderer(m_pRenderer->CreateRibbonRenderer());
	m_pManager->SetRingRenderer(m_pRenderer->CreateRingRenderer());
	m_pManager->SetTrackRenderer(m_pRenderer->CreateTrackRenderer());
	m_pManager->SetModelRenderer(m_pRenderer->CreateModelRenderer());

	// テクスチャ、モデル、マテリアルローダーの設定.
	// ユーザーが独自に拡張できる.現在はファイルから読み込み.
	m_pManager->SetTextureLoader(m_pRenderer->CreateTextureLoader());
	m_pManager->SetModelLoader(m_pRenderer->CreateModelLoader());
	m_pManager->SetMaterialLoader(m_pRenderer->CreateMaterialLoader());
	m_pManager->SetCurveLoader(::Effekseer::MakeRefPtr<::Effekseer::CurveLoader>());

}
