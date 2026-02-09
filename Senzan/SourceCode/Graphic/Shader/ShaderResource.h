#pragma once
#include "System/Singleton/SingletonTemplate.h"

#include "Base\VertexShaderBase.h"
#include "Base\PixelShaderBase.h"

/******************************************************************
*	シェーダーリソース管理クラス.
*	頂点シェーダーとピクセルシェーダーの情報を管理している.
******************************************************************/
class ShaderResource final
	: public Singleton<ShaderResource>
{
private:
	friend class Singleton<ShaderResource>;
	ShaderResource();
public:
	~ShaderResource();

	/****************************************************************
	* @brief シャドウマップ用の頂点・ピクセルシェーダーを取得.
	****************************************************************/
	const std::unique_ptr<VertexShaderBase>&	GetVSShadowMapStaticMesh();
	const std::unique_ptr<PixelShaderBase>&	GetPSShadowMapStaticMesh();
	const std::unique_ptr<VertexShaderBase>&	GetVSShadowMapSkinMesh();
	const std::unique_ptr<PixelShaderBase>&	GetPSShadowMapSkinMesh();

private:

	/*********************************************
	* @brief VS・PSの初期化.
	*********************************************/
	void Init();

private:
	// シャドウマップ用頂点・ピクセルシェーダー.
	std::unique_ptr<VertexShaderBase>	m_pVSShadowMapStaticMesh;
	std::unique_ptr<PixelShaderBase>	m_pPSShadowMapStaticMesh;
	std::unique_ptr<VertexShaderBase>	m_pVSShadowMapSkinMesh;
	std::unique_ptr<PixelShaderBase>	m_pPSShadowMapSkinMesh;
};