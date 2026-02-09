#include "Shadow.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Graphic/RenderTarget/RenderTargetManager.h"

namespace
{
	// デプスシャドウのサイズ.
	static constexpr int DEPSH_W = 4096 * 2;
	static constexpr int DEPSH_H = 4096 * 2;
}

Shadow::Shadow()
    : m_pCBuffer				( nullptr )
{
	RenderTargetManager::GetInstance().CreateRenderTarget(
		"ShadowMap",
		DEPSH_W,
		DEPSH_H,
		DXGI_FORMAT_R32G32_FLOAT,
		D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
		DXGI_FORMAT_D32_FLOAT,
		D3D11_BIND_DEPTH_STENCIL
	);

	CreateCBuffer();// コンスタントバッファ作成.
}

Shadow::~Shadow()
{
	SAFE_RELEASE(m_pCBuffer);
}

//---------------------------------------------------------------------------------.

void Shadow::Begin()
{
	Shadow& pI = GetInstance();

	// ビューポートを深度マップ用に変更.
	D3D11_VIEWPORT vp;
	vp.Width	= static_cast<float>(DEPSH_W);
	vp.Height	= static_cast<float>(DEPSH_H);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	DirectX11::GetInstance().GetContext()->RSSetViewports(1, &vp);

	// レンダーターゲットをクリア.
	RenderTargetManager::GetInstance().ClearRenderTarget("ShadowMap");	

	// 描画先を深度マップに変更.
	RenderTargetManager::GetInstance().SetRenderTargets("ShadowMap");
}

//---------------------------------------------------------------------------------.

void Shadow::End()
{
	Shadow& pI = GetInstance();

	// ビューポートをスクリーンサイズに変更.
	D3D11_VIEWPORT vp;
    vp.Width    = static_cast<float>(WND_W);
    vp.Height   = static_cast<float>(WND_H);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
	DirectX11::GetInstance().GetContext()->RSSetViewports(1, &vp);

	// レンダーターゲットを元に戻す.
	DirectX11::GetInstance().ResetRenderTarget();
}

//---------------------------------------------------------------------------------.

ID3D11Buffer* Shadow::GetCBuffer()
{
	return m_pCBuffer;
}

//---------------------------------------------------------------------------------.

void Shadow::CreateCBuffer()
{
	D3D11_BUFFER_DESC cb;
	cb.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth		= sizeof(CBUFFER);
	cb.CPUAccessFlags	= D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags		= 0;
	cb.Usage			= D3D11_USAGE_DYNAMIC;

	HRESULT result;
	result = DirectX11::GetInstance().GetDevice()->CreateBuffer(&cb, nullptr, &m_pCBuffer);
	if (FAILED(result)) {
		_ASSERT_EXPR(false, _T("コンスタントバッファ作成失敗"));
	}
}

