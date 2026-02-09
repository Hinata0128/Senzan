#pragma once
#include "ShaderBase.h"
class PixelShaderBase
	: public ShaderBase
{
public:
	PixelShaderBase();
	virtual ~PixelShaderBase() override;

	/*********************************************
	* @brief 初期化.
	* @param pShaderBlob：ブロブのポインタ.
	*********************************************/
	void Init(ID3DBlob* pShaderBlob) override;


	/*********************************************
	* @brief ピクセルシェーダーをパイプラインにセット.
	*********************************************/
	virtual void SetPixelShader();


	/*********************************************
	* @brief ピクセルシェーダーを取得.
	*********************************************/
	[[nodiscard]] ID3D11PixelShader* GetPixelShader() const;
protected:
	ID3D11PixelShader* m_pPixelShader;
};