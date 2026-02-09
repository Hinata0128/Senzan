#pragma once
class Texture;
class RenderTargetView;
class DepthStencilView;
class ShaderResourceView;

/***********************************************
*	レンダーターゲットクラス.
***********************************************/
class RenderTarget final
{
public:
	RenderTarget();
	~RenderTarget();

	/***************************************************
	* @brief 初期化.
	* @param width				：幅.
	* @param height				：高さ.
	* @param colorFormat		：カラーフォーマット.
	* @param colorBindFlags		：カラーテクスチャのバインドフラグ.
	* @param depthFormat		：深度ステンシルフォーマット.
	* @param depthBindFlags		：深度ステンシルのバインドフラグ.
	* @param clearColor			：クリアカラー.
	***************************************************/
	void Init(
		int width,
		int height,
		DXGI_FORMAT colorFormat,
		UINT colorBindFlags,
		DXGI_FORMAT depthFormat,
		UINT depthBindFlags,
		DirectX::XMFLOAT4 clearColor = { 1.0f,1.0f,1.0f,1.0f });


	/***************************************************
	* @brief レンダーターゲットをセット.
	***************************************************/
	void SetRenderTargets();


	/***************************************************
	* @brief レンダーターゲットをクリア.	
	***************************************************/
	void ClearRenderTarget();


	/***************************************************
	* @brief シェーダーリソースビューを取得.
	***************************************************/
	[[nodiscard]] ID3D11ShaderResourceView* GetShaderResourceView() const;
private:
	std::unique_ptr<Texture> m_pColorTextur;// カラーテクスチャ.
	std::unique_ptr<Texture> m_pDepthTextur;// 深度テクスチャ.

	std::unique_ptr<RenderTargetView>		m_pRenderTargetView;	// レンダーターゲットビュー.
	std::unique_ptr<DepthStencilView>		m_pDepthStencilView;	// デプスステンシルビュー.
	std::unique_ptr<ShaderResourceView>	m_pShaderResourceView;	// シェーダーリソースビュー.

	DirectX::XMFLOAT4 m_ClearColor;// クリアカラー.
};