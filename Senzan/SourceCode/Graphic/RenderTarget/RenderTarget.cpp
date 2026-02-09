#include "RenderTarget.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Graphic/Texture/Texture.h"
#include "Graphic/RenderTargetView/RenderTargetView.h"
#include "Graphic/DepthStencilView/DepthStencilView.h"
#include "Graphic\ShaderResourceView\ShaderResourceView.h"

RenderTarget::RenderTarget()
    : m_pColorTextur        ( std::make_unique<Texture>() )
    , m_pDepthTextur        ( std::make_unique<Texture>() )
    , m_pRenderTargetView   ( std::make_unique<RenderTargetView>() )
    , m_pDepthStencilView   ( std::make_unique<DepthStencilView>() )
    , m_pShaderResourceView ( std::make_unique<ShaderResourceView>() )
    , m_ClearColor          ()
{
}

RenderTarget::~RenderTarget()
{
}

//-----------------------------------------------------.

void RenderTarget::Init(
    int width,
    int height, 
    DXGI_FORMAT colorFormat, 
    UINT colorBindFlags, 
    DXGI_FORMAT depthFormat, 
    UINT depthBindFlags, 
    DirectX::XMFLOAT4 clearColor)
{
    m_ClearColor = clearColor;

    m_pColorTextur->Init(width, height, colorFormat, colorBindFlags);   // カラーテクスチャの初期化.
    m_pDepthTextur->Init(width, height, depthFormat, depthBindFlags);   // 深度テクスチャの初期化.

    m_pRenderTargetView->Init(m_pColorTextur->GetTexture());            // レンダーターゲットビューの初期化.
    m_pDepthStencilView->Init(m_pDepthTextur->GetTexture());            // 深度ステンシルビューの初期化.
    m_pShaderResourceView->Init(m_pColorTextur->GetTexture());          // シェーダーリソースビューの初期化.
}

//-----------------------------------------------------.

void RenderTarget::SetRenderTargets()
{
    ID3D11RenderTargetView* pRenderTargetView = m_pRenderTargetView->GetRenderTargetView();
    ID3D11DepthStencilView* pDepthStencilView = m_pDepthStencilView->GetDepthStencilView();

    DirectX11::GetInstance().GetContext()->OMSetRenderTargets(
        1,
        &pRenderTargetView,
        pDepthStencilView);
}

//-----------------------------------------------------.

void RenderTarget::ClearRenderTarget()
{
    m_pRenderTargetView->ClearRenderTargetView(m_ClearColor);
    m_pDepthStencilView->ClearDepthStencilView();
}

//-----------------------------------------------------.

ID3D11ShaderResourceView* RenderTarget::GetShaderResourceView() const
{
    return m_pShaderResourceView->GetShaderResourceView();
}
