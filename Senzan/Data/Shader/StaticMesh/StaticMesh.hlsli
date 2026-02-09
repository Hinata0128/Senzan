#ifndef STATIC_MESH_HLSLI
#define STATIC_MESH_HLSLI

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
}

// フレーム単位.
cbuffer frame : register(b2)
{
    float4 cb_CameraPosition;   // カメラ座標（視点）.
    float4 cb_LightDirection;   // ライト方向.
    float4 cb_IsLight;          // ライトを使用するか（xのみ使用）.
    float4 cb_IsShadow;         // 影を適用するか（xのみ使用）.
}

// 頂点シェーダーの出力パラメーター.
struct VSOutput
{
    float4 position             : SV_Position;  // 座標（プロジェクション変換）.
    float3 normal               : TEXCOORD0;    // 法線.
    float2 uv                   : TEXCOORD1;    // UV座標.
    float3 lightDirection       : TEXCOORD2;    // ライト方向.
    float3 eyeVector            : TEXCOORD3;    // 視線ベクトル.
    float4 worldPosition        : TEXCOORD4;    // 座標（ワールド変換）.
    float4 lightViewPosition    : TEXCOORD5;    // ライトビュー座標.
    float4 color                : COLOR;
};

#endif // STATIC_MESH_HLSLI