/***************************************************************************************************
*	CSkinMeshCode Version 2.40
*	LastUpdate	: 2024/06/14.
**/
#include "SkinMesh.h"
#include "Graphic/DirectX/DirectX9/DirectX9.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"

#include "System/Singleton/CameraManager/CameraManager.h"
#include "Graphic/Light/LightManager.h"
#include "Graphic/Light/DirectionLight/DirectionLight.h"
#include "Graphic/Shadow/Shadow.h"
#include "Graphic/RenderTarget/RenderTargetManager.h"
#include "Graphic/Shader/ShaderResource.h"
#include "Graphic/Shader/ShaderCompile.h"

#include "System/Singleton/SceneManager/SceneManager.h"
#include <string>

#include <stdlib.h>	//マルチバイト文字→Unicode文字変換で必要.
#include <locale.h>

#include <crtdbg.h>

#include <memory>
#include <algorithm>

//入れ替え関数.
template <typename T>
void swap(T& a, T& b)
{
	T temp = a;
	a = b;
	b = temp;
}

//範囲内に納める関数.
template <typename T>
const T& clamp(const T& val, const T& low, const T& high)
{
	return val > high ? high : val < low ? low : val;
}


namespace {
constexpr char VS_FILE_PATH[] = "Data\\Shader\\SkinMesh\\SkinMeshVS.hlsl";
constexpr char PS_FILE_PATH[] = "Data\\Shader\\SkinMesh\\SkinMeshPS.hlsl";
}


//コンスタントバッファを設定するスロット番号.
enum enCBSlot
{
	Mesh,		//メッシュ.
	Material,	//マテリアル.
	Frame,		//フレーム.
	Bones,		//ボーン.
};


SkinMesh::SkinMesh()
	: m_pCBufferPerBone(nullptr)
	, m_mView(DirectX::XMMatrixIdentity())
	, m_mProj(DirectX::XMMatrixIdentity())
	, m_CamPos({ 0.0f, 0.0f, 0.0f })
	, m_AnimSpeed(0.001)
	, m_AnimTime()
	, m_pD3dxMesh(nullptr)
	, m_FilePath()
	, m_Frame()
, m_GlobalAlpha(1.0f)
{
}

// (no static scene alpha)

//-------------------------------------------------------------------------------------------------------------------------------------

SkinMesh::~SkinMesh()
{
	//解放処理.
	Release();

	//シェーダやサンプラ関係.
	SAFE_RELEASE(m_pSampleLinear);

	//コンスタントバッファ関係.
	SAFE_RELEASE(m_pCBufferPerBone);
	SAFE_RELEASE(m_pCBufferPerFrame);
	SAFE_RELEASE(m_pCBufferPerMaterial);
	SAFE_RELEASE(m_pCBufferPerMesh);

	SAFE_RELEASE(m_pD3dxMesh);
//-------------------------------------------------------------------------------------------------------------------------------------
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT SkinMesh::Init(std::string FileName, const std::string& Name)
{
	m_pDevice9 = DirectX9::GetInstance().GetDevice();

	m_pDevice11 = DirectX11::GetInstance().GetDevice();
	m_pContext11 = DirectX11::GetInstance().GetContext();

	// 資源名を保存.
	m_ResourceName = Name;

	//モデル読み込み.
	if (FAILED(LoadXMesh(FileName)))
	{
		return E_FAIL;
	}
	//シェーダの作成.
	if (FAILED(CreateShader()))
	{
		return E_FAIL;
	}
	//コンスタントバッファ(メッシュごと).
	if (FAILED(
		CreateCBuffer(&m_pCBufferPerMesh, sizeof(CBUFFER_PER_MESH))))
	{
		return E_FAIL;
	}
	//コンスタントバッファ(メッシュごと).
	if (FAILED(
		CreateCBuffer(&m_pCBufferPerMaterial, sizeof(CBUFFER_PER_MATERIAL))))
	{
		return E_FAIL;
	}
	//コンスタントバッファ(メッシュごと).
	if (FAILED(
		CreateCBuffer(&m_pCBufferPerFrame, sizeof(CBUFFER_PER_FRAME))))
	{
		return E_FAIL;
	}
	//コンスタントバッファ(メッシュごと).
	if (FAILED(
		CreateCBuffer(&m_pCBufferPerBone, sizeof(CBUFFER_PER_BONES))))
	{
		return E_FAIL;
	}
	//テクスチャー用サンプラー作成.
	if (FAILED(CreateSampler(&m_pSampleLinear)))
	{
		return E_FAIL;
	}

	return S_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT	SkinMesh::CreateShader()
{
	ID3DBlob* pBlob = nullptr;
	ID3DBlob* pErrorBlob = nullptr;

	// 頂点インプットレイアウトを定義.
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0,							  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_INDEX",	0, DXGI_FORMAT_R32G32B32A32_UINT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONE_WEIGHT",0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// VSの作成.
	ShaderCompile(new std::string(VS_FILE_PATH), "main", "vs_5_0", pBlob, pErrorBlob);
	m_pVertexShader->ConfigInputLayout(layout);
	m_pVertexShader->Init(pBlob);
	SAFE_RELEASE(pErrorBlob);


	// PSの作成.
	ShaderCompile(new std::string(PS_FILE_PATH), "main", "ps_5_0", pBlob, pErrorBlob);
	m_pPixelShader->Init(pBlob);
	SAFE_RELEASE(pErrorBlob);

	return S_OK;
}


//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT SkinMesh::ReadSkinInfo(
	MYMESHCONTAINER* pContainer, SKIN_VERTEX* pVB, SKIN_PARTS_MESH* pParts)
{
	//Xファイルから抽出すべき情報は、
	//「頂点ごとのボーンインデックス」「頂点ごとのボーンウェイト」.
	//「バインド行列」「ポーズ行列」の4項目.

	int i, k, m, n;			//各種ループ変数.
	int NumVertex = 0;	//頂点数.
	int NumBone = 0;	//ボーン数.

	//頂点数.
	NumVertex = m_pD3dxMesh->GetNumVertices(pContainer);
	//ボーン数.
	NumBone = m_pD3dxMesh->GetNumBones(pContainer);

	//それぞれのボーンに影響を受ける頂点を調べる.
	//そこから逆に、頂点ベースでボーンインデックス・重みを整頓する.
	for (i = 0; i < NumBone; i++)
	{
		//このボーンに影響を受ける頂点数.
		auto NumIndex = m_pD3dxMesh->GetNumBoneVertices(pContainer, i);

		auto pIndex = std::make_unique<int[]>(NumIndex);
		auto pWeight = std::make_unique<double[]>(NumIndex);

		for (k = 0; k < NumIndex; k++)
		{
			pIndex[k] = m_pD3dxMesh->GetBoneVerticesIndices(pContainer, i, k);
			pWeight[k] = m_pD3dxMesh->GetBoneVerticesWeights(pContainer, i, k);
		}

		//頂点側からインデックスをたどって、頂点サイドで整理する.
		for (k = 0; k < NumIndex; k++)
		{
			//XファイルやCGソフトがボーン4本以内とは限らない.
			//5本以上の場合は、重みの大きい順に4本に絞る.

			SKIN_VERTEX* pV = &pVB[pIndex[k]];

			//ウェイトの大きさ順にソート(バブルソート).
			for (m = 4; m > 1; m--)
			{
				for (n = 1; n < m; n++)
				{
					if (pV->BoneWeight[n - 1] < pV->BoneWeight[n])
					{
						swap(pV->BoneWeight[n - 1], pV->BoneWeight[n]);
						swap(pV->BoneIndex[n - 1], pV->BoneIndex[n]);
					}
				}
			}
			//ソート後は、最後の要素に一番小さいウェイトが入ってるはず.
			bool flag = false;
			for (m = 0; m < 4; m++)
			{
				if (pV->BoneWeight[m] == 0)
				{
					flag = true;
					pV->BoneIndex[m] = i;
					pV->BoneWeight[m] = static_cast<float>(pWeight[k]);
					break;
				}
			}
			if (flag == false)
			{
				pV->BoneIndex[3] = i;
				pV->BoneWeight[3] = static_cast<float>(pWeight[k]);
				break;
			}
		}
	}


	//ボーン生成.
	pParts->NumBone = NumBone;
	pParts->pBoneArray = new BONE[NumBone]();

	//ポーズ行列を得る(初期ポーズ).
	for (i = 0; i < pParts->NumBone; i++)
	{
		pParts->pBoneArray[i].mBindPose = m_pD3dxMesh->GetBindPose(pContainer, i);
	}

	return S_OK;

	return S_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT SkinMesh::LoadXMesh(std::string FileName)
{
	const char* lp_filename = FileName.c_str();

	//ファイル名をパスごと取得.
	// FIX 2: Use the safe function _tcscpy_s (assuming m_FilePath is a TCHAR array)
	// Note: You must ensure m_FilePath is large enough.
	_tcscpy_s(m_FilePath, _countof(m_FilePath), lp_filename);

	//ファイル名をパスごと取得.
	lstrcpy(m_FilePath, lp_filename);

	//Xファイル読み込み.
	m_pD3dxMesh = new D3DXPARSER();
	m_pD3dxMesh->LoadMeshFromX(m_pDevice9, lp_filename);


	//全てのメッシュを作成する.
	BuildAllMesh(m_pD3dxMesh->m_pFrameRoot);

	return S_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT SkinMesh::CreateIndexBuffer(
	DWORD Size, int* pIndex, ID3D11Buffer** ppIndexBuffer)
{
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = Size;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = pIndex;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	if (FAILED(
		m_pDevice11->CreateBuffer(&bd, &InitData, ppIndexBuffer)))
	{
		return E_FAIL;
	}

	return S_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------

void SkinMesh::Render(LPD3DXANIMATIONCONTROLLER pAC)
{
	m_mView = CameraManager::GetInstance().GetViewMatrix();
	m_mProj = CameraManager::GetInstance().GetProjMatrix();
	m_CamPos = CameraManager::GetInstance().GetPosition();

	//使用するシェーダのセット.
	m_pContext11->VSSetShader(m_pVertexShader->GetVertexShader(), nullptr, 0);
	m_pContext11->PSSetShader(m_pPixelShader->GetPixelShader(), nullptr, 0);

	//シャドウマップを渡す.
	auto pShadowMapSRV = RenderTargetManager::GetInstance().GetRenderTarget("ShadowMap").GetShaderResourceView();
	m_pContext11->PSSetShaderResources(1, 1, &pShadowMapSRV);

	if (pAC == nullptr)
	{
		if (m_pD3dxMesh->m_pAnimController)
		{
			m_pD3dxMesh->m_pAnimController->AdvanceTime(m_AnimSpeed, nullptr);
		}
	}
	else
	{
		pAC->AdvanceTime(m_AnimSpeed, nullptr);
	}
  
    D3DXMATRIX m;
    D3DXMatrixIdentity(&m);
    m_pD3dxMesh->UpdateFrameMatrices(m_pD3dxMesh->m_pFrameRoot, &m);

    DrawFrame(m_pD3dxMesh->m_pFrameRoot);
}

LPD3DXFRAME SkinMesh::FindFrame(LPD3DXFRAME frame, LPCSTR frameName)
{
	if (frame)
	{
		if (frame->Name && strcmp(frame->Name, frameName) == 0)
		{
			return frame;
		}
		if (frame->pFrameSibling)
		{
			LPD3DXFRAME found = FindFrame(frame->pFrameSibling, frameName);
			if (found) return found;
		}
		if (frame->pFrameFirstChild)
		{
			LPD3DXFRAME found = FindFrame(frame->pFrameFirstChild, frameName);
			if (found) return found;
		}
	}
	return nullptr;
}

LPD3DXFRAME SkinMesh::GetFrameByName(LPCSTR frameName)
{
	if (m_pD3dxMesh && m_pD3dxMesh->m_pFrameRoot)
	{
		return FindFrame(m_pD3dxMesh->m_pFrameRoot, frameName);
	}
	return nullptr;
}

//-------------------------------------------------------------------------------------------------------------------------------------

void SkinMesh::RenderDepth(const LPD3DXANIMATIONCONTROLLER pAC)
{
	m_mView = CameraManager::GetInstance().GetViewMatrix();
	m_mProj = CameraManager::GetInstance().GetProjMatrix();
	m_CamPos = CameraManager::GetInstance().GetPosition();

	//使用するシェーダのセット.
	ShaderResource::GetInstance().GetVSShadowMapSkinMesh()->SetVertexShader();
	ShaderResource::GetInstance().GetPSShadowMapSkinMesh()->SetPixelShader();


	D3DXMATRIX m;
	D3DXMatrixIdentity(&m);
	m_pD3dxMesh->UpdateFrameMatrices(m_pD3dxMesh->m_pFrameRoot, &m);
	DrawFrameDepth(m_pD3dxMesh->m_pFrameRoot);
}

//-------------------------------------------------------------------------------------------------------------------------------------

void SkinMesh::BuildAllMesh(D3DXFRAME* pFrame)
{
	if (pFrame == nullptr) { return; }	

	if (pFrame && pFrame->pMeshContainer)
	{
		CreateAppMeshFromD3DXMesh(pFrame);
	}

	//再帰関数.
	if (pFrame->pFrameSibling != nullptr)
	{
		BuildAllMesh(pFrame->pFrameSibling);
	}
	if (pFrame->pFrameFirstChild != nullptr)
	{
		BuildAllMesh(pFrame->pFrameFirstChild);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT SkinMesh::CreateAppMeshFromD3DXMesh(LPD3DXFRAME p)
{
	MYFRAME* pFrame = reinterpret_cast<MYFRAME*>(p);

	//	LPD3DXMESH pD3DXMesh = pFrame->pMeshContainer->MeshData.pMesh;//D3DXメッシュ(ここから・・・).
	MYMESHCONTAINER* pContainer = reinterpret_cast<MYMESHCONTAINER*>(pFrame->pMeshContainer);

	//アプリメッシュ(・・・ここにメッシュデータをコピーする).
	SKIN_PARTS_MESH* pAppMesh = new SKIN_PARTS_MESH();
	pAppMesh->EnableTexture = false;

	//事前に頂点数、ポリゴン数等を調べる.
	pAppMesh->NumVert = m_pD3dxMesh->GetNumVertices(pContainer);
	pAppMesh->NumFace = m_pD3dxMesh->GetNumFaces(pContainer);
	pAppMesh->NumUV = m_pD3dxMesh->GetNumUVs(pContainer);
	//Direct3DではUVの数だけ頂点が必要.
	if (pAppMesh->NumVert < pAppMesh->NumUV) {
		//共有頂点等で、頂点が足りないとき.
		_ASSERT_EXPR(false,
			L"Direct3Dは、UVの数だけ頂点が必要です(UVを置く場所が必要です)テクスチャは正しく貼られないと思われます");
		return E_FAIL;
	}
	//一時的なメモリ確保(頂点バッファとインデックスバッファ).
	auto pVB = std::make_unique<SKIN_VERTEX[]>(pAppMesh->NumVert);
	auto pFaceBuffer = std::make_unique<int[]>(pAppMesh->NumFace * 3);
	//3頂点ポリゴンなので、1フェイス=3頂点(3インデックス).

	//頂点読み込み. (SKIN_VERTEX::positionがD3DXVECTOR3互換と仮定)
	for (DWORD i = 0; i < pAppMesh->NumVert; i++) {
		pVB[i].Position = m_pD3dxMesh->GetVertexCoord(pContainer, i);
	}
	//ポリゴン情報(頂点インデックス)読み込み.
	for (DWORD i = 0; i < pAppMesh->NumFace * 3; i++) {
		pFaceBuffer[i] = m_pD3dxMesh->GetIndex(pContainer, i);
	}
	//法線読み込み.
	for (DWORD i = 0; i < pAppMesh->NumVert; i++) {
		pVB[i].Normal = m_pD3dxMesh->GetNormal(pContainer, i);
	}
	//テクスチャ座標読み込み.
	for (DWORD i = 0; i < pAppMesh->NumVert; i++) {
		pVB[i].Texture = m_pD3dxMesh->GetUV(pContainer, i);
	}

	//マテリアル読み込み.
	pAppMesh->NumMaterial = m_pD3dxMesh->GetNumMaterials(pContainer);
	pAppMesh->pMaterial = new MY_SKINMATERIAL[pAppMesh->NumMaterial]();

	//マテリアルの数だけインデックスバッファを作成.
	pAppMesh->ppIndexBuffer = new ID3D11Buffer * [pAppMesh->NumMaterial]();
	//掛け算ではなく「ID3D11Buffer*」の配列という意味.
	for (DWORD i = 0; i < pAppMesh->NumMaterial; i++)
	{
		//環境光(アンビエント).
		pAppMesh->pMaterial[i].Ambient = m_pD3dxMesh->GetAmbient(pContainer, i);
		//拡散反射光(ディフューズ).
		pAppMesh->pMaterial[i].Diffuse = m_pD3dxMesh->GetDiffuse(pContainer, i);
		//鏡面反射光(スペキュラ).
		pAppMesh->pMaterial[i].Specular = m_pD3dxMesh->GetSpecular(pContainer, i);
		//自己発光(エミッシブ).
		pAppMesh->pMaterial[i].Emissive = m_pD3dxMesh->GetEmissive(pContainer, i);
		//スペキュラパワー.
		pAppMesh->pMaterial[i].SpecularPower = m_pD3dxMesh->GetSpecularPower(pContainer, i);

		//アンビエントが0だと光源からの直射光が全く当たらない陰の部分の明るさが、
		// 真っ暗になるので、最低値と最高値を設定する.
		float low = 0.3f;	//適当に0.3くらいにしておく.
		float high = 1.0f;

		// Ambient.x/y/zはD3DXVECTOR4/D3DXCOLORのx/y/zを保持
		pAppMesh->pMaterial[i].Ambient.x = clamp(pAppMesh->pMaterial[i].Ambient.x, low, high);
		pAppMesh->pMaterial[i].Ambient.y = clamp(pAppMesh->pMaterial[i].Ambient.y, low, high);
		pAppMesh->pMaterial[i].Ambient.z = clamp(pAppMesh->pMaterial[i].Ambient.z, low, high);
		//なお、x,y,z(r,g,b)のみ対処して、w(a)はそのまま使用する.

#if 0
		//テクスチャ(ディフューズテクスチャのみ).
#ifdef UNICODE
		WCHAR TexFilename_w[32] = L"";
		//テクスチャ名のサイズを取得.
		size_t charSize = strlen(m_pD3dxMesh->GetTexturePath(pContainer, i)) + 1;
		size_t ret;	//変換された文字数.

		//マルチバイト文字のシーケンスを対応するワイド文字のシーケンスに変換します.
		errno_t err = mbstowcs_s(
			&ret,
			TexFilename_w,
			charSize,
			m_pD3dxMesh->GetTexturePath(pContainer, i),
			_TRUNCATE);

		LPTSTR name = TexFilename_w;
#else//#ifdef UNICODE
		LPTSTR name = d3dxMaterials[No].pTextureFilename;
#endif//#ifdef UNICODE
#endif

		//テクスチャ(ディフューズテクスチャのみ).
#ifdef UNICODE
		WCHAR TextureName_w[32] = L"";
		//文字変換.
		LPTSTR name = nullptr;
		LPSTR name_org = m_pD3dxMesh->GetTexturePath(pContainer, i);
		if (name_org != nullptr) {
			ConvertCharaMultiByteToUnicode(TextureName_w, 32, name_org);
			name = TextureName_w;
		}
#else//#ifdef UNICODE
		LPTSTR name = m_pD3dxMesh->GetTexturePath(pContainer, i);
#endif//#ifdef UNICODE

		if (name != nullptr) {
			LPTSTR ret = _tcsrchr(m_FilePath, _T('\\'));
			if (ret != nullptr) {
				int check = static_cast<int>(ret - m_FilePath);
				TCHAR path[512];
				lstrcpy(path, m_FilePath);
				path[check + 1] = '\0';

				//パスのコピー.
				lstrcpy(pAppMesh->pMaterial[i].TextureName, path);
				//テクスチャファイル名を連結.
				lstrcat(pAppMesh->pMaterial[i].TextureName, name);
			}
		}
		//テクスチャを作成.
		if (pAppMesh->pMaterial[i].TextureName[0] != 0
			&& FAILED(
			D3DX11CreateShaderResourceViewFromFile(
			m_pDevice11, pAppMesh->pMaterial[i].TextureName,
			nullptr, nullptr, &pAppMesh->pMaterial[i].pTexture, nullptr)))
		{
			_ASSERT_EXPR(false, L"テクスチャ作成失敗");
			return E_FAIL;
		}
		//そのマテリアルであるインデックス配列内の開始インデックスを調べる.
		//さらにインデックスの個数を調べる.
		int count = 0;
		auto pIndex = std::make_unique<int[]>(pAppMesh->NumFace * 3);
		//とりあえず、メッシュ内のポリゴン数でメモリ確保.
		//(ここのポリゴングループは必ずこれ以下になる).

		for (DWORD k = 0; k < pAppMesh->NumFace; k++)
		{
			//もし i==k 番目のポリゴンのマテリアル番号なら.
			if (i == m_pD3dxMesh->GeFaceMaterialIndex(pContainer, k))
			{
				//k番目のポリゴンを作る頂点のインデックス.
				pIndex[count]
					= m_pD3dxMesh->GetFaceVertexIndex(pContainer, k, 0);	//1個目.
					pIndex[count + 1]
						= m_pD3dxMesh->GetFaceVertexIndex(pContainer, k, 1);	//2個目.
					pIndex[count + 2]
						= m_pD3dxMesh->GetFaceVertexIndex(pContainer, k, 2);	//3個目.
					count += 3;
			}
		}
		if (count > 0) {
			//インデックスバッファ作成.
			CreateIndexBuffer(count * sizeof(int),
				pIndex.get(), &pAppMesh->ppIndexBuffer[i]);
		}
		else {
			//解放時の処理に不具合が出たため.
			//カウント数が0以下の場合は、インデックスバッファを nullptr にしておく.
			pAppMesh->ppIndexBuffer[i] = nullptr;
		}

		//そのマテリアル内のポリゴン数.
		pAppMesh->pMaterial[i].NumFace = count / 3;

	}

	//スキン情報ある？
	if (pContainer->pSkinInfo == nullptr) {
#ifdef _DEBUG
		//不明なスキンあればここで教える.不要ならコメントアウトしてください.
		//TCHAR strDbg[128];
		//WCHAR str[64] = L"";
		//ConvertCharaMultiByteToUnicode(str, 64, pContainer->Name);
		//_stprintf_s(strDbg, _T("ContainerName:[%s]"), str);
		//MessageBox(nullptr, strDbg, _T("Not SkinInfo"), MB_OK);
#endif//#ifdef _DEBUG
		pAppMesh->EnableBones = false;
	}
	else {
		//スキン情報(ジョイント、ウェイト)読み込み.
		ReadSkinInfo(pContainer, pVB.get(), pAppMesh);
	}

	//バーテックスバッファを作成.
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SKIN_VERTEX) * (pAppMesh->NumVert);
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = pVB.get();

	HRESULT hRslt = S_OK;
	if (FAILED(
		m_pDevice11->CreateBuffer(
		&bd, &InitData, &pAppMesh->pVertexBuffer)))
	{
		hRslt = E_FAIL;
	}

	//パーツメッシュに設定.
	pFrame->pPartsMesh = pAppMesh;

	return hRslt;
}


//ボーンを次のポーズ位置にセットする関数.
void SkinMesh::SetNewPoseMatrices(
	SKIN_PARTS_MESH* pParts, int Frame, MYMESHCONTAINER* pContainer)
{
	//望むフレームでUpdateすること.
	//しないと行列が更新されない.
	//m_pD3dxMesh->UpdateFrameMatrices(
	// m_pD3dxMesh->m_pFrameRoot)をレンダリング時に実行すること.

	//また、アニメーション時間に見合った行列を更新するのはD3DXMESHでは
	//アニメーションコントローラが(裏で)やってくれるものなので、
	//アニメーションコントローラを使ってアニメを進行させることも必要.
	//m_pD3dxMesh->m_pAnimController->AdvanceTime(...)を.
	//レンダリング時に実行すること.

	if (pParts->NumBone <= 0) {
		//ボーンが 0　以下の場合.
		return;
	}

	for (int i = 0; i < pParts->NumBone; i++)
	{
		pParts->pBoneArray[i].mNewPose = m_pD3dxMesh->GetNewPose(pContainer, i);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------

// 【修正】戻り値を D3DXMATRIX から XMMATRIX に変更
DirectX::XMMATRIX SkinMesh::GetCurrentPoseMatrix(SKIN_PARTS_MESH* pParts, int Index)
{
	// mBindPose, mNewPose は XMMATRIX と仮定して XM 行列演算を使用
	DirectX::XMMATRIX ret =
		pParts->pBoneArray[Index].mBindPose * pParts->pBoneArray[Index].mNewPose;
	return ret;
}

//-------------------------------------------------------------------------------------------------------------------------------------
// SkinMesh.cpp の DrawFrame 関数の新しい実装

void SkinMesh::DrawFrame(LPD3DXFRAME p)
{
	if (p == nullptr) { return; }

	MYFRAME* pFrame = reinterpret_cast<MYFRAME*>(p);
	SKIN_PARTS_MESH* pPartsMesh = pFrame->pPartsMesh;
	MYMESHCONTAINER* pContainer = reinterpret_cast<MYMESHCONTAINER*>(pFrame->pMeshContainer);

	if (pPartsMesh != nullptr)
	{
		DirectX::XMMATRIX World = D3DXMatrixToXMMatrix(pFrame->CombinedTransformationMatrix);

		DrawPartsMesh(
			pPartsMesh,
			World,
			pContainer);
	}

	//再帰関数.
	//(兄弟)
	if (pFrame->pFrameSibling != nullptr)
	{
		DrawFrame(pFrame->pFrameSibling);
	}
	//(親子)
	if (pFrame->pFrameFirstChild != nullptr)
	{
		DrawFrame(pFrame->pFrameFirstChild);
	}
}

void SkinMesh::DrawFrameDepth(LPD3DXFRAME pFrame)
{
	if (pFrame == nullptr) { return; }

	MYFRAME* pMyFrame = reinterpret_cast<MYFRAME*>(pFrame);

	DirectX::XMMATRIX FrameLocal;
	FrameLocal = D3DXMatrixToXMMatrix(pMyFrame->TransformationMatrix);

	DirectX::XMMATRIX CombinedWorld = FrameLocal;

	if (pMyFrame && pMyFrame->pMeshContainer)
	{
		MYMESHCONTAINER* pContainer = reinterpret_cast<MYMESHCONTAINER*>(pMyFrame->pMeshContainer);
		DrawPartsMeshDepth(pMyFrame->pPartsMesh, CombinedWorld, pContainer);
	}

	// (子)
	if (pFrame->pFrameFirstChild != nullptr)
	{
		DrawFrameDepth(pFrame->pFrameFirstChild);
	}

	// (兄弟)
	if (pFrame->pFrameSibling != nullptr)
	{
		DrawFrameDepth(pFrame->pFrameSibling);
	}
}
#define SEND_CB_TO_FUNC
//-------------------------------------------------------------------------------------------------------------------------------------

// 【修正】引数を D3DXMATRIX から XMMATRIX に変更
void SkinMesh::DrawPartsMesh(
	SKIN_PARTS_MESH* pMesh, DirectX::XMMATRIX World, MYMESHCONTAINER* pContainer)
{
	//ワールド行列算出.
	CalcWorldMatrix();

	//アニメーションフレームを進める スキンを更新.
	m_Frame++;
	if (m_Frame >= 3600) {
		m_Frame = 0;
	}
	SetNewPoseMatrices(pMesh, m_Frame, pContainer);

	//コンスタントバッファに情報を送る(ボーン).
	SendCBufferPerBone(pMesh);

	//コンスタントバッファに情報を設定(フレームごと).
	SendCBufferPerFrame();

	//コンスタントバッファに情報を設定(メッシュごと).
	SendCBufferPerMesh();

	//バーテックスバッファをセット.
	UINT stride = sizeof(SKIN_VERTEX);
	UINT offset = 0;
	m_pContext11->IASetVertexBuffers(
		0, 1, &pMesh->pVertexBuffer, &stride, &offset);

	//頂点インプットレイアウトをセット.
	m_pContext11->IASetInputLayout(m_pVertexShader->GetInputLayout());

	//プリミティブ・トポロジーをセット.
	m_pContext11->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//マテリアルの数だけ、
	//それぞれのマテリアルのインデックスバッファを描画.
	for (DWORD i = 0; i < pMesh->NumMaterial; i++)
	{
		//使用されていないマテリアル対策.
		if (pMesh->pMaterial[i].NumFace == 0)
		{
			continue;
		}

		//インデックスバッファをセット.
		stride = sizeof(int);
		offset = 0;
		m_pContext11->IASetIndexBuffer(
			pMesh->ppIndexBuffer[i],
			DXGI_FORMAT_R32_UINT, 0);

		//コンスタントバッファに情報を設定（マテリアルごと）
		SendCBufferPerMaterial(&pMesh->pMaterial[i]);

		//テクスチャをシェーダに渡す.
		SendTexture(&pMesh->pMaterial[i]);

		//描画.
		m_pContext11->DrawIndexed(pMesh->pMaterial[i].NumFace * 3, 0, 0);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------

// 【修正】引数を D3DXMATRIX から XMMATRIX に変更
void SkinMesh::DrawPartsMeshDepth(SKIN_PARTS_MESH* pMesh, DirectX::XMMATRIX World, MYMESHCONTAINER* pContainer)
{
	//ワールド行列算出.
	CalcWorldMatrix();

	//アニメーションフレームを進める スキンを更新.
	m_Frame++;
	if (m_Frame >= 3600) {
		m_Frame = 0;
	}
	SetNewPoseMatrices(pMesh, m_Frame, pContainer);

	//コンスタントバッファに情報を送る(ボーン).
	SendCBufferPerBone(pMesh);

	SendCBufferPerMeshDepth();

	//バーテックスバッファをセット.
	UINT stride = sizeof(SKIN_VERTEX);
	UINT offset = 0;
	m_pContext11->IASetVertexBuffers(
		0, 1, &pMesh->pVertexBuffer, &stride, &offset);

	//頂点インプットレイアウトをセット.
	ShaderResource::GetInstance().GetVSShadowMapSkinMesh()->SetInputLayout();

	//プリミティブ・トポロジーをセット.
	m_pContext11->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//マテリアルの数だけ、
	//それぞれのマテリアルのインデックスバッファを描画.
	for (DWORD i = 0; i < pMesh->NumMaterial; i++)
	{
		//使用されていないマテリアル対策.
		if (pMesh->pMaterial[i].NumFace == 0)
		{
			continue;
		}

		//インデックスバッファをセット.
		stride = sizeof(int);
		offset = 0;
		m_pContext11->IASetIndexBuffer(
			pMesh->ppIndexBuffer[i],
			DXGI_FORMAT_R32_UINT, 0);

		//コンスタントバッファに情報を設定（マテリアルごと）
		SendCBufferPerMaterial(&pMesh->pMaterial[i]);

		//テクスチャをシェーダに渡す.
		SendTexture(&pMesh->pMaterial[i]);

		//描画.
		m_pContext11->DrawIndexed(pMesh->pMaterial[i].NumFace * 3, 0, 0);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------

void SkinMesh::SendCBufferPerBone(SKIN_PARTS_MESH* pMesh)
{
	D3D11_MAPPED_SUBRESOURCE pData;

	if (SUCCEEDED(
		m_pContext11->Map(
		m_pCBufferPerBone, 0,
		D3D11_MAP_WRITE_DISCARD, 0, &pData)))
	{
		auto cb_ptr = std::make_unique<CBUFFER_PER_BONES>();
		CBUFFER_PER_BONES* cb = cb_ptr.get(); 

		for (int i = 0; i < pMesh->NumBone; i++)
		{
			DirectX::XMMATRIX mat = GetCurrentPoseMatrix(pMesh, i);
			mat = DirectX::XMMatrixTranspose(mat);

			// ポインタ経由でアクセス
			cb->mBone[i] = mat;
		}

		// ヒープ上のデータをコピー
		memcpy_s(pData.pData, pData.RowPitch,
			reinterpret_cast<void*>(cb), sizeof(CBUFFER_PER_BONES));

		m_pContext11->Unmap(m_pCBufferPerBone, 0);
	}
	m_pContext11->VSSetConstantBuffers(enCBSlot::Bones, 1, &m_pCBufferPerBone);
	m_pContext11->PSSetConstantBuffers(enCBSlot::Bones, 1, &m_pCBufferPerBone);
}

//-------------------------------------------------------------------------------------------------------------------------------------

void SkinMesh::SendCBufferPerFrame()
{
	D3D11_MAPPED_SUBRESOURCE pData;

	if (SUCCEEDED(
		m_pContext11->Map(
		m_pCBufferPerFrame,
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&pData)))
	{
		CBUFFER_PER_FRAME cb;

		//カメラ位置.
		// 【修正】m_CamPos (XMFLOAT3) から XMVECTOR をロード
		DirectX::XMVECTOR camPos = DirectX::XMLoadFloat3(&m_CamPos);

		// 【修正】XMVECTOR を XMFLOAT4 にストア (CBUFFER_PER_FRAME::CameraPosがXMFLOAT4と仮定)
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&cb.CameraPos), camPos);

		//ライト方向.
		// LightManager::GetDirectionLight()->GetDirection() は D3DXVECTOR3 を返すと仮定
		DirectX::XMFLOAT3 d3dxLightDir = LightManager::GetDirectionLight()->GetDirection();

		// 【修正】D3DXVECTOR3 を XMVECTOR に変換し、正規化
		DirectX::XMVECTOR lightDir = DirectX::XMVectorSet(d3dxLightDir.x, d3dxLightDir.y, d3dxLightDir.z, 0.0f);
		lightDir = DirectX::XMVector3Normalize(lightDir);

		// 【修正】結果を cb.vLightDir (XMFLOAT4と仮定) にストア
		DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&cb.vLightDir), lightDir);

		// ライトを使用するか.
		cb.isLight.x = static_cast<float>(m_IsLight);

		// 影を使用するか.
		cb.isShadow.x = static_cast<float>(m_IsShadow);

		memcpy_s(pData.pData, pData.RowPitch,
			reinterpret_cast<void*>(&cb), sizeof(cb));

		m_pContext11->Unmap(m_pCBufferPerFrame, 0);
	}
	m_pContext11->VSSetConstantBuffers(enCBSlot::Frame, 1, &m_pCBufferPerFrame);
	m_pContext11->PSSetConstantBuffers(enCBSlot::Frame, 1, &m_pCBufferPerFrame);
}

//-------------------------------------------------------------------------------------------------------------------------------------

void SkinMesh::SendCBufferPerMesh()
{
	D3D11_MAPPED_SUBRESOURCE pDat;

	if (SUCCEEDED(
		m_pContext11->Map(
		m_pCBufferPerMesh, 0,
		D3D11_MAP_WRITE_DISCARD, 0, &pDat)))
	{
		CBUFFER_PER_MESH cb;
		cb.mW = m_WorldMatrix; 
		cb.mW = DirectX::XMMatrixTranspose(cb.mW);
			DirectX::XMMATRIX aa =  m_mView * m_mProj;
		
			cb.mWVP = m_WorldMatrix * aa;

			cb.mWVP = DirectX::XMMatrixTranspose(cb.mWVP);	//行列を転置する.
		//※行列の計算方法がDirectXとGPUで異なるため転置が必要.

		DirectX::XMMATRIX lightView = LightManager::GetDirectionLight()->GetViewMatrix();
		DirectX::XMMATRIX lightProj = LightManager::GetDirectionLight()->GetProjectionMatrix();
		cb.mWLVP = m_WorldMatrix * lightView * lightProj;
		cb.mWLVP = DirectX::XMMatrixTranspose(cb.mWLVP);


		memcpy_s(pDat.pData, pDat.RowPitch,
			reinterpret_cast<void*>(&cb), sizeof(cb));
		m_pContext11->Unmap(m_pCBufferPerMesh, 0);
	}

	m_pContext11->VSSetConstantBuffers(enCBSlot::Mesh, 1, &m_pCBufferPerMesh);
	m_pContext11->PSSetConstantBuffers(enCBSlot::Mesh, 1, &m_pCBufferPerMesh);
}

//-------------------------------------------------------------------------------------------------------------------------------------

// 【修正】引数に XMMATRIX World を追加
void SkinMesh::SendCBufferPerMeshDepth()
{
	ID3D11Buffer* CBuffer = Shadow::GetInstance().GetCBuffer();

	D3D11_MAPPED_SUBRESOURCE pDat;

	if (SUCCEEDED(
		m_pContext11->Map(
		CBuffer, 0,
		D3D11_MAP_WRITE_DISCARD, 0, &pDat)))
	{
		Shadow::CBUFFER cb;

		// LightManagerの行列を取得（D3DXMATRIXを返すため変換が必要）
		DirectX::XMMATRIX lightView = LightManager::GetDirectionLight()->GetViewMatrix();
		DirectX::XMMATRIX lightProj = LightManager::GetDirectionLight()->GetProjectionMatrix();

		// 【修正】mWLPをXMで計算し、転置
		cb.mWLP = m_WorldMatrix * lightView * lightProj;
		cb.mWLP = DirectX::XMMatrixTranspose(cb.mWLP);	//行列を転置する.
		//※行列の計算方法がDirectXとGPUで異なるため転置が必要.

		memcpy_s(pDat.pData, pDat.RowPitch,
			reinterpret_cast<void*>(&cb), sizeof(cb));
		m_pContext11->Unmap(CBuffer, 0);
	}

	m_pContext11->VSSetConstantBuffers(enCBSlot::Mesh, 1, &CBuffer);
	m_pContext11->PSSetConstantBuffers(enCBSlot::Mesh, 1, &CBuffer);

}

//-------------------------------------------------------------------------------------------------------------------------------------

void SkinMesh::SendCBufferPerMaterial(MY_SKINMATERIAL* pMaterial)
{
	D3D11_MAPPED_SUBRESOURCE pDat;

	if (SUCCEEDED(
		m_pContext11->Map(
		m_pCBufferPerMaterial, 0,
		D3D11_MAP_WRITE_DISCARD, 0, &pDat)))
	{
		CBUFFER_PER_MATERIAL cb;

		// pMaterial->AmbientなどはXMFLOAT4型に変換済みと仮定 (ヘッダーファイル変換済み)
		cb.Ambient = pMaterial->Ambient;
		cb.Diffuse = pMaterial->Diffuse;
		cb.Specular = pMaterial->Specular;
		// Emissive のアルファにグローバルアルファを乗算
		cb.Emissive = pMaterial->Emissive;
		cb.Diffuse.w *= m_GlobalAlpha;
		cb.Ambient.w *= m_GlobalAlpha;
		cb.Specular.w *= m_GlobalAlpha;

		// ディゾルブパラメータ設定
		cb.Dissolve.x = m_DissolveThreshold;    // 閾値
		cb.Dissolve.y = m_DissolveEdgeWidth;    // エッジ幅
		cb.Dissolve.z = m_DissolveEnabled ? 1.0f : 0.0f; // 有効フラグ
		cb.Dissolve.w = 0.0f;                   // 予約

		memcpy_s(pDat.pData, pDat.RowPitch,
			reinterpret_cast<void*>(&cb), sizeof(cb));

		m_pContext11->Unmap(m_pCBufferPerMaterial, 0);
	}

	m_pContext11->VSSetConstantBuffers(enCBSlot::Material, 1, &m_pCBufferPerMaterial);
	m_pContext11->PSSetConstantBuffers(enCBSlot::Material, 1, &m_pCBufferPerMaterial);
}

//-------------------------------------------------------------------------------------------------------------------------------------

void SkinMesh::SendTexture(MY_SKINMATERIAL* pMaterial)
{
	if (pMaterial->TextureName[0] != 0 && pMaterial->pTexture != nullptr) // TextNameが空でないか、pTextureが設定されているか
	{
		m_pContext11->PSSetSamplers(0, 1, &m_pSampleLinear);
		m_pContext11->PSSetShaderResources(0, 1, &pMaterial->pTexture);
	}
	else
	{
		ID3D11ShaderResourceView* Nothing[1] = { 0 };
		m_pContext11->PSSetShaderResources(0, 1, Nothing);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT SkinMesh::Release()
{
	if (m_pD3dxMesh != nullptr) {
		//全てのメッシュ削除.
		DestroyAllMesh(m_pD3dxMesh->m_pFrameRoot);

		//パーサークラス解放処理.
		m_pD3dxMesh->Release();
		SAFE_DELETE(m_pD3dxMesh);
	}

	return S_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------

void SkinMesh::DestroyAllMesh(D3DXFRAME* pFrame)
{
	if (pFrame == nullptr) { return; }

	if ((pFrame != nullptr) && (pFrame->pMeshContainer != nullptr))
	{
		DestroyAppMeshFromD3DXMesh(pFrame);
	}

	//再帰関数.
	if (pFrame->pFrameSibling != nullptr)
	{
		DestroyAllMesh(pFrame->pFrameSibling);
	}
	if (pFrame->pFrameFirstChild != nullptr)
	{
		DestroyAllMesh(pFrame->pFrameFirstChild);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT SkinMesh::DestroyAppMeshFromD3DXMesh(LPD3DXFRAME p)
{
	MYFRAME* pFrame = reinterpret_cast<MYFRAME*>(p);

	MYMESHCONTAINER* pMeshContainerTmp = reinterpret_cast<MYMESHCONTAINER*>(pFrame->pMeshContainer);

	//SKIN_PARTS_MESHの解放.
	if (pFrame->pPartsMesh != nullptr)
	{
		//ボーン情報の解放.
		if (pFrame->pPartsMesh->pBoneArray != nullptr)
		{
			delete[] pFrame->pPartsMesh->pBoneArray;
			pFrame->pPartsMesh->pBoneArray = nullptr;
		}

		if (pFrame->pPartsMesh->pMaterial != nullptr)
		{
			int iMax = static_cast<int>(pFrame->pPartsMesh->NumMaterial);

			for (int i = iMax - 1; i >= 0; i--)
			{
				if (pFrame->pPartsMesh->pMaterial[i].pTexture != nullptr)
				{
					pFrame->pPartsMesh->pMaterial[i].pTexture->Release();
					pFrame->pPartsMesh->pMaterial[i].pTexture = nullptr;
				}
			}

			delete[] pFrame->pPartsMesh->pMaterial;
			pFrame->pPartsMesh->pMaterial = nullptr;
		}

		// インデックスバッファの解放
		if (pFrame->pPartsMesh->ppIndexBuffer != nullptr)
		{
			int iMax = static_cast<int>(pFrame->pPartsMesh->NumMaterial);

			for (int i = iMax - 1; i >= 0; i--)
			{
				SAFE_RELEASE(pFrame->pPartsMesh->ppIndexBuffer[i]);
			}
			delete[] pFrame->pPartsMesh->ppIndexBuffer;
			pFrame->pPartsMesh->ppIndexBuffer = nullptr;
		}

		// 頂点バッファの解放
		SAFE_RELEASE(pFrame->pPartsMesh->pVertexBuffer);

		// SKIN_PARTS_MESH 本体を解放
		delete pFrame->pPartsMesh;
	}
	pFrame->pPartsMesh = nullptr;

	//SKIN_PARTS_MESH解放完了.

	//MYFRAMEのSKIN_PARTS_MESH以外のメンバーポインター変数は別の関数で解放されている.

	return S_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------

void SkinMesh::ChangeAnimSet(int index, LPD3DXANIMATIONCONTROLLER pAC)
{
	if (m_pD3dxMesh == nullptr) {
		return;
	}
	m_pD3dxMesh->ChangeAnimSet(index, pAC);
}

//-------------------------------------------------------------------------------------------------------------------------------------

void SkinMesh::ChangeAnimSet_StartPos(int index, double dStartFramePos, LPD3DXANIMATIONCONTROLLER pAC)
{
	if (m_pD3dxMesh == nullptr) {
		return;
	}
	m_pD3dxMesh->ChangeAnimSet_StartPos(index, dStartFramePos, pAC);
}

//-------------------------------------------------------------------------------------------------------------------------------------

double SkinMesh::GetAnimPeriod(int index)
{
	if (m_pD3dxMesh == nullptr) {
		return 0.0f;
	}
	return m_pD3dxMesh->GetAnimPeriod(index);
}

//-------------------------------------------------------------------------------------------------------------------------------------

int SkinMesh::GetAnimMax(LPD3DXANIMATIONCONTROLLER pAC)
{
	if (m_pD3dxMesh != nullptr) {
		return m_pD3dxMesh->GetAnimMax(pAC);
	}
	return -1;
}

//-------------------------------------------------------------------------------------------------------------------------------------

// 【修正】pOutMatの型をD3DXMATRIX*からDirectX::XMMATRIX*に変更
bool SkinMesh::GetMatrixFromBone(
	LPCSTR sBoneName, DirectX::XMMATRIX* pOutMat)
{
	if (m_pD3dxMesh != nullptr) {
		if (m_pD3dxMesh->GetMatrixFromBone(sBoneName, pOutMat)) {
			return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------------------------------------------

// 【修正】pOutPosの型をD3DXVECTOR3*からDirectX::XMFLOAT3*に変更
bool SkinMesh::GetPosFromBone(
	LPCSTR sBoneName, DirectX::XMFLOAT3* pOutPos)
{
	if (m_pD3dxMesh != nullptr)
	{
		if (m_pD3dxMesh->GetPosFromBone(sBoneName, pOutPos)) {

			// 【修正】ボーン位置をXMVECTORにロード
			DirectX::XMVECTOR tmpPosVector = DirectX::XMVectorSet(pOutPos->x, pOutPos->y, pOutPos->z, 1.0f);

			// 【修正】モデルのトランスフォームをXMで構築
			DirectX::XMMATRIX mScale, mRot;

			// m_spTransformはXMFLOAT3/4と仮定してロード
			DirectX::XMVECTOR scale = DirectX::XMLoadFloat3(&m_spTransform.Scale);
			DirectX::XMVECTOR rotation = DirectX::XMLoadFloat3(&m_spTransform.Rotation);
			DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&m_spTransform.Position);
			
			// XM 行列の作成
			mScale = DirectX::XMMatrixScalingFromVector(scale);
			mRot = DirectX::XMMatrixRotationRollPitchYawFromVector(rotation);
			DirectX::XMMATRIX mTran = DirectX::XMMatrixTranslationFromVector(tmpPosVector);

			// 【修正】ワールド行列の合成 (Translation * Scale * Rotation)
			// 注意: ボーンの位置は既にボーン階層変換後のワールド行列の平行移動成分であるため、
			// mTranではなく、ボーン行列全体 (GetMatrixFromBone) を使用するか、
			// ここでは単にボーンの位置に対してモデルのスケールと回転を適用し、最後にモデルの平行移動を加算する。
			// オリジナルのコードの意図通り、ボーン位置のみを抽出し、それに対してスケールと回転を適用しているものとする。

			// ボーン位置を原点とし、そこからモデルのスケールと回転を適用
			DirectX::XMVECTOR transformedPos = DirectX::XMVector3TransformCoord(tmpPosVector, mScale * mRot);

			// モデルの全体位置を加算
			DirectX::XMVECTOR finalPos = DirectX::XMVectorAdd(transformedPos, position);

			// 【修正】結果を pOutPos (XMFLOAT3) に格納
			DirectX::XMStoreFloat3(pOutPos, finalPos);

			return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------------------------------------------

// 【修正】pOutPos, SpecifiedPos の型を D3DXVECTOR3/D3DXVECTOR3* から XMFLOAT3/XMFLOAT3* に変更
bool SkinMesh::GetDeviaPosFromBone(
	LPCSTR sBoneName, DirectX::XMFLOAT3* pOutPos, DirectX::XMFLOAT3 SpecifiedPos)
{
	if (m_pD3dxMesh != nullptr)
	{
		DirectX::XMMATRIX  d3dxBoneMat;
		// D3DXPARSER::GetMatrixFromBone は D3DXMATRIX を返す
		if (m_pD3dxMesh->GetMatrixFromBone(sBoneName, &d3dxBoneMat)) {

			// 【修正】D3DXMATRIX を XMMATRIX に変換
			DirectX::XMMATRIX mBone = d3dxBoneMat;

			// 【修正】SpecifiedPos (ずれ)を行列化 (XMMATRIX)
			DirectX::XMVECTOR vDevia = DirectX::XMLoadFloat3(&SpecifiedPos);
			DirectX::XMMATRIX mDevia = DirectX::XMMatrixTranslationFromVector(vDevia);

			// 【修正】ずれ * ボーン行列を XM で計算
			DirectX::XMMATRIX mtmp = mDevia * mBone;

			// 【修正】mtmp の平行移動成分を取得 (ボーンの最終位置)
			DirectX::XMVECTOR tmpPosVector = mtmp.r[3];

			// 【修正】ここから GetPosFromBone と同様のモデル全体トランスフォームを適用

			DirectX::XMVECTOR scale = DirectX::XMLoadFloat3(&m_spTransform.Scale);
			DirectX::XMVECTOR rotation = DirectX::XMLoadFloat3(&m_spTransform.Rotation);
			DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&m_spTransform.Position);

			DirectX::XMMATRIX mScale = DirectX::XMMatrixScalingFromVector(scale);
			DirectX::XMMATRIX mRot = DirectX::XMMatrixRotationRollPitchYawFromVector(rotation);

			// ボーン位置を原点とし、そこからモデルのスケールと回転を適用
			DirectX::XMVECTOR transformedPos = DirectX::XMVector3TransformCoord(tmpPosVector, mScale * mRot);

			// モデルの全体位置を加算
			DirectX::XMVECTOR finalPos = DirectX::XMVectorAdd(transformedPos, position);

			// 【修正】結果を pOutPos (XMFLOAT3) に格納
			DirectX::XMStoreFloat3(pOutPos, finalPos);

			return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT SkinMesh::CreateCBuffer(
	ID3D11Buffer** pConstantBuffer,
	UINT Size)
{
	D3D11_BUFFER_DESC cb;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.ByteWidth = Size;
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	cb.Usage = D3D11_USAGE_DYNAMIC;

	if (FAILED(
		m_pDevice11->CreateBuffer(&cb, nullptr, pConstantBuffer)))
	{
		return E_FAIL;
	}
	return S_OK;
}

//-------------------------------------------------------------------------------------------------------------------------------------

HRESULT SkinMesh::CreateSampler(ID3D11SamplerState** pSampler)
{
	//テクスチャー用サンプラー作成.
	D3D11_SAMPLER_DESC SamDesc;
	ZeroMemory(&SamDesc, sizeof(D3D11_SAMPLER_DESC));

	SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	if (FAILED(
		m_pDevice11->CreateSamplerState(&SamDesc, &m_pSampleLinear)))
	{
		return E_FAIL;
	}
	return S_OK;
}

//マルチバイト文字をUnicode文字に変換する.
void SkinMesh::ConvertCharaMultiByteToUnicode(
	WCHAR* Dest,			//(out)変換後の文字列(Unicode文字列).
	size_t DestArraySize,	//変換後の文字列の配列の要素最大数.
	const CHAR* str)		//(in)変換前の文字列(マルチバイト文字列).
{
	//テクスチャ名のサイズを取得.
	size_t charSize = strlen(str) + 1;
	size_t ret;	//変換された文字数.

	//マルチバイト文字のシーケンスを対応するワイド文字のシーケンスに変換します.
	errno_t err = mbstowcs_s(
		&ret,
		Dest,
		charSize,
		str,
		_TRUNCATE);
}

