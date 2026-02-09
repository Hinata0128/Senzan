/********************************************************************************
*   スキンメッシュ用のシャドウマップのキャスター（深度記録）.
********************************************************************************/

// ボーン最大数.
static const int MAX_BONE_MATRICES = 255;

// メッシュ単位.
cbuffer mesh : register(b0)
{
    matrix cb_mWLVP; // ワールドライトビュープロジェクション行列.
}

// ボーンのポーズ行列.
cbuffer bones : register(b3)
{
    float4x4 cb_mConstBoneWorld[MAX_BONE_MATRICES];
}

// スキン後の頂点・法線.
struct Skin
{
    float4 position;
    float4 normal;
};

// 頂点バッファーの入力.
struct VSSkinIn
{
    float3 position	: POSITION;		// 位置.  
    float3 normal   : NORMAL;       // 頂点法線.	
	uint4  bones	: BONE_INDEX;	// ボーンのインデックス.
	float4 weights	: BONE_WEIGHT;	// ボーンの重み.
};

//ピクセルシェーダーの入力（頂点バッファーの出力）　
struct PSSkinIn
{
    float4 position         : SV_Position;  // 位置.    
};

// 指定した番号のボーンのポーズ行列を返す
matrix FetchBoneMatrix(uint bone)
{
    return cb_mConstBoneWorld[bone];
}

// 頂点をスキニング（ボーンにより移動）する.
Skin SkinVertex(VSSkinIn input)
{
    Skin output = (Skin) 0;
    
    float4 position = float4(input.position, 1.0f);  // 座標のwは 1.
    float4 normal   = float4(input.normal, 0.0f);    // ベクトルのwは 0.
    
	// ボーン0.
    uint bone       = input.bones.x;
    float weight    = input.weights.x;
	float4x4 m	    = FetchBoneMatrix( bone );
    output.position += weight * mul(position, m);
    output.normal   += weight * mul(normal, m);
	
	// ボーン1.
    bone            = input.bones.y;
    weight          = input.weights.y;
    m               = FetchBoneMatrix(bone);
    output.position += weight * mul(position, m);
    output.normal   += weight * mul(normal, m);
	
	// ボーン2.
    bone            = input.bones.z;
    weight          = input.weights.z;
    m               = FetchBoneMatrix(bone);
    output.position += weight * mul(position, m);
    output.normal   += weight * mul(normal, m);
	
	// ボーン3.
    bone            = input.bones.w;
    weight          = input.weights.w;
    m               = FetchBoneMatrix(bone);
    output.position += weight * mul(position, m);
    output.normal   += weight * mul(normal, m);
    
    return output;
}

PSSkinIn VSMain(VSSkinIn input)
{
    PSSkinIn output = (PSSkinIn) 0;
    
    // スキニング.
    Skin skinned = SkinVertex(input);
    
    // ライト視点.
    output.position = mul(skinned.position, cb_mWLVP);
    
    return output;
}

float4 PSMain(PSSkinIn input) : SV_Target
{
    return float4(input.position.z, input.position.z, input.position.z, 1.0f);
}