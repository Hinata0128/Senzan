#include "Texture.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"

Texture::Texture()
    : m_pTexture(nullptr)
{
}

Texture::~Texture()
{
    SAFE_RELEASE(m_pTexture);
}

//-----------------------------------------------------------------------------.

void Texture::Init(int width, int height, DXGI_FORMAT format, UINT bindFlags)
{
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width              = width;
    desc.Height             = height;
    desc.MipLevels          = 1;
    desc.ArraySize          = 1;
    desc.Format             = format;
    desc.SampleDesc.Count   = 1;
    desc.Usage              = D3D11_USAGE_DEFAULT;
    desc.BindFlags          = bindFlags;
    desc.CPUAccessFlags     = 0;
    desc.MiscFlags          = 0;

    HRESULT result = DirectX11::GetInstance().GetDevice()->CreateTexture2D(&desc, nullptr, &m_pTexture);
    if (FAILED(result)) 
    {
        assert(0 && "テクスチャの作成に失敗");
    }
}

//-----------------------------------------------------------------------------.

ID3D11Texture2D* Texture::GetTexture() const
{
    return m_pTexture;
}
