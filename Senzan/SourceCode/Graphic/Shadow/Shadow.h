#pragma once
#include "System/Singleton/SingletonTemplate.h"

/********************************************
*	影クラス.
********************************************/
class Shadow final
	: public Singleton<Shadow>
{
private:
	friend class Singleton<Shadow>;
	Shadow();
public:	
	~Shadow();

	/**************************************
	* @brief 影描画開始.
	**************************************/
	static void Begin();
	

	/**************************************
	* @brief 影描画終了.
	**************************************/
	static void End();


	/**************************************
	* @brief コンスタントバッファを取得.		
	**************************************/
	ID3D11Buffer*	GetCBuffer();

private:

	/**************************************
	* @brief コンスタントバッファを作成.
	* *************************************/
	void CreateCBuffer();

public:

	// コンスタントバッファ.
	struct CBUFFER
	{
		DirectX::XMMATRIX mWLP;
	};

private:	
	ID3D11Buffer*		m_pCBuffer;
};