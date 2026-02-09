/********************************************************************************
*   スタティックメッシュ用のシャドウマップのキャスター（深度記録）.
********************************************************************************/

// コンスタントバッファ.
cbuffer cb : register(b0)
{
    matrix cb_mWLVP; // ワールドライトビュープロジェクション行列.
}

// 頂点シェーダーの出力.
struct VSOutput
{
    float4 position : SV_Position;
};

VSOutput VSMain(float4 position : POSITION)
{
    VSOutput output = (VSOutput) 0;
    
    output.position = mul(position, cb_mWLVP);
    
    return output;
}

float PSMain(VSOutput input) : SV_Target
{
    return float4(input.position.z, input.position.z, input.position.z, 1.0f);
}