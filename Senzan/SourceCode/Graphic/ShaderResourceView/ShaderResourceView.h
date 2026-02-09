#pragma once
/******************************************
*	シェーダーリソースビュークラス.
******************************************/
class ShaderResourceView final
{
public:
	ShaderResourceView();
	~ShaderResourceView();

	/***************************************
	* @brief 初期化.
	* @param pTexture：テクスチャのポインタ.
	***************************************/
	void Init(ID3D11Texture2D* pTexture);

	void Init(const std::wstring pTexturePath);

	/***************************************
	* @brief シェーダーリソースビューを取得.
	***************************************/
	[[nodiscard]] ID3D11ShaderResourceView* GetShaderResourceView() const;
private:
	ID3D11ShaderResourceView* m_pShaderResourceView;
};