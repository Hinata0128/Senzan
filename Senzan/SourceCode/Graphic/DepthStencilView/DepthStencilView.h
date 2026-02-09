#pragma once
/***********************************************
*	デプスステンシルビュークラス.
***********************************************/
class DepthStencilView final
{
public:
	DepthStencilView();
	~DepthStencilView();

	/***************************************
	* @brief 初期化.
	* @param pTexture：テクスチャのポインタ.
	***************************************/
	void Init(ID3D11Texture2D* pTexture);


	/************************************************
	* @brief デプスステンシルビューをクリア.
	* @param clearColor：クリアカラー.
	************************************************/
	void ClearDepthStencilView();


	/***************************************
	* @brief デプスステンシルビューを取得.
	***************************************/
	[[nodiscard]] ID3D11DepthStencilView* GetDepthStencilView();
private:
	// デプスステンシルビュー.(デプスは深度情報,ステンシルはマスク情報).
	ID3D11DepthStencilView* m_pDepthStencilView;
};