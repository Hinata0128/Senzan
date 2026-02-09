#pragma once

/*********************************************
*	レンダーターゲットビュークラス.
*********************************************/
class RenderTargetView final
{
public:
	RenderTargetView();
	~RenderTargetView();

	/************************************************
	* @brief 初期化.
	* @param pTexture：テクスチャのポインタ.
	************************************************/
	void Init(ID3D11Texture2D* pTexture);

	/************************************************
	* @brief レンダーターゲットビューをクリア.
	* @param clearColor：クリアカラー.
	************************************************/
	void ClearRenderTargetView(const DirectX::XMFLOAT4& clearColor);

	/************************************************
	* @brief レンダーターゲットビューを取得.
	************************************************/
	[[nodiscard]] ID3D11RenderTargetView* GetRenderTargetView();
private:
	ID3D11RenderTargetView* m_pRenderTargetView;
};