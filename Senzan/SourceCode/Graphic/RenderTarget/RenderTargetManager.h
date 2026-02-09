#pragma once
#include "System/Singleton/SingletonTemplate.h"

#include "RenderTarget.h"

/***************************************************
*	レンダーターゲット管理クラス.
***************************************************/
class RenderTargetManager final
	: public Singleton<RenderTargetManager>
{
private:
	friend class Singleton<RenderTargetManager>;
	RenderTargetManager();
public:
	~RenderTargetManager();

	/***************************************************
	* @brief レンダーターゲットを作成.
	* @param name				：名前.
	* @param width				：幅.
	* @param height				：高さ.
	* @param colorFormat		：カラーフォーマット.
	* @param colorBindFlags		：カラーテクスチャのバインドフラグ.
	* @param depthFormat		：深度ステンシルフォーマット.
	* @param depthBindFlags		：深度ステンシルのバインドフラグ.
	* @param clearColor			：クリアカラー
	***************************************************/
	void CreateRenderTarget(
		std::string name,
		int width,
		int height,
		DXGI_FORMAT colorFormat,
		UINT colorBindFlags,
		DXGI_FORMAT depthFormat,
		UINT depthBindFlags,
		DirectX::XMFLOAT4 clearColor = { 1.0f,1.0f,1.0f,1.0f });


	/***************************************************
	* @brief レンダーターゲットを取得.
	* @param name：取得するレンダーターゲット名.
	***************************************************/
	RenderTarget& GetRenderTarget(std::string name);


	/***************************************************
	* @brief レンダーターゲットをセット.
	* @param name：セットするレンダーターゲット名.
	***************************************************/
	void SetRenderTargets(std::string name);


	/***************************************************
	* @brief レンダーターゲットをクリア.
	* @param name：クリアするレンダーターゲット名.
	***************************************************/
	void ClearRenderTarget(std::string name);


	/***************************************************
	* @brief 全てのレンダーターゲットをクリア.
	***************************************************/
	void AllClearRenderTarget();
private:
	std::unordered_map<std::string, std::unique_ptr<RenderTarget>> m_pRenderTargets;
};