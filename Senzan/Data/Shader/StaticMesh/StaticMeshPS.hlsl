#include "StaticMesh.hlsli"

/************************************************************************
*   スタティックメッシュ用PSシェーダー.
************************************************************************/
float4 main(VSOutput input) : SV_Target
{
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
    
    return color;
}