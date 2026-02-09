#pragma once
#include "..\MeshBase.h"

// 警告についてのコード分析を無効にする. 4005:再定義.
#pragma warning( disable:4005)

// _declspec()	:DLLから(関数,クラス,クラスのメンバ関数を)エクスポートする.
// align()		:(強制的に)16byte で使用する.
#define ALIGN16	_declspec( align(16) )

// 前方宣言.
class DirectX9;
class DirectX11;
class CameraBase;
class LightManager;

/**************************************************
*	スタティックメッシュクラス	
**************************************************/
class StaticMesh final
	: public MeshBase
{
public:
	// マテリアル構造体.
	struct MY_MATERIAL
	{
		DirectX::XMFLOAT4	Diffuse;	// ディフューズ.
		DirectX::XMFLOAT4	Ambient;	// アンビエント.
		DirectX::XMFLOAT4	Specular;	// スペキュラ.
		DirectX::XMFLOAT4 Emissive;	// エミッシブ.
		float		Power;		// スペキュラパワー.
		TCHAR TextureName[256];	// テクスチャファイル名.
		ID3D11ShaderResourceView* pTexture;// テクスチャ.
		DWORD dwNumFace;		// マテリアルのポリゴン数.

		MY_MATERIAL()
			: Diffuse		()
			, Ambient		()
			, Specular		()
			, Emissive		()
			, Power			()
			, TextureName	()
			, pTexture		( nullptr )
			, dwNumFace		( 0 )
		{
		}
		~MY_MATERIAL() {
			SAFE_RELEASE( pTexture );
		}
	};

	// Xファイルのロードで必要。受け取るもの.
	struct MY_MODEL
	{
		TCHAR			FileName[256];		// ファイル名.
		LPD3DXMESH		pMesh;				// メッシュオブジェクト.
		LPD3DXBUFFER	pD3DXMtrlBuffer;	// マテリアルバッファ.
		DWORD			NumMaterials;		// マテリアル数.

		MY_MODEL()
			: FileName			()
			, pMesh				( nullptr )
			, pD3DXMtrlBuffer	( nullptr )
			, NumMaterials		()
		{}
		~MY_MODEL(){
			SAFE_RELEASE( pMesh );
			SAFE_RELEASE( pD3DXMtrlBuffer );
		}
	};

public:
	StaticMesh();			
	~StaticMesh()override;	

	// 初期化処理.
	HRESULT Init(std::string FileName, const  std::string& Name) override;

	// 解放関数.
	void Release();

	// レンダリング用.
	void Render();

	// 深度マップに描画.
	void RenderDepth();

	// メッシュを取得.
	const LPD3DXMESH& GetMesh() const;

	//レイとの当たり判定用のメッシュを取得.
	LPD3DXMESH GetMeshForRay() const { return m_ModelForRay.pMesh; }

	// 頂点バッファを取得.
	ID3D11Buffer* GetVertexBuffer();
	// インデックスバッファを取得.
	std::vector<ID3D11Buffer*> GetIndexBuffer();
	// インデックスを取得.
	std::vector<DWORD> GetIndex();

private:
	// メッシュ読み込み.
	HRESULT LoadXMesh(std::string lpFileName);
	// シェーダ作成.
	HRESULT CreateShader();
	// モデル作成.
	HRESULT CreateModel();
	// マテリアル作成.
	HRESULT CreateMaterials();
	// インデックスバッファ作成.
	HRESULT CreateIndexBuffer();
	// 頂点バッファ作成.
	HRESULT CreateVertexBuffer();
	// コンスタントバッファ作成.
	HRESULT CreateConstantBuffer();
	// サンプラ作成.
	HRESULT CreateSampler();

	//レンダリング関数(クラス内でのみ使用する).
	void RenderMesh(DirectX::XMMATRIX& mWorld, DirectX::XMMATRIX& mView, DirectX::XMMATRIX& mProj);
	// 深度マップにレンダリング.
	void RenderMehDepsh(DirectX::XMMATRIX& mWorld, DirectX::XMMATRIX& mView, DirectX::XMMATRIX& mProj);

	// モデルの半径を取得する.
	float GetModelRadius();
private:
	ID3D11Buffer*		m_pVertexBuffer;	// 頂点バッファ.
	ID3D11Buffer**		m_ppIndexBuffer;	// インデックスバッファ.
	MY_MODEL			m_Model;
	MY_MODEL			m_ModelForRay;		// レイとの当たり判定で使用する.
	MY_MATERIAL*		m_pMaterials;		// マテリアル構造体.
	DWORD				m_NumAttr;			// 属性数.
	DWORD				m_AttrID[300];		// 属性ID ※300属性まで.
	bool				m_EnableTexture;	// テクスチャあり.
 };
