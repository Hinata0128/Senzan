#ifndef SKIN_MESH_HLSLI
#define SKIN_MESH_HLSLI

// ボーン最大数.
static const int MAX_BONE_MATRICES = 255;

Texture2D g_Texture     : register(t0);// テクスチャ.
Texture2D g_ShadowMap   : register(t1);// シャドウマップ.

SamplerState g_SamLinear : register(s0); //サンプラ

// メッシュ単位.
cbuffer mesh : register(b0)
{
    matrix cb_mW;      // ワールド行列.
    matrix cb_mWVP;    // ワールド、ビュー、プロジェクションの合成行列.
    matrix cb_mWLVP;   // ワールド、ライトビュー、ライトプロジェクションの合成行列.
}

// マテリアル単位.
cbuffer material : register(b1)
{
    float4 cb_Diffuse; // 拡散反射色.
    float4 cb_Ambient; // 環境色.
    float4 cb_Specular;// 鏡面反射色.
    float4 cb_Emissive;// 自己発光色.
    float4 cb_Dissolve;// ディゾルブ (x=閾値, y=エッジ幅, z=有効フラグ, w=予約).
}

// フレーム単位.
cbuffer frame : register(b2)
{
    float4 cb_CameraPosition;   // カメラ座標（視点）.
    float4 cb_LightDirection;   // ライト方向.
    float4 cb_IsLight;          // ライトを使用するか（xのみ使用）.
    float4 cb_IsShadow;         // 影を適用するか（xのみ使用）.
}

// ボーンのポーズ行列.
cbuffer bones : register(b3)
{
    float4x4 cb_mConstBoneWorld[MAX_BONE_MATRICES];
}

// スキン後の頂点・法線.
struct Skin
{
    float4 position;
    float4 normal;
};

// 頂点バッファーの入力.
struct VSSkinIn
{
    float3 position	: POSITION;		// 位置.  
    float3 normal   : NORMAL;       // 頂点法線.
	float2 uv		: TEXCOORD;		// UV座標（テクスチャー座標）.
	uint4  bones	: BONE_INDEX;	// ボーンのインデックス.
	float4 weights	: BONE_WEIGHT;	// ボーンの重み.
};

//ピクセルシェーダーの入力（頂点バッファーの出力）　
struct PSSkinIn
{
    float4 position         : SV_Position;  // 位置.
    float3 normal           : TEXCOORD0;    // 頂点法線.
    float2 uv               : TEXCOORD1;    // UV座標（テクスチャー座標）.
	float3 lightDirection	: TEXCOORD2;	// ライト.
	float3 eyeVector	    : TEXCOORD3;	// 視線ベクトル.
	float4 worldPosition    : TEXCOORD4;	// ワールド座標.
    float4 lightViewPosition: TEXCOORD5;    // ライトビュー座標.
	float4 color		    : COLOR;		// 最終カラー（頂点シェーダーにおいての）.
};

#endif // SKIN_MESH_HLSLI
