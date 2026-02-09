#pragma once

//警告についてのコード分析を無効にする.4005:再定義.
#pragma warning(disable:4005)

//DirectX9.
#include <d3dx9.h>

//ライブラリ読み込み.
#pragma comment( lib, "winmm.lib" )
//DirectX9.
#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "d3dx9.lib" )
#include "System/Singleton/SingletonTemplate.h"

/**************************************************
*	DirectX9 セットアップ.
**/
class DirectX9
	: public Singleton<DirectX9>
{
	friend class Singleton<DirectX9>;
private:
	DirectX9();

public:

	~DirectX9();

	//DirectX9構築.
	HRESULT Create( HWND hWnd );
	//DirectX9解放.
	void Release();

	//デバイスオブジェクトを取得.
	LPDIRECT3DDEVICE9 GetDevice() const { return m_pDevice9; }


private:
	LPDIRECT3DDEVICE9	m_pDevice9;	//デバイスオブジェクト.
};