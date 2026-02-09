#include "VertexShaderBase.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"

VertexShaderBase::VertexShaderBase()
    : ShaderBase       ()
    , m_pVertexShader   ( nullptr )
    , m_pInputLayout    ( nullptr )
    , m_Layout          ()
{
}

//-----------------------------------------------------------------.

VertexShaderBase::~VertexShaderBase()
{
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pVertexShader);
}

//-----------------------------------------------------------------.

void VertexShaderBase::Init(ID3DBlob* pShaderBlob)
{

	// 頂点シェーダーを作成.
	auto result = DirectX11::GetInstance().GetDevice()->CreateVertexShader(
		pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(),
		nullptr,
		&m_pVertexShader);


	if (FAILED(result))
	{
		SAFE_RELEASE(pShaderBlob);
		assert(0 && "頂点シェーダーの作成に失敗");
	}

	// 入力データを構成.
	//ConfigInputLayout();

	// 入力データを作成.
	result = DirectX11::GetInstance().GetDevice()->CreateInputLayout(
		m_Layout.data(),
		static_cast<UINT>(m_Layout.size()),
		pShaderBlob->GetBufferPointer(),
		pShaderBlob->GetBufferSize(),
		&m_pInputLayout);

	if (FAILED(result))
	{
		SAFE_RELEASE(pShaderBlob);
		assert(0 && "頂点データの作成に失敗");
	}
}

//-----------------------------------------------------------------.

void VertexShaderBase::SetVertexShader()
{
	DirectX11::GetInstance().GetContext()->VSSetShader(m_pVertexShader, nullptr, 0);
}

//-----------------------------------------------------------------.

void VertexShaderBase::SetInputLayout()
{
	DirectX11::GetInstance().GetContext()->IASetInputLayout(m_pInputLayout);
}

//-----------------------------------------------------------------.

ID3D11VertexShader* VertexShaderBase::GetVertexShader() const
{
    return m_pVertexShader;
}

//-----------------------------------------------------------------.

ID3D11InputLayout* VertexShaderBase::GetInputLayout() const
{
    return m_pInputLayout;
}

//-----------------------------------------------------------------.

void VertexShaderBase::ConfigInputLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout)
{
	m_Layout = layout;
}
