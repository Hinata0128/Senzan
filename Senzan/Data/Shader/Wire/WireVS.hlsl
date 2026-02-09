
// レジスタ b0: 投影行列 (アプリケーション側で更新).
cbuffer ViewProjCBuffer : register(b0)
{
    matrix ViewProj;
};

// 頂点データ (スロット0).
struct VS_INPUT_VERTEX
{
    float4 Position : POSITION;
};

// インスタンスデータ.
// DebugColliderInfo 構造体に相当する.
struct VS_INPUT_INSTANCE
{
    // 1. World (Offset 0 - 48)
    float4 World0 : WORLD0;
    float4 World1 : WORLD1;
    float4 World2 : WORLD2;
    float4 World3 : WORLD3;
    
    float4 Color : COLOR0;
    
    int ShapeInfo : SHAPETYPE0;
    
    float4 Data0 : DATA0;
    float4 Data1 : DATA1;
};
// VSからPSへ渡すデータ.
struct VS_OUTPUT
{
    float4 Position : SV_POSITION;  // 最終的なスクリーン座標.
    float4 Color    : COLOR0;       // デバッグカラー.
};

// EP.
VS_OUTPUT VS_Main(VS_INPUT_VERTEX inputV, VS_INPUT_INSTANCE inputI)
{
    VS_OUTPUT output;
    
    // ワールド行列を再構築.
    matrix World = { inputI.World0, inputI.World1, inputI.World2, inputI.World3 };
    
    // それぞれの形状に合わせてサイズを変更.
    float4 ScaledPosition = inputV.Position;
    int ShapeType = (int) inputI.ShapeInfo.x;
    
    // それぞれの値.
    float Radius = inputI.Data0.x;      // Sphere, Capsule  : 半径.
    float Height = inputI.Data0.y;      // Capsule          : 高さ.
    float3 Size  = inputI.Data0.xyz;    // Box              : サイズ.
    
    // 単位メッシュの基準値.
    float R_unit = 0.5f;
    float H_unit_half = 1.0f;

    // Boxの場合.
    if (ShapeType == 0)
    {
        ScaledPosition.xyz *= Size;
    }
    
    // Sphereの場合.
    else if (ShapeType == 1)
    {
        float scaleFactor = Radius / R_unit;
        ScaledPosition.xyz *= scaleFactor;
    }
    // Capsulの場合.
    else if (ShapeType == 2)
    {
        float ScaleXZ = Radius / R_unit;
        float CylinderHalfLength = (Height - 2.0f * Radius) / 2.0f;
        ScaledPosition.x *= ScaleXZ;
        ScaledPosition.z *= ScaleXZ;
       
        // 南極側の半球 (Y <= -1.0).
        if (ScaledPosition.y < -H_unit_half + 0.001f) // Y <= -1.0.
        {
            ScaledPosition.y = -CylinderHalfLength + ((ScaledPosition.y + H_unit_half) * ScaleXZ);
        }
        // 北極側の半球 (Y >= 1.0).
        else if (ScaledPosition.y > H_unit_half - 0.001f) // Y >= 1.0.
        {
            ScaledPosition.y = CylinderHalfLength + ((ScaledPosition.y - H_unit_half) * ScaleXZ);
        }
        // 円筒部分 (-1.0 < Y < 1.0).
        else
        {
            ScaledPosition.y *= (CylinderHalfLength / H_unit_half);
        }
    }
    
    // World * ViewProj で最終座標を計算.
    output.Position = mul(mul(ScaledPosition, World), ViewProj);
    
    // ピクセルシェーダーへデバッグカラーを渡す.
    output.Color = inputI.Color;
    
    return output;
}
