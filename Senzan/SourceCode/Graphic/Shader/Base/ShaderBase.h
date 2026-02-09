#pragma once
class ShaderBase
{
public:
	ShaderBase();
	virtual ~ShaderBase();

	/*********************************************
	* @brief 初期化.
	* @param pShaderBlob：ブロブのポインタ.
	*********************************************/
	virtual void Init(ID3DBlob* pShaderBlob) = 0;
};