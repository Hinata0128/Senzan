
#pragma once

//警告についてのｺｰﾄﾞ分析を無効にする.4005:再定義.
#pragma warning(disable:4005)


// MEMO : Windowライブラリに入っていmin,maxが邪魔なので無効にする.
//		: Windows.hより先に定義するすることが推奨されているためこの位置.

#ifndef NOMINMAX
#define NOMINMAX
#endif
#pragma warning( push )
// C26495: メンバー変数が必ず初期化されていない警告 (type.6)
// C4834: 破棄された戻り値のある波括弧初期化の警告
// C4305: doubleからfloatへの切り捨て (リテラル初期化でよくある)
#pragma warning( disable : 26495 4834 4305 ) 
//警告についてのｺｰﾄﾞ分析を無効にする.4005:再定義.
#pragma warning(disable:4005)
#include <Windows.h>
#include <crtdbg.h>

//DirectX9
#include <d3dx9.h>
//DirectX11
#include <D3DX11.h>
#include <D3D11.h>
//ライブラリ読み込み.
#pragma comment( lib, "winmm.lib" )
//DirectX9
#pragma comment( lib, "d3dx9.lib" )
#pragma comment( lib, "d3d9.lib" )
//DirectX11
#pragma comment( lib, "d3dx11.lib" )
#pragma comment( lib, "d3d11.lib" )

#pragma warning( pop )
#include <stdio.h>
#include <tchar.h>
#include <functional>
#include <cassert>
#include <filesystem>	// ファイル関連.
#include <vector>       // 可変長配列.	
#include <array>        // 定数長配列.
#include <algorithm>    // アルゴリズム.
#include <map>          // マップ.
#include <unordered_map>// マップ.
#include <cmath>        // 数学関数.
#include <memory>       // メモリ管理.
#include <string>       // 文字列.
#include <fstream>		// ファイル入出力.	
#include <chrono>		// 時間計測.	
#include<type_traits>

#include "Game/04_Time/Time.h"	// Time.	

#include "Utility/Assert/Assert.inl"	// HRESULTのtrycatchをする.
#include "Utility/Math/Math.h"			// 算数系.	

#include "Utility/ComPtr/ComPtr.h"		// Microsoft::WRL::ComPtrのようなもの.	
#include "Utility/CustomSTL/pair/Pair.h"// 添え字が使えるpair.	
//#include "Utility/EnumFlags/EnumFlags.h"// Enumのビット演算子オーバーロード.
#include "Utility/Transform/Transform.h"// Transform.	

#include "System/Singleton/SingletonTemplate.h"	// シングルトンテンプレト.	
#include "System/Singleton/Debug/Log/DebugLog.h"	// シングルトンテンプレト.	

#define _SOUND_STOP_DEBUG 0

#if _DEBUG
#include <dxgidebug.h>	// メモリリークの検出.
#endif // _DEBUG.

// DirectX12.
//#include <D3D12.h>
//#include <dxgi1_6.h>
//#include <DirectXMath.h>

// DirectSound.
#include <dsound.h>

//ライブラリ読み込み.
#pragma comment(lib, "winmm.lib")
//#pragma comment(lib, "d3d12.lib")
//#pragma comment(lib, "dxgi.lib")
//DirectSound.
#pragma comment( lib, "dsound.lib" )

//// ライブラリを設定しなくてはならない.
//// $(DXTEX_DIR)\Bin\Desktop_2022_Win10\x64\Debugをリンカー>全般>追加のライブラリディテクトリに追加.
//#pragma comment( lib,"DirectXTex.lib" )


#include "Macro.h"	
#include "Constant.h"
