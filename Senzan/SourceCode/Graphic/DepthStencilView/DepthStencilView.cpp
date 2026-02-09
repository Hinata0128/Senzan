#include "DepthStencilView.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"

DepthStencilView::DepthStencilView()
    :m_pDepthStencilView( nullptr )
{
}

DepthStencilView::~DepthStencilView()
{
    SAFE_RELEASE(m_pDepthStencilView);
}

//-------------------------------------------------------------------------.

void DepthStencilView::Init(ID3D11Texture2D* pTexture)
{
    // デプスステンシルビューの作成.
    auto result = DirectX11::GetInstance().GetDevice()->CreateDepthStencilView(pTexture, nullptr, &m_pDepthStencilView);
    if (FAILED(result))
    {
        _ASSERT_EXPR(false, _T("デプスステンシルビュー作成失敗"));
    }
}

//-------------------------------------------------------------------------.

void DepthStencilView::ClearDepthStencilView()
{
    DirectX11::GetInstance().GetContext()->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0, 0);
}

//-------------------------------------------------------------------------.

ID3D11DepthStencilView* DepthStencilView::GetDepthStencilView()
{
    return m_pDepthStencilView;
}
