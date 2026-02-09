#pragma once
/*************************************************************************
* ファイル名	：EffekseerManager.h.
* 作成日		：2025-05-02.
* 更新日		：2025-05-02.
* 作成者		：藤澤 幸輝.
* 説明		：Effekseerの描画管理なのどを行う.
*************************************************************************/

#include "../../SingletonTemplate.h"

#include "../../../../../Data/Library/Effekseer/include/Effekseer.h"
#include "../../../../../Data/Library/Effekseer/include/EffekseerRendererDX11.h"

#ifdef _DEBUG
#pragma comment( lib, "Effekseerd.lib" )
#pragma comment( lib, "EffekseerRendererDX11d.lib" )
#else//#ifdef _DEBUG
#pragma comment( lib, "Effekseer.lib" )
#pragma comment( lib, "EffekseerRendererDX11.lib" )
#endif//#ifdef _DEBUG

class CameraBase;

/******************************************************************
*	EffekseerManager.
******************************************************************/
class EffekseerManager final
	: public Singleton<EffekseerManager>
{
private:
	friend class Singleton<EffekseerManager>;
	EffekseerManager();
public:
	~EffekseerManager();


	/*******************************************************************
	* @brief 指定したエフェクトハンドル更新.
	* @param[in] handle：更新するエフェクトハンドル.
	*******************************************************************/
	void UpdateHandle(::Effekseer::Handle handle);


	/*******************************************************************
	* @brief 指定したエフェクトハンドル描画.
	* @param[in] handle		：描画するエフェクトハンドル.
	* @param[in] pUseCamera	：描画に使用するカメラ.
	*******************************************************************/
	void RenderHandle(::Effekseer::Handle handle, CameraBase* pUseCamera);


	/*******************************************************************
	* @brief UI用エフェクト描画（スクリーン座標系）.
	* @param[in] handle：描画するエフェクトハンドル.
	* @note Play時の座標はスクリーン座標（ピクセル）で指定.
	*******************************************************************/
	void RenderHandleUI(::Effekseer::Handle handle);


	/*******************************************************************
	* @brief DirectX::XMFLOAT3をEffekseer::Vector3に変換.
	*******************************************************************/
	static ::Effekseer::Vector3D ConvertToEfkVector3(const DirectX::XMFLOAT3& vector3Dx);


	/*******************************************************************
	* @brief DirectX::XMFLOAT3をEffekseer::Vector3に変換.
	*******************************************************************/
	static DirectX::XMFLOAT3 ConvertToXMFloat3(const ::Effekseer::Vector3D& vector3Efk);


	/*******************************************************************
	* @brief DirectX::XMMATRIXをEffekseer::Matrixに変換.
	*******************************************************************/
	static ::Effekseer::Matrix44 ConvertToEfkMatrix(const DirectX::XMMATRIX& matrixDx);


	/*******************************************************************
	* @brief Effekseer::MatrixをDirectX::XMMATRIXに変換.
	*******************************************************************/
	static DirectX::XMMATRIX ConvertToXMMatrix(const ::Effekseer::Matrix44& matrixEfk);


	/*******************************************************************
	* @brief 背景テクスチャを設定（歪みエフェクト用）.
	* @param[in] pSRV：背景のシェーダーリソースビュー.
	* @note 歪み(distortion)エフェクトを使用する場合は描画前に呼び出す.
	*******************************************************************/
	void SetBackgroundTexture(ID3D11ShaderResourceView* pSRV);

	/*******************************************************************
	* @brief 背景テクスチャをクリア.
	*******************************************************************/
	void ClearBackgroundTexture();

public: // Getter.

	/*******************************************************************
	* @brief マネージャーを取得.
	*******************************************************************/
	const ::Effekseer::ManagerRef GetManager() const;

	/*******************************************************************
	* @brief レンダラーを取得.
	*******************************************************************/
	const EffekseerRendererDX11::RendererRef GetRenderer() const { return m_pRenderer; }

private:

	/*******************************************************************
	* @brief マネージャーやレンダラなどの初期化.
	*******************************************************************/
	void Initialize();

private:
	::Effekseer::ManagerRef				m_pManager; // マネージャー.
	EffekseerRendererDX11::RendererRef	m_pRenderer;	// レンダラ.
};
