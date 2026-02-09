#include "Sprite2D.hlsli"

/****************************************************************************
*	UI用ピクセルシェーダー.
****************************************************************************/
float4 main(VSOutput input) : SV_Target
{
    float4 color = g_Texture.Sample(g_SamLinear, input.uv);
	
	// テクスチャの幅、高さを取得.
    float texWidth, texHeight;
    g_Texture.GetDimensions(texWidth, texHeight);
	
	// uv座標を計算.
    float u = cb_DrawSize.x / texWidth;
    float v = cb_DrawSize.y / texHeight;
    
	// 表示するuv座標を超えていたらマスクする.
    if (input.uv.x > u || input.uv.y > v)
    {
        color.a = 0.0f;
    }
	
	//プログラム制御のα値を設定する.
    color *= cb_Color;

    return color;
}