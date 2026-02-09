#pragma once
#include <filesystem>
#include "Utility/Transform/RectTransform.h"


#pragma warning(disable:4005)
#define ALIGN16	_declspec( align(16) )

class VertexShaderBase;
class PixelShaderBase;

class Sprite2D
{
public:

	//頂点の構造体.
	struct VERTEX
	{
		DirectX::XMFLOAT3 pos;	//頂点座標.
		DirectX::XMFLOAT2	tex;	//テクスチャ座標.
	};

	//幅高さ構造体.
	struct  WHSIZE
	{
		float w;
		float h;
	};

	//スプライト構造体.
	struct SPRITE_STATE
	{
		WHSIZE Disp;	//表示幅高さ.
		WHSIZE Base;	//元画像幅高さ.
		WHSIZE Stride;	//1コマあたりの幅高さ.
	};

	//コンスタントバッファのアプリ側の定義.
	struct SHADER_CONSTANT_BUFFER
	{
		ALIGN16	DirectX::XMMATRIX	mWorld;			//ワールド行列.	
		ALIGN16	DirectX::XMFLOAT4	vColor;			//カラー（RGBAの型に合わせる）.
		ALIGN16	DirectX::XMFLOAT4	vUV;			//UV座標（x,yのみ使用）.
		ALIGN16 float		fViewPortWidth;	//ビューポート幅.
		ALIGN16 float		fViewPortHeight;//ビューポート高さ.
		ALIGN16 DirectX::XMFLOAT2 DawSize;		// 描画幅高さ.
	};

	// DirectX::XMFLOAT2をキーとして使用するためのハッシュ.
	struct HASH_D3DXVECTER2
	{
		size_t operator()(const DirectX::XMFLOAT2& key) const {
			return std::hash<float>()(key.x) ^ std::hash<float>()(key.y);
		}
	};
	// DirectX::XMFLOAT2を比較するための等価比較関数.
	struct EQUAL_XMFLOAT2
	{
		bool operator()(const DirectX::XMFLOAT2& lhs, const DirectX::XMFLOAT2& rhs) const
		{
			// unordered_mapの動作に必要な等価演算子 (operator==) を定義します。
			// floatの比較は、一般的には許容誤差 (epsilon) を使うべきですが、
			// 簡略化のため、ここでは完全一致をチェックします。
			return lhs.x == rhs.x && lhs.y == rhs.y;
		}
	};
public:
	Sprite2D();
	~Sprite2D();


	/*********************************************************
	* @brief 初期化.
	* @param filePath：画像のファイルパス.
	*********************************************************/
	bool Initialize(const std::filesystem::path& filePath);


	/*********************************************************
	* @brief 描画.
	*********************************************************/
	void Render();

public: // Getter、Setter.

	/*********************************************************
	* @brief RectTransformを取得.
	*********************************************************/
	const std::unique_ptr<RectTransform>& GetRectTransform() const;


	/*********************************************************
	* @brief 描画幅、高さを設定.
	*********************************************************/
	void SetDrawSize(const DirectX::XMFLOAT2& drawSize);


	/*********************************************************
	* @brief 色を設定.
	*********************************************************/
	void SetColor(const DirectX::XMFLOAT4& color);


	/*********************************************************
	* @brief 資源名を取得.
	*********************************************************/
	const std::string& GetResourceName()const ;


	/*********************************************************
	* @brief 資源名を設定.
	*********************************************************/
	void SetResourceName(const std::string& name);


	/*********************************************************
	* @brief テクスチャを取得.
	*********************************************************/
	ID3D11ShaderResourceView* GetTexture();

private:

	/*********************************************************
	* @brief 画像サイズの読み込み.
	*********************************************************/
	void LoadImageSize(const std::filesystem::path& filePath);


	/*********************************************************
	* @brief シェーダー作成.
	*********************************************************/
	void CreateShader();


	/*********************************************************
	* @brief モデル作成.
	*********************************************************/
	const HRESULT CreateModel();


	/*********************************************************
	* @brief テクスチャ作成.
	*********************************************************/
	const HRESULT CreateTexture(const std::wstring& filePath);


	/*********************************************************
	* @brief サンプラ作成.
	*********************************************************/
	const HRESULT CreateSampler();


	/*********************************************************
	* @brief ワールド行列を計算.
	*********************************************************/
	void CalcWorldMatrix();


	/*********************************************************
	* @brief 描画に使用する頂点バッファを取得.
	*********************************************************/
	ID3D11Buffer* GetUseVertexBuffer();

private:
	std::unique_ptr<RectTransform>		m_pRectTransform;
	std::unique_ptr<VertexShaderBase>	m_pVertexShader;
	std::unique_ptr<PixelShaderBase>	m_pPixelShaderBase;

	std::unordered_map<DirectX::XMFLOAT2, ID3D11Buffer*, HASH_D3DXVECTER2, EQUAL_XMFLOAT2> m_pCashVertexBuffers;
	ID3D11Buffer*				m_pConstantBuffer;	//コンスタントバッファ.
	ID3D11ShaderResourceView*	m_pTexture;			//テクスチャ.
	ID3D11SamplerState*			m_pSampleLinear;	//サンプラ:テクスチャに各種フィルタをかける.

	std::string m_ResourceName;	// 使用している資源の名前.
	DirectX::XMMATRIX	m_WorldMatrix;	// ワールド行列.
	DirectX::XMFLOAT2 m_DrawSize;		// 表示幅、高さ.
	DirectX::XMFLOAT4 m_Color;		// 色（R,G,B,A）
};