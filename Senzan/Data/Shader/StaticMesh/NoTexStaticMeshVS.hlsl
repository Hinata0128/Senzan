#include "StaticMesh.hlsli"

/************************************************************************
*   テクスチャ無しスタティックメッシュ用VSシェーダー.
************************************************************************/
VSOutput main(
    float4 pos      : POSITION,
    float4 normal   : NORMAL)
{
    VSOutput output = (VSOutput) 0;
    
    // プロジェクション変換.
    output.position = mul(pos, cb_mWVP);
    
    // 法線をモデルの姿勢に合わせる.
    output.normal = mul(normal, (float3x3) cb_mW);
    output.normal = normalize(output.normal);
    
    // ライト方向.
    output.lightDirection = normalize(-cb_LightDirection);
    
    // ワールド変換.
    output.worldPosition = mul(pos, cb_mW);
    
    // 視線ベクトル.
    output.eyeVector = normalize(cb_CameraPosition - output.worldPosition);    
    
    // ライト座標のワールド変換.
    output.lightViewPosition = mul(pos, cb_mWLVP);
        
    return output;
}
