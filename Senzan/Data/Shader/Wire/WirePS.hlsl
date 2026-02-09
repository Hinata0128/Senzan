// VSから渡される構造体（VS_OUTPUTと一致させる）
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
};

float4 PS_Main(PS_INPUT input) : SV_TARGET
{
    // ワイヤーフレームの色をそのまま出力.
    return input.Color;
}