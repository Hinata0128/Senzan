#include "SkinMesh.hlsli"

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

PSSkinIn main(VSSkinIn input)
{
    PSSkinIn output = (PSSkinIn) 0;
    
    // スキニング.
    Skin skinned = SkinVertex(input);
    
    //プロジェクション変換.
    output.position = mul(skinned.position, cb_mWVP);    
	
	 // 法線をモデルの姿勢に合わせる.
    output.normal = normalize((float3) mul(skinned.normal, cb_mW));
	
	 // ライト方向（-にして正規化しているのは方向と反対の面が暗くなってしまうため）.
    output.lightDirection = normalize(-cb_LightDirection.xyz);    
	
	//ワールド変換.
    output.worldPosition = mul(float4(input.position, 1.0f), cb_mW);    
	
	//視線ベクトル:
    output.eyeVector = normalize(cb_CameraPosition.xyz - output.worldPosition.xyz);    

	//UV座標（テクスチャ座標）.	
    output.uv = input.uv;

	// ライトビューのワールド座標.    
    output.lightViewPosition = mul(skinned.position, cb_mWLVP);
    
    return output;
}
