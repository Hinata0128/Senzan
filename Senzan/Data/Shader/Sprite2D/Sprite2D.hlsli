#ifndef SPRITE2D_HLSLI
#define SPRITE2D_HLSLI

// テクスチャ.
Texture2D g_Texture : register(t0);

// サンプラ.
SamplerState g_SamLinear : register(s0);

// メッシュ単位.
cbuffer mesh : register(b0)
{
    matrix	cb_mWorld	: packoffset( c0 );	// ワールド行列.
	float4	cb_Color	: packoffset( c4 );	// 色（RGBA:xyzw）.
	float4	cb_UV		: packoffset( c5 );	// UV座標（xyしか使わない）.
	float	cb_ViewPortW: packoffset( c6 );	// ビューポート幅.
	float	cb_ViewPortH: packoffset( c7 );	// ビューポート高さ.
	float2	cb_DrawSize	: packoffset( c8 ); // 描画幅高さ.
}

// 頂点シェーダーの出力.
struct VSOutput
{
    float4 position			: SV_Position;	// 座標.
    float2 uv				: TEXCOORD0;	// UV座標.
    float4 localPosition	: TEXCOORD1;	// ローカル座標.
};

#endif // SPRITE2D_HLSLI