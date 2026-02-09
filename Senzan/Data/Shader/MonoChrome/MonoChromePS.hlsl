#include "MonoChrome.hlsli"

Texture2D tex0 : register(t0);
SamplerState samp : register(s0);

float4 main(VS_OUT input) : SV_Target
{
    // テクスチャサンプリング
    float3 c = tex0.Sample(samp, input.uv).rgb;

    // NTSC係数による輝度計算（グレースケール）
    float g = dot(c, float3(0.299, 0.587, 0.114));
    float3 grayColor = float3(g, g, g);

    // エフェクトが無効なら通常色を返す
    if (g_EffectActive < 0.5f)
    {
        return float4(c, 1.0f);
    }

    // 画面中心からの距離を計算（アスペクト比補正）
    float2 center = float2(0.5f, 0.5f);
    float2 diff = input.uv - center;
    diff.x *= g_AspectRatio;  // アスペクト比補正
    float dist = length(diff);

    // 円状エフェクト
    // 広がり中: 円の内側がグレー、外側が通常
    // 戻り中: 円の内側が通常、外側がグレー
    float3 finalColor;
    
    if (g_IsExpanding > 0.5f)
    {
        // 広がり中: 半径より内側がグレースケール
        float blend = smoothstep(g_CircleRadius - 0.05f, g_CircleRadius + 0.05f, dist);
        finalColor = lerp(grayColor, c, blend);
    }
    else
    {
        // 戻り中: 半径より内側が通常色
        float blend = smoothstep(g_CircleRadius - 0.05f, g_CircleRadius + 0.05f, dist);
        finalColor = lerp(c, grayColor, blend);
    }

    return float4(finalColor, 1.0f);
}
