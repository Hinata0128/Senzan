#include "PixelShaderBase.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"

PixelShaderBase::PixelShaderBase()
    : ShaderBase   ()
    , m_pPixelShader()
{
}

//-------------------------------------------------------------------.

PixelShaderBase::~PixelShaderBase()
{
}

//-------------------------------------------------------------------.

void PixelShaderBase::Init(ID3DBlob* pShaderBlob)
{
	// ピクセルシェーダを作成.
	auto result = DirectX11::GetInstance().GetDevice()->CreatePixelShader(
		pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(),
		nullptr, &m_pPixelShader);

	if (FAILED(result))
	{
		SAFE_RELEASE(pShaderBlob);
		assert(0 && "ピクセルシェーダの作成に失敗");
	}
}

//-------------------------------------------------------------------.

void PixelShaderBase::SetPixelShader()
{
	DirectX11::GetInstance().GetContext()->PSSetShader(m_pPixelShader, nullptr, 0);
}

//-------------------------------------------------------------------.

ID3D11PixelShader* PixelShaderBase::GetPixelShader() const
{
    return m_pPixelShader;
}
