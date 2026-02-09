#pragma once

//警告についてのコード分析を無効にする.4005:再定義.
#pragma warning(disable:4005)
#include "System/Singleton/SingletonTemplate.h"

#include "System/Singleton/Debug/Log/DebugLog.h"

//ヘッダ読込.
#include <D3DX11.h>
#include <D3D11.h>
#include <D3DX10.h>	//「D3DX～」の定義使用時に必要.
#include <D3D10.h>

//ライブラリ読み込み.
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx10.lib")	//「D3DX～」の定義使用時に必要.

// ラスタライザのフィルモード
enum class eRasterizerMode : uint8_t
{
	Solid = 1 << 0,	// 面で塗りつぶす.
	Wireframe = 1 << 1,	// ワイヤーフレーム.
	None = 1 << 2,	// カリングなし.
	Front = 1 << 3,	// 正面カリング.
	Back = 1 << 4,	// 背面カリング.
};

DEFINE_ENUM_FLAG_OPERATORS(eRasterizerMode); // ビット演算子を定義.

/**************************************************
*	DirectX11 セットアップ.
**/
class DirectX11
	: public Singleton<DirectX11>
{
	friend class Singleton<DirectX11>; // Singletonからのアクセスを許可.
private:
	DirectX11();
	
public:
	~DirectX11();
	//DirectX11構築.
	HRESULT Create(HWND hWnd);
	//DirectX11解放.
	void Release();

	//深度（Ｚ）テストON/OFF切り替え.
	void SetDepth(bool flag);
	//アルファブレンドON/OFF切り替え.
	void SetAlphaBlend(bool flag);

	//バックバッファクリア関数.
	void ClearBackBuffer();

	// レンダーターゲットをリセット.
	void ResetRenderTarget();

	//表示.
	void Present();

	//デバイスを取得.
	ID3D11Device* GetDevice() { return GetInstance().m_pDevice11; }
	//デバイスコンテキストを取得.
	ID3D11DeviceContext* GetContext() { return GetInstance().m_pContext11; }
	// スワップチェインを取得.
	IDXGISwapChain* GetSwapChain() { return GetInstance().m_pSwapChain; }


	/****************************************************
	* @brief			: ラスタライザの設定.
	* @param fillMode	: フィルモード.
	* @param cullingMode: カリングモード.
	****************************************************/
	void SetRasterizerState(eRasterizerMode fill_cullingMode);

    ID3D11DepthStencilView* GetBackBufferDSV() const;
    ID3D11RenderTargetView* GetBackBufferRTV() const;

private:
	//デバイスとスワップチェイン作成.
	HRESULT CreateDeviceAndSwapChain(
		HWND hWnd, UINT uFPS, UINT uWidth, UINT uHeight);

	//バックバッファ作成:カラー用レンダーターゲットビュー作成.
	HRESULT CreateColorBackBufferRTV();

	//バックバッファ作成:デプスステンシル用レンダーターゲットビュー作成.
	HRESULT CreateDepthStencilBackBufferRTV();

	//デプスステンシルステート作成.
	HRESULT CreateDepthStencilState();

	//アルファブレンドステート作成.
	HRESULT CreateAlphaBlendState();

	//オプションが有効かどうかをチェック.
	bool IsOptionEnabled(eRasterizerMode currentOptions, eRasterizerMode optionsToCheck) const;
private:
	ID3D11Device* m_pDevice11;			//デバイスオブジェクト.
	ID3D11DeviceContext* m_pContext11;			//デバイスコンテキスト.
	IDXGISwapChain* m_pSwapChain;			//スワップチェーン.
	ID3D11RenderTargetView* m_pBackBuffer_TexRTV;	//レンダーターゲットビュー.
	ID3D11Texture2D* m_pBackBuffer_DSTex;	//デプスステンシル用テクスチャ.
	ID3D11DepthStencilView* m_pBackBuffer_DSTexDSV;	//デプスステンシルビュー.

	//深度（Ｚ）テスト設定.
	ID3D11DepthStencilState* m_pDepthStencilStateOn;		//有効設定.
	ID3D11DepthStencilState* m_pDepthStencilStateOff;	//無効設定.

	//アルファブレンド.
	ID3D11BlendState* m_pAlphaBlendOn;	//有効設定.
	ID3D11BlendState* m_pAlphaBlendOff;	//無効設定.

	// ラスタライザ.
	MyComPtr<ID3D11RasterizerState> m_RasterizerState;		// ラスタライザ.
};
