#include "RenderTargetView.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"

RenderTargetView::RenderTargetView()
    : m_pRenderTargetView( nullptr )
{
}

RenderTargetView::~RenderTargetView()
{
    SAFE_RELEASE(m_pRenderTargetView);
}

//------------------------------------------------------------.

void RenderTargetView::Init(ID3D11Texture2D* pTexture)
{
    // テクスチャからDescを取得.
    D3D11_TEXTURE2D_DESC textureDesc = {};
    pTexture->GetDesc(&textureDesc);

    // レンダーターゲットビューの作成.
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format              = textureDesc.Format;
    rtvDesc.ViewDimension       = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice  = 0;
    auto result = DirectX11::GetInstance().GetDevice()->CreateRenderTargetView(pTexture, &rtvDesc, &m_pRenderTargetView);
    if (FAILED(result))
    {
        assert(0 && "レンダーターゲットビューの作成に失敗");
    }
}

//------------------------------------------------------------.

void RenderTargetView::ClearRenderTargetView(const DirectX::XMFLOAT4& clearColor)
{
    DirectX11::GetInstance().GetContext()->ClearRenderTargetView(m_pRenderTargetView, reinterpret_cast<const FLOAT*>(&clearColor));
}
//------------------------------------------------------------.

ID3D11RenderTargetView* RenderTargetView::GetRenderTargetView()
{
    return m_pRenderTargetView;
}
