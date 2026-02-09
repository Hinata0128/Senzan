// PostEffect.hlsli
struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

// 円状グレースケールエフェクト用定数バッファ
cbuffer CircleGrayBuffer : register(b0)
{
    float g_CircleRadius;    // 現在の円の半径（0.0〜2.0くらい）
    float g_IsExpanding;     // 1.0: 広がり中, 0.0: 戻り中
    float g_EffectActive;    // 1.0: エフェクト有効, 0.0: 無効
    float g_AspectRatio;     // アスペクト比補正
};
