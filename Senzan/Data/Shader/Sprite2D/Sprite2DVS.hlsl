#include "Sprite2D.hlsli"

/****************************************************************************
*	UI用頂点シェーダー.
****************************************************************************/
VSOutput main(
    float4 position : POSITION,
    float2 uv       : TEXCOORD)
{
    VSOutput output = (VSOutput) 0;
	
	output.position = mul( position, cb_mWorld);
 
	//スクリーン座標に合わせる計算.
    output.position.x = (output.position.x / cb_ViewPortW) * 2.0f - 1.0f;
	output.position.y = 1.0f - (output.position.y / cb_ViewPortH) * 2.0f;
	output.position.z = 0.5;
	output.position.w = 1;
	
	output.uv = uv;
	
	//UVスクロール(UV座標を操作する).
	output.uv.x += cb_UV.x;
	output.uv.y += cb_UV.y;

    output.localPosition= position;
	
	return output;
}