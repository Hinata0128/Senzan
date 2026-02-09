#include "StaticMesh.hlsli"

/************************************************************************
*   テクスチャ無しスタティックメッシュ用PSシェーダー.
************************************************************************/
float4 main(VSOutput input) : SV_Target
{
    // 最終色.
    float4 color = { 0.0f, 0.0f, 0.0f, 0.0f };
    
    // ライトを使用する場合、光の計算を行う.
    if (cb_IsLight.x >= 1.0f)
    {
        // 環境光.
        float4 ambient = cb_Ambient;
    
        // 拡散反射光.
        float nl = saturate(dot(input.normal, input.lightDirection));
        float4 diffuse = cb_Diffuse * nl;
    
        // 鏡面反射光.
        float3 reflect = normalize(2.0f * nl * input.normal - input.lightDirection);
        float4 specular = pow(saturate(dot(reflect, input.eyeVector)), 4.0f) * cb_Specular;
    
        color = ambient + diffuse + specular + cb_Emissive;
    }
    
    return color;
}