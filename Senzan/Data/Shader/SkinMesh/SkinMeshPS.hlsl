#include "SkinMesh.hlsli"

// 簡易ノイズ関数（プロシージャル）
float hash(float2 p)
{
    return frac(sin(dot(p, float2(127.1, 311.7))) * 43758.5453123);
}

float noise(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);
    f = f * f * (3.0 - 2.0 * f);
    
    float a = hash(i);
    float b = hash(i + float2(1.0, 0.0));
    float c = hash(i + float2(0.0, 1.0));
    float d = hash(i + float2(1.0, 1.0));
    
    return lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y);
}

float4 main(PSSkinIn input) : SV_Target
{
    // ディゾルブ処理
    if (cb_Dissolve.z >= 1.0f) // 有効フラグ
    {
        // UV座標からノイズを生成
        float dissolveNoise = noise(input.uv * 10.0f);
        
        // 閾値より小さい部分を破棄
        if (dissolveNoise < cb_Dissolve.x)
        {
            discard;
        }
        
        // エッジ発光（ディゾルブ境界を光らせる）
        float edgeWidth = cb_Dissolve.y; // エッジ幅
        float edgeGlow = 0.0f;
        if (dissolveNoise < cb_Dissolve.x + edgeWidth)
        {
            edgeGlow = 1.0f - (dissolveNoise - cb_Dissolve.x) / edgeWidth;
        }
        
        // 通常の色計算
        float4 color = { 0.0f, 0.0f, 0.0f, 0.0f };
        float4 texterColor = g_Texture.Sample(g_SamLinear, input.uv);
        
        if (cb_IsLight.x >= 1.0f)
        {
            float4 ambient = texterColor * cb_Ambient;
            float nl = saturate(dot(input.normal, input.lightDirection));
            float4 diffuse = ((cb_Diffuse * 0.5f) + (texterColor * 0.5f)) * nl;
            float3 reflect = normalize(2.0f * nl * input.normal - input.lightDirection);
            float4 specular = pow(saturate(dot(reflect, input.eyeVector)), 4.0f) * cb_Specular;
            color = ambient + diffuse + specular + cb_Emissive;
        }
        else
        {
            color = texterColor;
        }
        
        // 影処理
        if (cb_IsShadow.x >= 1.0f)
        {
            float2 shadowMapUV = input.lightViewPosition.xy / input.lightViewPosition.w;
            shadowMapUV *= float2(0.5f, -0.5f);
            shadowMapUV += 0.5f;
            float zInLVP = input.lightViewPosition.z / input.lightViewPosition.w;
            if (shadowMapUV.x > 0.0f && shadowMapUV.x < 1.0f
                && shadowMapUV.y > 0.0f && shadowMapUV.y < 1.0f)
            {
                float zInShadowMap = g_ShadowMap.Sample(g_SamLinear, shadowMapUV).r;
                if (zInLVP > zInShadowMap + 0.0005f)
                {
                    color.xyz *= 0.5f;
                }
            }
        }
        
        // エッジ発光を追加（オレンジ色）
        color.rgb += float3(1.0f, 0.5f, 0.0f) * edgeGlow * 2.0f;
        
        float alpha = cb_Diffuse.w * texterColor.a;
        color.a = alpha;
        return color;
    }
    
    // 通常描画（ディゾルブ無効時）
    // 最終色.
    float4 color = { 0.0f, 0.0f, 0.0f, 0.0f };

    // テクスチャカラー.
    float4 texterColor = g_Texture.Sample(g_SamLinear, input.uv);
    
    // ライトを使用する場合、光の計算を行う.
    if (cb_IsLight.x >= 1.0f)
    {
        // 環境光.
        float4 ambient = texterColor * cb_Ambient;
    
        // 拡散反射光.
        float nl = saturate(dot(input.normal,input.lightDirection));
        float4 diffuse = ((cb_Diffuse * 0.5f) + (texterColor * 0.5f)) * nl;
    
        // 鏡面反射光.
        float3 reflect = normalize(2.0f * nl * input.normal - input.lightDirection);
        float4 specular = pow(saturate(dot(reflect, input.eyeVector)), 4.0f) * cb_Specular;
    
        color = ambient + diffuse + specular + cb_Emissive;        
    }
    else
    {
        color = texterColor;
    }
    
    // 影を落とす場合、影が落ちる部分のカラーを減らす.
    if(cb_IsShadow.x >= 1.0f)
    {
        // ライト空間での位置をスクリーン座標に変換.        
        float2 shadowMapUV = input.lightViewPosition.xy / input.lightViewPosition.w;
        shadowMapUV *= float2(0.5f, -0.5f);
        shadowMapUV += 0.5f;
        
        // ライト空間内での深度値を取得.
        float zInLVP = input.lightViewPosition.z / input.lightViewPosition.w;

        // シャドウマップ内の座標が0.0f～1.0fの範囲内にあるか.
        if (shadowMapUV.x > 0.0f && shadowMapUV.x < 1.0f
		    && shadowMapUV.y > 0.0f && shadowMapUV.y < 1.0f)
        {
            // シャドウマップから現在の座標の深度をサンプリング.
            float zInShadowMap = g_ShadowMap.Sample(g_SamLinear, shadowMapUV).r;
            
            // ライト空間での深度がシャドウマップ内の深度値より大きければ、影が落ちている.
            if (zInLVP > zInShadowMap + 0.0005f)
            {
                color.xyz *= 0.5f;
            }
        }
    }
    
    // アルファはマテリアルのDiffuse.wとテクスチャのアルファを乗算して出力
    float alpha = cb_Diffuse.w * texterColor.a;
    color.a = alpha;
    return color;
}
