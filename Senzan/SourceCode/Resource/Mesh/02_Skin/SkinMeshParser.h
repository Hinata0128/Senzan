/***************************************************************************************************
*	SkinMeshCode Version 2.40
*	LastUpdate	: 2024/06/14.
**/
#pragma once

//警告についてのコード分析を無効にする。4005：再定義.
#pragma warning( disable : 4005 )

#include <D3D9.h>
#include <D3D11.h>
#include <tchar.h>
#include <string>
#include <vector>

//---------------------------------------------------------------------------------
// D3DX 構造体（D3DXPARSER 内部での D3DX 依存型）
// D3DXPARSER は D3DXFRAME/D3DXMESHCONTAINER に依存するため、その定義は D3DX のものを使用
typedef struct _MYFRAME : public D3DXFRAME
{
	// D3DXPARSER の UpdateFrameMatrices が D3DXMATRIX を使用するため、このメンバは D3DXMATRIX のまま残す
	D3DXMATRIX CombinedTransformationMatrix;
	struct SKIN_PARTS_MESH* pPartsMesh;
} MYFRAME, * LPMYFRAME;

typedef struct _MYMESHCONTAINER : public D3DXMESHCONTAINER
{
	LPDIRECT3DTEXTURE9* ppTextures;
	DWORD				Weight;				//重みの個数（重みとは頂点への影響。）.
	DWORD				BoneNum;			//ボーンの数.
	LPD3DXBUFFER		pBoneBuffer;		//ボーン・テーブル.
	DirectX::XMMATRIX** ppBoneMatrix;		//全てのボーンのワールド行列の先頭ポインタ.
	DirectX::XMMATRIX* pBoneOffsetMatrices;//フレームとしてのボーンのワールド行列のポインタ.

	_MYMESHCONTAINER()
		: D3DXMESHCONTAINER()
		, ppTextures(nullptr)
		, Weight()
		, BoneNum()
		, pBoneBuffer(nullptr)
		, ppBoneMatrix(nullptr)
		, pBoneOffsetMatrices(nullptr)
	{
	}
} MYMESHCONTAINER, * LPMYMESHCONTAINER;


//==================================================================================================
//
//	カスタム構造体定義
//
//==================================================================================================

//オリジナル　マテリアル構造体.
struct MY_SKINMATERIAL
{
	TCHAR Name[110];
	DirectX::XMFLOAT4	Ambient;			//アンビエント.
	DirectX::XMFLOAT4	Diffuse;			//ディフューズ.
	DirectX::XMFLOAT4	Specular;			//スペキュラ.
	DirectX::XMFLOAT4	Emissive;			//エミッシブ.
	float		SpecularPower;		//スペキュラパワー.
	TCHAR		TextureName[512];	//テクスチャーファイル名.
	ID3D11ShaderResourceView* pTexture;
	DWORD		NumFace;	//そのマテリアルであるポリゴン数.

	MY_SKINMATERIAL()
		: Name()
		, Ambient()
		, Diffuse()
		, Specular()
		, Emissive()
		, SpecularPower()
		, TextureName()
		, pTexture(nullptr)
		, NumFace()
	{
	}
	~MY_SKINMATERIAL()
	{
		// SAFE_RELEASE は D3D11 のマクロと仮定
		// pTexture は D3D11 の SRV のため、変更なし
		// SAFE_RELEASE(pTexture); // ヘッダーでは宣言のみ
	}
};

//ボーン構造体.
struct BONE
{
	DirectX::XMMATRIX	mBindPose;		//初期ポーズ（ずっと変わらない）.
	DirectX::XMMATRIX	mNewPose;		//現在のポーズ（その都度変わる）.
	DWORD		NumChild;		//子の数.
	int			ChildIndex[50];	//自分の子の"インデックス"50個まで.
	char		Name[256];

	BONE()
		: mBindPose(DirectX::XMMatrixIdentity())
		, mNewPose(DirectX::XMMatrixIdentity())
		, NumChild()
		, ChildIndex()
		, Name()
	{
	}
};

//パーツメッシュ構造体.
struct SKIN_PARTS_MESH
{
	DWORD				NumVert;
	DWORD				NumFace;
	DWORD				NumUV;
	DWORD				NumMaterial;
	MY_SKINMATERIAL* pMaterial;
	char				TextureFileName[8][256];	//テクスチャーファイル名(8枚まで).
	bool				EnableTexture;

	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer** ppIndexBuffer;

	//ボーン.
	int		NumBone;
	BONE* pBoneArray;

	bool	EnableBones;	//ボーンの有無フラグ.

	SKIN_PARTS_MESH()
		: NumVert()
		, NumFace()
		, NumUV()
		, NumMaterial()
		, pMaterial(nullptr)
		, TextureFileName()
		, EnableTexture()
		, pVertexBuffer(nullptr)
		, ppIndexBuffer(nullptr)
		, NumBone()
		, pBoneArray(nullptr)
		, EnableBones()
	{
	}
};

//==================================================================================================
//
//	ID3DXAllocateHierarchy 派生クラス.
//
//==================================================================================================

//Xファイル内のアニメーション階層を読み下してくれるクラスを派生させる.
//	ID3DXAllocateHierarchyは派生すること想定して設計されている.
class MY_HIERARCHY final
	: public ID3DXAllocateHierarchy
{
public:
	MY_HIERARCHY()
		: ID3DXAllocateHierarchy()
	{
	}
	STDMETHOD(CreateFrame)(THIS_ LPCSTR, LPD3DXFRAME*);
	STDMETHOD(CreateMeshContainer)(THIS_ LPCSTR, CONST D3DXMESHDATA*, CONST D3DXMATERIAL*,
		CONST D3DXEFFECTINSTANCE*, DWORD, CONST DWORD*, LPD3DXSKININFO, LPD3DXMESHCONTAINER*);
	STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME);
	STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER);
};

//==================================================================================================
//
//	パーサークラス.
//
//==================================================================================================
class D3DXPARSER final
{
public:
	//最大ボーン数.
	static constexpr int MAX_BONES = 255;
	//最大アニメーションセット数.
	static constexpr int MAX_ANIM_SET = 100;

public:
	D3DXPARSER();
	~D3DXPARSER();

	HRESULT LoadMeshFromX(LPDIRECT3DDEVICE9, LPCTSTR fileName);
	HRESULT AllocateBoneMatrix(LPD3DXMESHCONTAINER);
	HRESULT AllocateAllBoneMatrices(LPD3DXFRAME);
	// LPD3DXMATRIX は D3DXPARSER 内部で必要
	VOID UpdateFrameMatrices(LPD3DXFRAME, LPD3DXMATRIX);

	//--------------------------------------.
	//	取得関数. (戻り値を XM 互換型に修正)
	//--------------------------------------.
	int GetNumVertices(MYMESHCONTAINER* pContainer);
	int GetNumFaces(MYMESHCONTAINER* pContainer);
	int GetNumMaterials(MYMESHCONTAINER* pContainer);
	int GetNumUVs(MYMESHCONTAINER* pContainer);
	int GetNumBones(MYMESHCONTAINER* pContainer);
	int GetNumBoneVertices(MYMESHCONTAINER* pContainer, int BoneIndex);
	DWORD GetBoneVerticesIndices(MYMESHCONTAINER* pContainer, int BoneIndex, int IndexInGroup);
	double GetBoneVerticesWeights(MYMESHCONTAINER* pContainer, int BoneIndex, int IndexInGroup);
	DirectX::XMFLOAT3 GetVertexCoord(MYMESHCONTAINER* pContainer, DWORD Index);
	DirectX::XMFLOAT3 GetNormal(MYMESHCONTAINER* pContainer, DWORD Index);
	DirectX::XMFLOAT2 GetUV(MYMESHCONTAINER* pContainer, DWORD Index);
	int GetIndex(MYMESHCONTAINER* pContainer, DWORD Index);
	DirectX::XMFLOAT4 GetAmbient(MYMESHCONTAINER* pContainer, int Index);
	DirectX::XMFLOAT4 GetDiffuse(MYMESHCONTAINER* pContainer, int Index);
	DirectX::XMFLOAT4 GetSpecular(MYMESHCONTAINER* pContainer, int Index);
	DirectX::XMFLOAT4 GetEmissive(MYMESHCONTAINER* pContainer, int Index);
	LPSTR GetTexturePath(MYMESHCONTAINER* pContainer, int Index);
	float GetSpecularPower(MYMESHCONTAINER* pContainer, int Index);
	int GeFaceMaterialIndex(MYMESHCONTAINER* pContainer, int FaceIndex);
	int GetFaceVertexIndex(MYMESHCONTAINER* pContainer, int FaceIndex, int IndexInFace);
	DirectX::XMMATRIX GetBindPose(MYMESHCONTAINER* pContainer, int BoneIndex);
	DirectX::XMMATRIX GetNewPose(MYMESHCONTAINER* pContainer, int BoneIndex);
	LPCSTR GetBoneName(MYMESHCONTAINER* pContainer, int BoneIndex);

	//メッシュコンテナを取得する.
	LPD3DXMESHCONTAINER GetMeshContainer(LPD3DXFRAME pFrame);

	//アニメーションセットの切り替え. (LPD3DXANIMATIONCONTROLLER は D3DX 依存のまま)
	void ChangeAnimSet(int Index, LPD3DXANIMATIONCONTROLLER pAC);
	//アニメーションセットの切り替え(開始フレーム指定可能版).
	void ChangeAnimSet_StartPos(int Index, double StartFramePos, LPD3DXANIMATIONCONTROLLER pAC);

	//アニメーション停止時間を取得.
	double GetAnimPeriod(int Index);
	//アニメーション数を取得.
	int GetAnimMax(LPD3DXANIMATIONCONTROLLER pAC);

	//指定したボーン情報(座標・行列)を取得する関数.
	bool GetMatrixFromBone(LPCSTR BoneName, DirectX::XMMATRIX* pOutMat);
	bool GetPosFromBone(LPCSTR BoneName, DirectX::XMFLOAT3* pOutPos);

	//一括解放処理.
	HRESULT Release();

public:
	MY_HIERARCHY	cHierarchy;
	MY_HIERARCHY* m_pHierarchy;
	LPD3DXFRAME		m_pFrameRoot;

	// D3DX 依存のまま
	LPD3DXANIMATIONCONTROLLER	m_pAnimController;			//デフォルトで一つ.
	LPD3DXANIMATIONSET			m_pAnimSet[MAX_ANIM_SET];
};