#include "MonoChrome.hlsli"

VS_OUT main(uint id : SV_VertexID)
{
    VS_OUT o;

    // スクリーン全体を覆う三角形/四角形の頂点
    float2 pos[4] = { float2(-1, -1), float2(-1, 1), float2(1, -1), float2(1, 1) };
    float2 uv[4] = { float2(0, 1), float2(0, 0), float2(1, 1), float2(1, 0) };

    o.pos = float4(pos[id], 0, 1);
    o.uv = uv[id];
    return o;
}
