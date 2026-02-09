#include "StaticMesh.h"
#include "Graphic/DirectX/DirectX9/DirectX9.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Resource/Mesh/MeshBase.h"
#include "System/Singleton/CameraManager/CameraManager.h"
#include "Graphic/Light/LightManager.h"
#include "Graphic/Shadow/Shadow.h"
#include "Graphic\RenderTarget\RenderTargetManager.h"
#include "Graphic/Light/DirectionLight/DirectionLight.h"
#include "Graphic\Shader\ShaderResource.h"
#include "Graphic\Shader\ShaderCompile.h"

#include <stdlib.h>	// マルチバイト文字→Unicode文字変換で必要.
#include <locale.h>

#include "Utility/Assert/Assert.inl"
#include "wrl.h"

namespace {
	static constexpr TCHAR VS_SHADER_FILE_PATH[] = _T("Data\\Shader\\StaticMesh\\StaticMeshVS.hlsl");
	static constexpr TCHAR PS_SHADER_FILE_PATH[] = _T("Data\\Shader\\StaticMesh\\StaticMeshPS.hlsl");

	static constexpr TCHAR NO_TEX_VS_SHADER_FILE_PATH[] = _T("Data\\Shader\\StaticMesh\\NoTexStaticMeshVS.hlsl");
	static constexpr TCHAR NO_TEX_PS_SHADER_FILE_PATH[] = _T("Data\\Shader\\StaticMesh\\NoTexStaticMeshPS.hlsl");
}

StaticMesh::StaticMesh()
	: m_pVertexBuffer(nullptr)
	, m_ppIndexBuffer(nullptr)
	, m_Model()
	, m_pMaterials()
	, m_NumAttr()
	, m_AttrID()
	, m_EnableTexture(false)
{
}

//-------------------------------------------------------------------------------------------------------------------------------------

StaticMesh::~StaticMesh()
{
	// 解放処理.
	Release();
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT StaticMesh::Init(std::string FileName, const  std::string& Name)
{
	m_pDevice9 = DirectX9::GetInstance().GetDevice();

	m_pDevice11 = DirectX11::GetInstance().GetDevice();
	m_pContext11 = DirectX11::GetInstance().GetContext();

	// 資源名を保存.
	m_ResourceName = Name;

	// ファイル読み込み.
	if (FAILED(LoadXMesh(FileName)))
	{
		return E_FAIL;
	}
	// モデル作成.
	if (FAILED(CreateModel()))
	{
		return E_FAIL;
	}
	// シェーダ作成.
	if (FAILED(CreateShader()))
	{
		return E_FAIL;
	}
	// コンスタントバッファ作成.
	if (FAILED(CreateConstantBuffer()))
	{
		return E_FAIL;
	}
	// サンプラ作成.
	if (FAILED(CreateSampler()))
	{
		return E_FAIL;
	}

	return S_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------

void StaticMesh::Release()
{
	// インデックスバッファ解放.	←バグるので調査必要.
	if (m_ppIndexBuffer != nullptr) {
		for (int No = m_Model.NumMaterials - 1; No >= 0; No--) {
			if (m_ppIndexBuffer[No] != nullptr) {
				SAFE_RELEASE(m_ppIndexBuffer[No]);
			}
		}
		delete[] m_ppIndexBuffer;
		m_ppIndexBuffer = nullptr;
	}
	// マテリアル解放.
	if (m_pMaterials != nullptr) {
		delete[] m_pMaterials;
		m_pMaterials = nullptr;
	}
	// メッシュデータの解放.
	SAFE_RELEASE(m_Model.pD3DXMtrlBuffer);
	SAFE_RELEASE(m_Model.pMesh);
	// メッシュデータ(レイとの判定用)の解放.
	SAFE_RELEASE(m_ModelForRay.pD3DXMtrlBuffer);
	SAFE_RELEASE(m_ModelForRay.pMesh);

	SAFE_RELEASE(m_pSampleLinear);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pCBufferPerFrame);
	SAFE_RELEASE(m_pCBufferPerMaterial);
	SAFE_RELEASE(m_pCBufferPerMesh);
}

//-------------------------------------------------------------------------------------------------------------------------------------

void StaticMesh::Render()
{
	// ワールド行列を計算.
	CalcWorldMatrix();

	// 使用するシェーダのセット.
	m_pContext11->VSSetShader(m_pVertexShader->GetVertexShader(), nullptr, 0);	// 頂点シェーダ.
	m_pContext11->PSSetShader(m_pPixelShader->GetPixelShader(), nullptr, 0);	// ピクセルシェーダ.

	//シャドウマップを渡す.
	auto pShadowMapSRV = RenderTargetManager::GetInstance().GetRenderTarget("ShadowMap").GetShaderResourceView();
	m_pContext11->PSSetShaderResources(1, 1, &pShadowMapSRV);

	// シェーダのコンスタントバッファに各種データを渡す.
	D3D11_MAPPED_SUBRESOURCE pData;
	if (SUCCEEDED(m_pContext11->Map(
		m_pCBufferPerFrame,
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&pData)))
	{
		// コンスタントバッファ(フレーム用).
		CBUFFER_PER_FRAME cb;

		// カメラ位置.		
		cb.CameraPos = DirectX::XMFLOAT4(CameraManager::GetInstance().GetPosition().x, CameraManager::GetInstance().GetPosition().y, CameraManager::GetInstance().GetPosition().z, 0.0f);

		// ライト方向の計算をD3DXからXMへ移行
		// 1. ライト方向を取得し、XMVECTORにロード
		DirectX::XMFLOAT3 lightDirF3 = LightManager::GetDirectionLight()->GetDirection();
		DirectX::XMVECTOR vLightDirXM = DirectX::XMLoadFloat3(&lightDirF3);

		// 2. XMVECTORで正規化を実行 (D3DXVec4Normalize -> XMVector3Normalize)
		// 方向ベクトルは3成分で正規化
		vLightDirXM = DirectX::XMVector3Normalize(vLightDirXM);

		// 3. 正規化されたXMVECTORをcb.vLightDir (XMFLOAT4) にストア
		DirectX::XMStoreFloat4(&cb.vLightDir, vLightDirXM);

		// ライトを使用するか.
		cb.isLight.x = static_cast<float>(m_IsLight);

		// 影を使用するか.
		cb.isShadow.x = static_cast<float>(m_IsShadow);

		memcpy_s(
			pData.pData,	// コピー先のバッファ.
			pData.RowPitch,	// コピー先のバッファサイズ.
			(void*)(&cb),	// コピー元のバッファ.
			sizeof(cb));	// コピー元のバッファサイズ.

		// バッファ内のデータの書き換え終了時にUnmap.
		m_pContext11->Unmap(m_pCBufferPerFrame, 0);
	}

	// このコンスタントバッファをどのシェーダで使用するか？.
	m_pContext11->VSSetConstantBuffers(2, 1, &m_pCBufferPerFrame);	//頂点シェーダ.
	m_pContext11->PSSetConstantBuffers(2, 1, &m_pCBufferPerFrame);	//ピクセルシェーダ.


	// メッシュのレンダリング.
	DirectX::XMMATRIX View = CameraManager::GetInstance().GetViewMatrix();
	DirectX::XMMATRIX Proj = CameraManager::GetInstance().GetProjMatrix();
	RenderMesh(m_WorldMatrix, View, Proj);
}
//-------------------------------------------------------------------------------------------------------------------------------------

void StaticMesh::RenderDepth()
{
	// ワールド行列を計算.
	CalcWorldMatrix();

	// 使用するシェーダーを設定.
	ShaderResource::GetInstance().GetVSShadowMapStaticMesh()->SetVertexShader();
	ShaderResource::GetInstance().GetPSShadowMapStaticMesh()->SetPixelShader();


	DirectX::XMMATRIX lightView = LightManager::GetDirectionLight()->GetViewMatrix();
	DirectX::XMMATRIX lightProj = LightManager::GetDirectionLight()->GetProjectionMatrix();

	RenderMehDepsh(m_WorldMatrix, lightView, lightProj);
}

//-------------------------------------------------------------------------------------------------------------------------------------

const LPD3DXMESH& StaticMesh::GetMesh() const
{
	return m_Model.pMesh;
}

//-------------------------------------------------------------------------------------------------------------------------------------

ID3D11Buffer* StaticMesh::GetVertexBuffer()
{
	return m_pVertexBuffer;
}

//-------------------------------------------------------------------------------------------------------------------------------------

std::vector<ID3D11Buffer*> StaticMesh::GetIndexBuffer()
{
	std::vector<ID3D11Buffer*> indexBuffers;
	for (DWORD No = 0; No < m_NumAttr; No++)
	{
		indexBuffers.emplace_back(m_ppIndexBuffer[No]);
	}
	return indexBuffers;
}

//-------------------------------------------------------------------------------------------------------------------------------------

std::vector<DWORD> StaticMesh::GetIndex()
{
	std::vector<DWORD> index;

	// インデックスをstd::vector<DWORD>に追加.
	for (DWORD No = 0; No < m_NumAttr; No++)
	{
		index.emplace_back(m_pMaterials[m_AttrID[No]].dwNumFace);
	}

	return index;
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT StaticMesh::LoadXMesh(std::string lpFileName)
{
	const char* lp_filename = lpFileName.c_str();

	// ファイル名をコピー.
	_tcscpy_s(m_Model.FileName, _countof(m_Model.FileName), lp_filename);
	_tcscpy_s(m_ModelForRay.FileName, _countof(m_ModelForRay.FileName), lp_filename);

	// マテリアルﾊﾞｯﾌｧ.
	LPD3DXBUFFER pD3DXMtrlBuffer = nullptr;

	// Xファイルのロード.
	if (FAILED(D3DXLoadMeshFromX(
		m_Model.FileName,					// ファイル名.
		D3DXMESH_SYSTEMMEM			// システムメモリに読み込み.
		| D3DXMESH_32BIT,			// 32bit.
		m_pDevice9, nullptr,
		&m_Model.pD3DXMtrlBuffer,	// (out)マテリアル情報.
		nullptr,
		&m_Model.NumMaterials,		// (out)マテリアル数.
		&m_Model.pMesh)))		// (out)メッシュオブジェクト.
	{
		_ASSERT_EXPR(false, _T("Xファイル読込失敗"));
		return E_FAIL;
	}

	// Xファイルのロード(レイとの判定用に別設定で読み込む).
	if (FAILED(D3DXLoadMeshFromX(
		m_ModelForRay.FileName,						// ファイル名.
		D3DXMESH_SYSTEMMEM,				// システムメモリに読み込み.
		m_pDevice9, nullptr,
		&m_ModelForRay.pD3DXMtrlBuffer,	// (out)マテリアル情報.
		nullptr,
		&m_ModelForRay.NumMaterials,	// (out)マテリアル数.
		&m_ModelForRay.pMesh)))		// (out)メッシュオブジェクト.
	{
		_ASSERT_EXPR(false, _T("Xファイル読込失敗"));
		return E_FAIL;
	}

	return S_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT StaticMesh::CreateShader()
{
	ID3DBlob* pBlob = nullptr;
	ID3DBlob* pErrorBlob = nullptr;
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout;

	if (m_EnableTexture)
	{
		ShaderCompile(new std::string(VS_SHADER_FILE_PATH), "main", "vs_5_0", pBlob, pErrorBlob);

		// 頂点レイアウトを定義.
		layout.emplace_back("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);
		layout.emplace_back("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0);
		layout.emplace_back("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0);
	}
	else
	{
		ShaderCompile(new std::string(NO_TEX_VS_SHADER_FILE_PATH), "main", "vs_5_0", pBlob, pErrorBlob);

		// 頂点レイアウトを定義.
		layout.emplace_back("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0);
		layout.emplace_back("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0);
	}


	// VSの作成.
	m_pVertexShader->ConfigInputLayout(layout);
	m_pVertexShader->Init(pBlob);
	SAFE_RELEASE(pErrorBlob);


	if (m_EnableTexture)
	{
		ShaderCompile(new std::string(PS_SHADER_FILE_PATH), "main", "ps_5_0", pBlob, pErrorBlob);
	}
	else
	{
		ShaderCompile(new std::string(NO_TEX_PS_SHADER_FILE_PATH), "main", "ps_5_0", pBlob, pErrorBlob);
	}

	// PSの作成.
	m_pPixelShader->Init(pBlob);
	SAFE_RELEASE(pErrorBlob);

	return S_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT StaticMesh::CreateModel()
{
	// マテリアル作成.
	if (FAILED(CreateMaterials()))
	{
		return E_FAIL;
	}
	// インデックスバッファ作成.
	if (FAILED(CreateIndexBuffer()))
	{
		return E_FAIL;
	}
	// 頂点バッファ作成.
	if (FAILED(CreateVertexBuffer()))
	{
		return E_FAIL;
	}

	return S_OK;

}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT StaticMesh::CreateMaterials()
{
	// ファイルのパス(ディレクトリ)を確保.
	const int PATH_MAX = 64;
	TCHAR path[PATH_MAX] = _T("");
	int path_count = lstrlen(m_Model.FileName);
	for (int k = path_count; k >= 0; k--) {
		if (m_Model.FileName[k] == '\\') {
			for (int j = 0; j <= k; j++) {
				path[j] = m_Model.FileName[j];
			}
			path[k + 1] = '\0';
			break;
		}
	}

	// 読み込んだ情報から必要な情報を抜き出す.
	D3DXMATERIAL* d3dxMaterials
		= static_cast<D3DXMATERIAL*>(m_Model.pD3DXMtrlBuffer->GetBufferPointer());
	//マテリアル数分の領域を確保.
	m_pMaterials = new MY_MATERIAL[m_Model.NumMaterials]();
	// マテリアル数分繰り返し.
	for (DWORD No = 0; No < m_Model.NumMaterials; No++)
	{
		// マテリアル情報のコピー.
		// ディフューズ.
		m_pMaterials[No].Diffuse.x = d3dxMaterials[No].MatD3D.Diffuse.r;
		m_pMaterials[No].Diffuse.y = d3dxMaterials[No].MatD3D.Diffuse.g;
		m_pMaterials[No].Diffuse.z = d3dxMaterials[No].MatD3D.Diffuse.b;
		m_pMaterials[No].Diffuse.w = d3dxMaterials[No].MatD3D.Diffuse.a;
		// アンビエント.
		m_pMaterials[No].Ambient.x = d3dxMaterials[No].MatD3D.Ambient.r;
		m_pMaterials[No].Ambient.y = d3dxMaterials[No].MatD3D.Ambient.g;
		m_pMaterials[No].Ambient.z = d3dxMaterials[No].MatD3D.Ambient.b;
		m_pMaterials[No].Ambient.w = d3dxMaterials[No].MatD3D.Ambient.a;
		// スペキュラ.
		m_pMaterials[No].Specular.x = d3dxMaterials[No].MatD3D.Specular.r;
		m_pMaterials[No].Specular.y = d3dxMaterials[No].MatD3D.Specular.g;
		m_pMaterials[No].Specular.z = d3dxMaterials[No].MatD3D.Specular.b;
		m_pMaterials[No].Specular.w = d3dxMaterials[No].MatD3D.Specular.a;
		// スペキュラパワー.
		m_pMaterials[No].Power = d3dxMaterials[No].MatD3D.Power;
		// エミッシブ.
		m_pMaterials[No].Emissive.x = d3dxMaterials[No].MatD3D.Emissive.r;
		m_pMaterials[No].Emissive.y = d3dxMaterials[No].MatD3D.Emissive.g;
		m_pMaterials[No].Emissive.z = d3dxMaterials[No].MatD3D.Emissive.b;
		m_pMaterials[No].Emissive.w = d3dxMaterials[No].MatD3D.Emissive.a;

		// (その面に)テクスチャが貼られているか？.
		if (d3dxMaterials[No].pTextureFilename != nullptr
			&& strlen(d3dxMaterials[No].pTextureFilename) > 0)
		{
#ifdef UNICODE
			WCHAR TexFilename_w[256] = L"";
			// テクスチャ名のサイズを取得.
			size_t charSize = strlen(d3dxMaterials[No].pTextureFilename) + 1;
			size_t ret;	//変換された文字数.

			// マルチバイト文字のシーケンスを対応するワイド文字のシーケンスに変換します.
			errno_t err = mbstowcs_s(
				&ret,
				TexFilename_w,
				charSize,
				d3dxMaterials[No].pTextureFilename,
				_TRUNCATE);

			LPTSTR filename = TexFilename_w;
#else//#ifdef UNICODE
			LPTSTR filename = d3dxMaterials[No].pTextureFilename;
#endif//#ifdef UNICODE

			// テクスチャありのフラグを立てる.
			m_EnableTexture = true;

			// パスをコピー.
			lstrcpy(m_pMaterials[No].TextureName, path);
			// テクスチャファイル名を連結.
			lstrcat(m_pMaterials[No].TextureName, filename);

			// テクスチャ作成.
			if (FAILED(D3DX11CreateShaderResourceViewFromFile(
				m_pDevice11, m_pMaterials[No].TextureName,//テクスチャファイル名.
				nullptr, nullptr,
				&m_pMaterials[No].pTexture,//(out)テクスチャオブジェクト.
				nullptr)))
			{
				_ASSERT_EXPR(false, _T("テクスチャ作成失敗"));
				return E_FAIL;
			}
		}
	}

	return S_OK;

}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT StaticMesh::CreateIndexBuffer()
{
	D3D11_BUFFER_DESC	bd;	// Dx11バッファ構造体.
	D3D11_SUBRESOURCE_DATA	InitData;// 初期化データ.

	// マテリアル数分の領域を確保.
	m_ppIndexBuffer = new ID3D11Buffer * [m_Model.NumMaterials]();
	for (DWORD No = 0; No < m_Model.NumMaterials; No++) {
		m_ppIndexBuffer[No] = nullptr;
	}

	// メッシュの属性情報を得る.
	// 属性情報でインデックスバッファから細かいマテリアルごとのインデックスバッファを分離できる.
	D3DXATTRIBUTERANGE* pAttrTable = nullptr;

	// メッシュの面および頂点の順番変更を制御し、パフォーマンスを最適化する.
	// D3DXMESHOPT_COMPACT : 面の順番を変更し、使用されていない頂点と面を削除する.
	// D3DXMESHOPT_ATTRSORT : パフォーマンスを上げる為、面の順番を変更して最適化を行う.
	m_Model.pMesh->OptimizeInplace(D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT,
		nullptr, nullptr, nullptr, nullptr);
	// 属性テーブルの取得.
	m_Model.pMesh->GetAttributeTable(nullptr, &m_NumAttr);
	pAttrTable = new D3DXATTRIBUTERANGE[m_NumAttr];
	if (FAILED(m_Model.pMesh->GetAttributeTable(pAttrTable, &m_NumAttr)))
	{
		_ASSERT_EXPR(false, _T("属性テーブル取得失敗"));
		return E_FAIL;
	}

	// 同じくLockしないと取り出せない.
	int* pIndex = nullptr;
	m_Model.pMesh->LockIndexBuffer(
		D3DLOCK_READONLY, (void**)&pIndex);
	// 属性ごとのインデックスバッファを作成.
	for (DWORD No = 0; No < m_NumAttr; No++)
	{
		m_AttrID[No] = pAttrTable[No].AttribId;
		// Dx9のインデックスバッファからの情報で、Dx11のインデックスバッファを作成.
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth
			= sizeof(int) * pAttrTable[No].FaceCount * 3;//面数×3で頂点数.
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		// 大きいインデックスバッファ内のオフセット(×3する).
		InitData.pSysMem = &pIndex[pAttrTable[No].FaceStart * 3];

		if (FAILED(m_pDevice11->CreateBuffer(
			&bd, &InitData, &m_ppIndexBuffer[No])))
		{
			_ASSERT_EXPR(false, _T("インデックスバッファ作成失敗"));
			return E_FAIL;
		}
		// 面の数をコピー.
		m_pMaterials[m_AttrID[No]].dwNumFace = pAttrTable[No].FaceCount;
	}
	// 属性テーブルの削除.
	delete[] pAttrTable;
	// 使用済みのインデックスバッファの解放.
	m_Model.pMesh->UnlockIndexBuffer();

	return S_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT StaticMesh::CreateVertexBuffer()
{
	D3D11_BUFFER_DESC	bd;	// Dx11バッファ構造体.
	D3D11_SUBRESOURCE_DATA	InitData;// 初期化データ.

	// Dx9の場合、mapではなくLockで頂点バッファからデータを取り出す.
	LPDIRECT3DVERTEXBUFFER9 pVB = nullptr;
	m_Model.pMesh->GetVertexBuffer(&pVB);
	DWORD dwStride = m_Model.pMesh->GetNumBytesPerVertex();
	BYTE* pVertices = nullptr;
	VERTEX* pVertex = nullptr;
	if (SUCCEEDED(
		pVB->Lock(0, 0, (VOID**)&pVertices, 0)))
	{
		pVertex = (VERTEX*)pVertices;
		// Dx9の頂点バッファからの情報で、Dx11頂点バッファを作成.
		bd.Usage = D3D11_USAGE_DEFAULT;
		// 頂点を格納するのに必要なバイト数.
		bd.ByteWidth = m_Model.pMesh->GetNumBytesPerVertex() * m_Model.pMesh->GetNumVertices();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		InitData.pSysMem = pVertex;
		if (FAILED(m_pDevice11->CreateBuffer(
			&bd, &InitData, &m_pVertexBuffer)))
		{
			_ASSERT_EXPR(false, _T("頂点バッファ作成失敗"));
			return E_FAIL;
		}
		pVB->Unlock();
	}
	SAFE_RELEASE(pVB);	// 頂点バッファ解放.
	return S_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT StaticMesh::CreateConstantBuffer()
{
	// コンスタントバッファ(メッシュ用).
	D3D11_BUFFER_DESC cb;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	// コンスタントバッファを指定.
	cb.ByteWidth = sizeof(CBUFFER_PER_MESH);	// コンスタントバッファのサイズ.
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// 書き込みでアクセス.
	cb.MiscFlags = 0;				// その他のフラグ(未使用).
	cb.StructureByteStride = 0;		// 構造体のサイズ(未使用).
	cb.Usage = D3D11_USAGE_DYNAMIC;	// 使用方法:直接書き込み.

	// コンスタントバッファの作成.
	if (FAILED(
		m_pDevice11->CreateBuffer(&cb, nullptr, &m_pCBufferPerMesh)))
	{
		_ASSERT_EXPR(false, _T("コンスタントバッファ(メッシュ)作成失敗"));
		return E_FAIL;
	}

	// コンスタントバッファ(マテリアル用).
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;		// コンスタントバッファを指定.
	cb.ByteWidth = sizeof(CBUFFER_PER_MATERIAL);	// コンスタントバッファのサイズ.
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;		// 書き込みでアクセス.
	cb.MiscFlags = 0;				// その他のフラグ(未使用).
	cb.StructureByteStride = 0;		// 構造体のサイズ(未使用).
	cb.Usage = D3D11_USAGE_DYNAMIC;	// 使用方法:直接書き込み.

	// コンスタントバッファの作成.
	if (FAILED(
		m_pDevice11->CreateBuffer(&cb, nullptr, &m_pCBufferPerMaterial)))
	{
		_ASSERT_EXPR(false, _T("コンスタントバッファ(マテリアル用)作成失敗"));
		return E_FAIL;
	}

	//コンスタントバッファ(ﾌﾚｰﾑ用).
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	// コンスタントバッファを指定.
	cb.ByteWidth = sizeof(CBUFFER_PER_FRAME);	// コンスタントバッファのサイズ.
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	// 書き込みでアクセス.
	cb.MiscFlags = 0;				// その他のフラグ(未使用).
	cb.StructureByteStride = 0;		// 構造体のサイズ(未使用).
	cb.Usage = D3D11_USAGE_DYNAMIC;	// 使用方法:直接書き込み.

	// コンスタントバッファの作成.
	if (FAILED(
		m_pDevice11->CreateBuffer(&cb, nullptr, &m_pCBufferPerFrame)))
	{
		_ASSERT_EXPR(false, _T("コンスタントバッファ(フレーム用)作成失敗"));
		return E_FAIL;
	}

	return S_OK;

}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT StaticMesh::CreateSampler()
{
	// テクスチャ用のサンプラ構造体.
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory(&samDesc, sizeof(samDesc));
	samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;// リニアフィルタ(線形補間).
	// POINT:高速だが粗い.
	samDesc.AddressU
		= D3D11_TEXTURE_ADDRESS_WRAP;// ラッピングモード(WRAP:繰り返し).
	samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	// MIRROR: 反転繰り返し.
	// CLAMP : 端の模様を引き伸ばす.
	// BORDER: 別途境界色を決める.

	// サンプラ作成.
	if (FAILED(m_pDevice11->CreateSamplerState(
		&samDesc, &m_pSampleLinear)))// (out)サンプラ.
	{
		_ASSERT_EXPR(false, _T("サンプラ作成失敗"));
		return E_FAIL;
	}

	return S_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------
void StaticMesh::RenderMesh(DirectX::XMMATRIX& mWorld, DirectX::XMMATRIX& mView, DirectX::XMMATRIX& mProj)
{
	// シェーダのコンスタントバッファに各種データを渡す.
	D3D11_MAPPED_SUBRESOURCE pData;

	// バッファ内のデータの書き換え開始時にMap.
	if (SUCCEEDED(m_pContext11->Map(
		m_pCBufferPerMesh, 0,
		D3D11_MAP_WRITE_DISCARD,
		0, &pData)))
	{
		// コンスタントバッファ(メッシュ用).
		CBUFFER_PER_MESH cb;

		// 1. ワールド行列の転置
		// D3DXMatrixTranspose(&cb.mW, &cb.mW) の代替
		cb.mW = DirectX::XMMatrixTranspose(mWorld);

		// 2. WVP行列の計算と転置
		// DirectX::XMMATRIX mWVP = mWorld * mView * mProj; の代替
		DirectX::XMMATRIX mWVP_temp = DirectX::XMMatrixMultiply(mWorld, mView);
		mWVP_temp = DirectX::XMMatrixMultiply(mWVP_temp, mProj);

		// D3DXMatrixTranspose(&mWVP, &mWVP) の代替
		DirectX::XMMATRIX mWVP = DirectX::XMMatrixTranspose(mWVP_temp);
		cb.mWVP = mWVP;

		// 3. WLP行列の計算と転置
		DirectX::XMMATRIX lightView = LightManager::GetDirectionLight()->GetViewMatrix();
		DirectX::XMMATRIX lightProj = LightManager::GetDirectionLight()->GetProjectionMatrix();

		// DirectX::XMMATRIX mWLP = mWorld * lightView * lightProj; の代替
		DirectX::XMMATRIX mWLP_temp = DirectX::XMMatrixMultiply(mWorld, lightView);
		mWLP_temp = DirectX::XMMatrixMultiply(mWLP_temp, lightProj);

		// D3DXMatrixTranspose(&mWLP, &mWLP) の代替
		DirectX::XMMATRIX mWLP = DirectX::XMMatrixTranspose(mWLP_temp);
		cb.mWLVP = mWLP;

		memcpy_s(
			pData.pData,	// コピー先のバッファ.
			pData.RowPitch,	// コピー先のバッファサイズ.
			(void*)(&cb),	// コピー元のバッファ.
			sizeof(cb));	// コピー元のバッファサイズ.

		// バッファ内のデータの書き換え終了時にUnmap.
		m_pContext11->Unmap(m_pCBufferPerMesh, 0);
	}

	// (以下、D3D11 APIおよびD3DX由来のメッシュアクセスは変更なし)

	m_pContext11->VSSetConstantBuffers(0, 1, &m_pCBufferPerMesh);
	m_pContext11->PSSetConstantBuffers(0, 1, &m_pCBufferPerMesh);

	m_pContext11->IASetInputLayout(m_pVertexShader->GetInputLayout());

	m_pContext11->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 頂点バッファをセット.
	UINT stride = m_Model.pMesh->GetNumBytesPerVertex();
	UINT offset = 0;
	m_pContext11->IASetVertexBuffers(
		0, 1, &m_pVertexBuffer, &stride, &offset);

	// 属性の数だけ、それぞれの属性のインデックスバッファを描画.
	for (DWORD No = 0; No < m_NumAttr; No++)
	{
		// 使用されていないマテリアル対策.
		if (m_pMaterials[m_AttrID[No]].dwNumFace == 0) {
			continue;
		}
		// インデックスバッファをセット.
		m_pContext11->IASetIndexBuffer(
			m_ppIndexBuffer[No], DXGI_FORMAT_R32_UINT, 0);

		// マテリアルの各要素をシェーダに渡す.
		D3D11_MAPPED_SUBRESOURCE pDataMat;
		if (SUCCEEDED(
			m_pContext11->Map(m_pCBufferPerMaterial,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pDataMat)))
		{
			// コンスタントバッファ(マテリアル用).
			CBUFFER_PER_MATERIAL cb;
			// ディフューズ,アンビエント,スペキュラをシェーダに渡す.
			cb.Diffuse = m_pMaterials[m_AttrID[No]].Diffuse;
			cb.Ambient = m_pMaterials[m_AttrID[No]].Ambient;
			cb.Specular = m_pMaterials[m_AttrID[No]].Specular;
			cb.Emissive = m_pMaterials[m_AttrID[No]].Emissive;

			memcpy_s(pDataMat.pData, pDataMat.RowPitch,
				(void*)&cb, sizeof(cb));

			m_pContext11->Unmap(m_pCBufferPerMaterial, 0);
		}

		// このコンスタントバッファをどのシェーダで使うか？.
		m_pContext11->VSSetConstantBuffers(1, 1, &m_pCBufferPerMaterial);
		m_pContext11->PSSetConstantBuffers(1, 1, &m_pCBufferPerMaterial);

		// テクスチャをシェーダに渡す.
		if (m_pMaterials[m_AttrID[No]].pTexture != nullptr) {
			// テクスチャがあるとき.
			m_pContext11->PSSetSamplers(0, 1, &m_pSampleLinear);
			m_pContext11->PSSetShaderResources(
				0, 1, &m_pMaterials[m_AttrID[No]].pTexture);
		}
		else {
			// テクスチャがないとき.
			ID3D11ShaderResourceView* pNothing[1] = { 0 };
			m_pContext11->PSSetShaderResources(0, 1, pNothing);
		}

		// プリミティブ(ポリゴン)をレンダリング.
		m_pContext11->DrawIndexed(
			m_pMaterials[m_AttrID[No]].dwNumFace * 3, 0, 0);
	}
}
//-------------------------------------------------------------------------------------------------------------------------------------

void StaticMesh::RenderMehDepsh(DirectX::XMMATRIX& mWorld, DirectX::XMMATRIX& mView, DirectX::XMMATRIX& mProj)
{
	ID3D11Buffer* CBuffer = Shadow::GetInstance().GetCBuffer();
	// シェーダのコンスタントバッファに各種データを渡す.
	D3D11_MAPPED_SUBRESOURCE pData;


	// ワールドライトビュープロジェクション行列を渡す.

	// 【D3DXからXMへ】行列乗算をXMMatrixMultiplyに置き換え
	DirectX::XMMATRIX mWLP_temp = DirectX::XMMatrixMultiply(mWorld, mView);
	mWLP_temp = DirectX::XMMatrixMultiply(mWLP_temp, mProj);

	// 【D3DXからXMへ】行列の転置をXMMatrixTransposeに置き換え
	DirectX::XMMATRIX mWLP = DirectX::XMMatrixTranspose(mWLP_temp);

	// バッファ内のデータの書き換え開始時にMap.
	if (SUCCEEDED(m_pContext11->Map(
		CBuffer, 0,
		D3D11_MAP_WRITE_DISCARD,
		0, &pData)))
	{
		// コンスタントバッファ(メッシュ用).
		Shadow::CBUFFER cb;
		cb.mWLP = mWLP; // XMMATRIXをそのまま代入

		memcpy_s(
			pData.pData,	// コピー先のバッファ.
			pData.RowPitch,	// コピー先のバッファサイズ.
			(void*)(&cb),	// コピー元のバッファ.
			sizeof(cb));	// コピー元のバッファサイズ.

		// バッファ内のデータの書き換え終了時にUnmap.
		m_pContext11->Unmap(CBuffer, 0);
	}

	// このコンスタントバッファをどのシェーダで使用するか？.
	m_pContext11->VSSetConstantBuffers(0, 1, &CBuffer);	// 頂点シェーダ.
	m_pContext11->PSSetConstantBuffers(0, 1, &CBuffer);	// ピクセルシェーダ.

	// 頂点インプットレイアウトをセット.
	ShaderResource::GetInstance().GetVSShadowMapStaticMesh()->SetInputLayout();

	// プリミティブ・トポロジーをセット.
	m_pContext11->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 頂点バッファをセット.
	// D3DXのアクセスはそのまま維持 (環境全体を合わせないため)
	UINT stride = m_Model.pMesh->GetNumBytesPerVertex();
	UINT offset = 0;
	m_pContext11->IASetVertexBuffers(
		0, 1, &m_pVertexBuffer, &stride, &offset);

	// 属性の数だけ、それぞれの属性のインデックスバッファを描画.
	for (DWORD No = 0; No < m_NumAttr; No++)
	{
		// D3DXのアクセスはそのまま維持 (環境全体を合わせないため)
		// 使用されていないマテリアル対策.
		if (m_pMaterials[m_AttrID[No]].dwNumFace == 0) {
			continue;
		}
		// インデックスバッファをセット.
		m_pContext11->IASetIndexBuffer(
			m_ppIndexBuffer[No], DXGI_FORMAT_R32_UINT, 0);

		// マテリアルの各要素をシェーダに渡す（このブロックは変更なし）.
		D3D11_MAPPED_SUBRESOURCE pDataMat;
		if (SUCCEEDED(
			m_pContext11->Map(m_pCBufferPerMaterial,
			0, D3D11_MAP_WRITE_DISCARD, 0, &pDataMat)))
		{
			// コンスタントバッファ(マテリアル用).
			CBUFFER_PER_MATERIAL cb;
			// ディフューズ,アンビエント,スペキュラをシェーダに渡す.
			cb.Diffuse = m_pMaterials[m_AttrID[No]].Diffuse;
			cb.Ambient = m_pMaterials[m_AttrID[No]].Ambient;
			cb.Specular = m_pMaterials[m_AttrID[No]].Specular;

			memcpy_s(pDataMat.pData, pDataMat.RowPitch,
				(void*)&cb, sizeof(cb));

			m_pContext11->Unmap(m_pCBufferPerMaterial, 0);
		}

		// このコンスタントバッファをどのシェーダで使うか？.
		m_pContext11->VSSetConstantBuffers(1, 1, &m_pCBufferPerMaterial);
		m_pContext11->PSSetConstantBuffers(1, 1, &m_pCBufferPerMaterial);

		// テクスチャをシェーダに渡す. (変更なし)
		if (m_pMaterials[m_AttrID[No]].pTexture != nullptr) {
			// テクスチャがあるとき.
			m_pContext11->PSSetSamplers(0, 1, &m_pSampleLinear);
			m_pContext11->PSSetShaderResources(
				0, 1, &m_pMaterials[m_AttrID[No]].pTexture);
		}
		else {
			// テクスチャがないとき.
			ID3D11ShaderResourceView* pNothing[1] = { 0 };
			m_pContext11->PSSetShaderResources(0, 1, pNothing);
		}

		// プリミティブ(ポリゴン)をレンダリング.
		// D3DXのアクセスはそのまま維持 (環境全体を合わせないため)
		m_pContext11->DrawIndexed(
			m_pMaterials[m_AttrID[No]].dwNumFace * 3, 0, 0);
	}
}
//-------------------------------------------------------------------------------------------------------------------------------------

float StaticMesh::GetModelRadius()
{
	// D3D9/D3DXのヘッダも必要に応じてインクルードされていると仮定

	if (m_OriginalRadius <= 0.0f)
	{
		Microsoft::WRL::ComPtr<IDirect3DVertexBuffer9> pVB = nullptr;
		void* pVertices = nullptr;

		// 頂点バッファを取得 (D3D9 APIを維持)
		MyAssert::IsFailed(_T("頂点バッファの取得に失敗"), &ID3DXMesh::GetVertexBuffer, *m_Model.pMesh, pVB.GetAddressOf());

		// メッシュの頂点バッファをロックする (D3D9 APIを維持)
		MyAssert::IsFailed(_T("頂点バッファのロックに失敗"), &IDirect3DVertexBuffer9::Lock, pVB.Get(), 0, 0, &pVertices, 0);

		// ---------------------------------------------------------------------------------
		// D3DXComputeBoundingSphereのカスタム置き換えロジック (XMVectorMin/Maxを使用)
		// ---------------------------------------------------------------------------------

		// 頂点情報のサイズを取得. (D3DXGetFVFVertexSizeの代わりに、XMで走査)
		// ここでは、頂点データがXMFLOAT3から始まっており、全体のストライドは16バイトであると仮定
		// (元のコードの * 16 の部分を基に)
		UINT stride = D3DXGetFVFVertexSize(GetMesh()->GetFVF()); // D3DXGetFVFVertexSizeは計算関数ではないためそのまま維持
		// もしD3DXGetFVFVertexSizeを使いたくない場合、ここでは stride = 16; などと手動で指定が必要です。

		// 頂点の数を取得
		DWORD numVertices = GetMesh()->GetNumVertices();

		// 最小・最大位置を追跡するためのXMVECTOR
		DirectX::XMVECTOR vMin = DirectX::XMVectorSet(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), 1.0f);
		DirectX::XMVECTOR vMax = DirectX::XMVectorSet(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), 1.0f);

		BYTE* pCurrentVertex = static_cast<BYTE*>(pVertices);

		for (DWORD i = 0; i < numVertices; ++i)
		{
			// 頂点情報のアドレスから座標(XMFLOAT3)のアドレスを読み取る.
			// (元のコードの * 16 の部分は stride の乗算に置き換え)
			const DirectX::XMFLOAT3* posFloat = reinterpret_cast<const DirectX::XMFLOAT3*>(pCurrentVertex);

			// XMVECTORにロードし、最小・最大値を更新 (SIMD演算)
			DirectX::XMVECTOR vPos = DirectX::XMLoadFloat3(posFloat);
			vMin = DirectX::XMVectorMin(vMin, vPos);
			vMax = DirectX::XMVectorMax(vMax, vPos);

			// 次の頂点へ移動
			pCurrentVertex += stride;
		}

		// Min/Maxから中心座標 (Center) と半径 (Radius) を計算
		DirectX::XMVECTOR vCenter = DirectX::XMVectorScale(DirectX::XMVectorAdd(vMin, vMax), 0.5f);
		DirectX::XMVECTOR vDiff = DirectX::XMVectorSubtract(vMax, vCenter);
		DirectX::XMVECTOR vRadius = DirectX::XMVector3Length(vDiff);

		// 結果をメンバー変数にストア
		// D3DXVECTOR3 out; -> m_CenterPosition (XMFLOAT3) へのストア
		DirectX::XMStoreFloat3(&m_CenterPosition, vCenter);
		DirectX::XMStoreFloat(&m_OriginalRadius, vRadius);


		//メッシュの頂点バッファをアンロックする (D3D9 APIを維持)
		if (pVB.Get() != nullptr) {
			pVB->Unlock();
		}

		// 冗長なループとD3DXの呼び出しは削除
		/* 削除されたコード
		BYTE* vertexData = static_cast<BYTE*>(pVertices);
		std::vector<DirectX::XMFLOAT3> Vpos;
		for (DWORD i = 1; i < m_Model.pMesh->GetNumVertices(); ++i)
		{
			DirectX::XMFLOAT3* pos = reinterpret_cast<DirectX::XMFLOAT3*>(vertexData + i * 16);
			Vpos.push_back(*pos);
		}
		auto wpm = Vpos;

		D3DXComputeBoundingSphere(
			static_cast<DirectX::XMFLOAT3*>(pVertices),
			GetMesh()->GetNumVertices(),               
			D3DXGetFVFVertexSize(GetMesh()->GetFVF()), 
			&m_CenterPosition,                         
			&m_OriginalRadius);                         
		*/
	}

	m_Radius = m_OriginalRadius * (std::max)(m_spTransform.Scale.x, (std::max)(m_spTransform.Scale.y, m_spTransform.Scale.z));
	return m_Radius;
}

