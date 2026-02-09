Texture2D inputTex : register(t0);
SamplerState samp : register(s0);

cbuffer BlurCB : register(b0)
{
    float TexelWidth;
    float TexelHeight;
    int Horizontal; // 1: horizontal, 0: vertical
    float Padding;
};

static const float w0 = 0.227027f;
static const float w1 = 0.1945946f;
static const float w2 = 0.1216216f;
static const float w3 = 0.054054f;
static const float w4 = 0.016216f;

struct PS_IN { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };

float4 PSMain(PS_IN input) : SV_Target
{
    float2 uv = input.uv;
    float2 texel = float2(TexelWidth, TexelHeight);
    float2 dir = Horizontal == 1 ? float2(1,0) : float2(0,1);

    float4 color = w0 * inputTex.Sample(samp, uv);
    color += w1 * inputTex.Sample(samp, uv + dir * texel * 1.0f);
    color += w1 * inputTex.Sample(samp, uv - dir * texel * 1.0f);
    color += w2 * inputTex.Sample(samp, uv + dir * texel * 2.0f);
    color += w2 * inputTex.Sample(samp, uv - dir * texel * 2.0f);
    color += w3 * inputTex.Sample(samp, uv + dir * texel * 3.0f);
    color += w3 * inputTex.Sample(samp, uv - dir * texel * 3.0f);
    color += w4 * inputTex.Sample(samp, uv + dir * texel * 4.0f);
    color += w4 * inputTex.Sample(samp, uv - dir * texel * 4.0f);
    return color;
}

// Simple blend PS for accumulation: lerp(current, prev, factor)
Texture2D prevTex : register(t1);
cbuffer BlendCB : register(b1)
{
    float BlendFactor; // how much previous contributes
    float pad0; float pad1; float pad2;
};

float4 PSBlend(PS_IN input) : SV_Target
{
    float2 uv = input.uv;
    float4 curr = inputTex.Sample(samp, uv);
    float4 prev = prevTex.Sample(samp, uv);
    return lerp(curr, prev, BlendFactor);
}
