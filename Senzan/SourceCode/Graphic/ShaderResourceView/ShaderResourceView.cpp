#include "ShaderResourceView.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"

ShaderResourceView::ShaderResourceView()
    : m_pShaderResourceView( nullptr )
{
}

ShaderResourceView::~ShaderResourceView()
{
    SAFE_RELEASE(m_pShaderResourceView);
}

//---------------------------------------------------------------------.

void ShaderResourceView::Init(ID3D11Texture2D* pTexture)
{
    // テクスチャからDescを取得.
    D3D11_TEXTURE2D_DESC textureDesc = {};
    pTexture->GetDesc(&textureDesc);

    // シェーダーリソースビューの作成.
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format              = textureDesc.Format;
    srvDesc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    auto result = DirectX11::GetInstance().GetDevice()->CreateShaderResourceView(pTexture, &srvDesc, &m_pShaderResourceView);
    if (FAILED(result)) 
    {
        assert(0 && "シェーダーリソースビューの作成に失敗");
    }
}

void ShaderResourceView::Init(const std::wstring pTexturePath)
{
    // テクスチャ作成.
    if (FAILED(D3DX11CreateShaderResourceViewFromFileW(
        DirectX11::GetInstance().GetDevice(), pTexturePath.c_str(),//テクスチャファイル名.
        nullptr, nullptr,
        &m_pShaderResourceView,
        nullptr)))
    {
        _ASSERT_EXPR(false, _T("テクスチャ作成失敗"));
    }
}

//---------------------------------------------------------------------.

ID3D11ShaderResourceView* ShaderResourceView::GetShaderResourceView() const
{
    return m_pShaderResourceView;
}
