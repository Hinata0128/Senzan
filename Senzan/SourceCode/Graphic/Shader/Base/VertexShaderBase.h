#pragma once
#include "ShaderBase.h"
class VertexShaderBase
	: public ShaderBase
{
public:
	VertexShaderBase();
	virtual ~VertexShaderBase() override;


	/*********************************************
	* @brief 初期化.
	* @param pShaderBlob：ブロブのポインタ.
	*********************************************/
	void Init(ID3DBlob* pShaderBlob) override;


	/*********************************************
	* @brief 頂点シェーダーをパイプラインにセット.
	*********************************************/
	virtual void SetVertexShader();


	/*********************************************
	* @brief 頂点レイアウトをパイプラインにセット.
	*********************************************/
	void SetInputLayout();


	/*********************************************
	* @brief 頂点シェーダーを取得.
	*********************************************/
	[[nodiscard]] ID3D11VertexShader* GetVertexShader() const;


	/*********************************************
	* @brief 頂点レイアウトを取得.
	*********************************************/
	[[nodiscard]] ID3D11InputLayout* GetInputLayout() const;

	/*********************************************
	* @brief 入力データを構成.
	*********************************************/
	void ConfigInputLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout);

protected:
	ID3D11VertexShader* m_pVertexShader;
	ID3D11InputLayout*	m_pInputLayout;
	std::vector<D3D11_INPUT_ELEMENT_DESC> m_Layout;
};