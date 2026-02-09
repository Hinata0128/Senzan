#pragma once

//警告についてのコード分析を無効にする. 4005:再定義.
#pragma warning( disable:4005)

//_declspec()	:DLLから(関数,クラス,クラスのメンバ関数を)エクスポートする.
//align()		:(強制的に)16byte で使用する.
#define ALIGN16	_declspec( align(16) )

#include <DirectXMath.h>	// DirectXMathライブラリ.

//前方宣言.
class DirectX9;
class DirectX11;
class CameraBase;
class VertexShaderBase;
class PixelShaderBase;

/***********************************************************************
*	メッシュベースクラス.
***********************************************************************/
class MeshBase
{
public:
	//======================================
	//	構造体.
	//======================================s
	// コンスタントバッファのアプリ側の定義.
	// ※シェーダ内のコンスタントバッファと一致している必要あり.
	// メッシュ単位.
	struct CBUFFER_PER_MESH
	{
		DirectX::XMMATRIX mW;			// ワールド行列.
		DirectX::XMMATRIX mWVP;		// ワールドから射影までの変換行列.
		DirectX::XMMATRIX mWLVP;		// ワールドライトプロジェクション.		
	};


	//マテリアル単位.
	struct CBUFFER_PER_MATERIAL
	{
		DirectX::XMFLOAT4 Diffuse;	// 拡散反射光（ディフューズ）.
		DirectX::XMFLOAT4 Ambient;	// 環境光（アンビエント）.
		DirectX::XMFLOAT4 Specular;	// 鏡面反射光（スペキュラ）.
		DirectX::XMFLOAT4 Emissive;	// 自己発光（エミッシブ）.
		DirectX::XMFLOAT4 Dissolve;	// ディゾルブ (x=閾値, y=エッジ幅, z=有効フラグ, w=予約).
	};

	//シェーダーに渡す値.
	struct CBUFFER_PER_FRAME
	{
		DirectX::XMFLOAT4 CameraPos;		// カメラ位置.
		DirectX::XMFLOAT4 vLightDir;		// ライト方向.
		DirectX::XMFLOAT4 isLight;		// ライト使用フラグ.
		DirectX::XMFLOAT4 isShadow;		// 影使用フラグ.
	};

	//頂点構造体.
	struct VERTEX
	{
		DirectX::XMFLOAT3	Position;	// 頂点位置.
		DirectX::XMFLOAT3	Normal;		// 頂点法線.
		DirectX::XMFLOAT2	Texture;	// UV座標.
	};

public:
	MeshBase();
	virtual ~MeshBase();

	// 初期化.
	virtual HRESULT Init(std::string FileName, const std::string& Name) = 0;

public: // Getter、Setter.

	/******************************************************************
	* @brief 座標を設定.
	******************************************************************/
	void SetPosition(const DirectX::XMFLOAT3& Position);


	/******************************************************************
	* @brief 回転を設定.
	******************************************************************/
	void SetRotation(const DirectX::XMFLOAT3& Rotation);


	/******************************************************************
	* @brief クォータニオンを設定.
	******************************************************************/
	void SetQuaternion(const DirectX::XMFLOAT4& quaternion);

	/******************************************************************
	* @brief 拡縮を設定.
	******************************************************************/
	void SetScale(const DirectX::XMFLOAT3& Scale);


	/******************************************************************
	* @brief ライトを使用するか設定.
	******************************************************************/
	void SetIsLight(const bool& isLight);


	/******************************************************************
	* @brief 影を落とすか設定.
	******************************************************************/
	void SetIsShadow(const bool& isShadow);


	/******************************************************************
	* @brief モデル名を取得.
	******************************************************************/
	const std::string& GetResourceName() const;

protected:
	// ワールド行列を計算.
	void CalcWorldMatrix();
protected:
	//　Dx9.
	LPDIRECT3DDEVICE9	m_pDevice9;

	//　Dx11.
	ID3D11Device* m_pDevice11;
	ID3D11DeviceContext* m_pContext11;

	std::unique_ptr<VertexShaderBase>	m_pVertexShader;// 頂点シェーダー.
	std::unique_ptr<PixelShaderBase>	m_pPixelShader;	// ピクセルシェーダー.
	ID3D11SamplerState* m_pSampleLinear;	//　サンプラ:テクスチャに各種フィルタをかける.

	ID3D11Buffer* m_pCBufferPerMesh;		// コンスタントバッファ(メッシュ毎).
	ID3D11Buffer* m_pCBufferPerMaterial;	// コンスタントバッファ(マテリアル毎).
	ID3D11Buffer* m_pCBufferPerFrame;		// コンスタントバッファ(フレーム毎).

	Transform		m_spTransform;
	DirectX::XMFLOAT4	m_Quaternion;

	DirectX::XMMATRIX		m_WorldMatrix;// ワールド行列.

	std::string m_ResourceName;// モデル固有の名前.

	float m_OriginalRadius;// モデルのオリジナル半径.
	float m_Radius;// モデルの半径.

	DirectX::XMFLOAT3 m_CenterPosition;// モデルの中心座標.

	bool m_IsLight; // ライトを使用するか.
	bool m_IsShadow;// 影を使用するか.
};
