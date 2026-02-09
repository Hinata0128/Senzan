#include "Header.hlsli"

Output VS(VSInput input)
{
    Output output;

    // スキニング処理を開始
    // float4 skinnedPos = float4(0.0f, 0.0f, 0.0f, 1.0f); // 初期化はこれでOK
    // float3 skinnedNormal = float3(0.0f, 0.0f, 0.0f);     // 法線もこれでOK
    // ただし、累積していくので、ゼロで初期化して最後に合算したものが結果となる

    float4 cumulativeSkinnedPos = float4(0.0f, 0.0f, 0.0f, 0.0f); // 累積用
    float3 cumulativeSkinnedNormal = float3(0.0f, 0.0f, 0.0f); // 累積用

    float totalWeight = 0.0f; // スキニングが適用されない頂点のフォールバック用

    // 各ボーンのウェイトとインデックスに基づいてスキニングを適用
    // PMXの仕様上、BoneIndicesの最大数は4つ
    for (int i = 0; i < 4; ++i)
    {
        // ウェイトが0より大きい場合のみ処理（浮動小数点誤差を考慮）
        if (input.boneWeights[i] > 0.0001f)
        {
            uint boneIndex = input.boneIndices[i];
            float boneWeight = input.boneWeights[i];

            // ボーンインデックスが有効範囲内か確認
            if (boneIndex < BoneCount)
            {
                float4x4 boneTransform = boneTransforms[boneIndex]; // GPUから取得したボーンのスキニング行列

                // 頂点位置のスキニング
                // ここは CPU 側で InitialInverseBindPose * CurrentGlobalBoneTransform を計算して渡す場合、
                // HLSLでは mul(vector, matrix) の順序が一般的です。
                // mul(vector, matrix) の場合: vector が行ベクトル、matrix が通常行列
                // mul(matrix, vector) の場合: matrix が通常行列、vector が列ベクトル
                // DirectXMath は通常 mul(M1, M2) = M1 * M2 で、これは v' = v * M という行ベクトル変換。
                // なので、HLSLでは mul(input.pos, boneTransform) を推奨します。
                cumulativeSkinnedPos += mul(float4(input.pos.xyz, 1.0f), boneTransform) * boneWeight;
                
                // 法線のスキニング
                // 法線は回転成分のみ影響するので、行列の3x3部分のみを使い、必ず正規化する
                cumulativeSkinnedNormal += mul(input.normal.xyz, (float3x3) boneTransform) * boneWeight;
            }
            totalWeight += boneWeight; // 合計ウェイトを計算
        }
    }
    
    float4 finalSkinnedPos;
    float3 finalSkinnedNormal;

    // スキニングが適用されない頂点（ウェイトの合計がほぼゼロ）の場合のフォールバック
    if (totalWeight <= 0.0001f) // ほぼゼロの場合
    {
        finalSkinnedPos = float4(input.pos.xyz, 1.0f);
        finalSkinnedNormal = input.normal.xyz;
    }
    else
    {
        // 各ウェイトで乗算済みなので、ここで totalWeight で割る必要は通常ない
        // （PMXのウェイト合計が1になる前提）
        finalSkinnedPos = cumulativeSkinnedPos;
        finalSkinnedNormal = cumulativeSkinnedNormal;
    }

    // スキニング結果にモデルのワールド行列を適用
    // finalSkinnedPos.xyz がワールド空間の頂点位置
    float3 worldPos = mul(finalSkinnedPos, world).xyz;
    
    // finalSkinnedNormal がワールド空間の法線 (正規化は後でまとめて)
    float3 worldNormal = normalize(mul(finalSkinnedNormal, (float3x3) world));

    // --- ビュー・プロジェクション変換 (クリップ空間座標の計算) ---
    float4 viewPos = mul(float4(worldPos, 1.0f), view);
    output.svpos = mul(viewPos, proj);

    // --- その他の出力 ---
    output.uv = input.uv;
    
    // output.normal と output.vnormal は同じものを使うことが多い
    output.normal = float4(worldNormal, 1.0f); // 正規化済み
    output.vnormal = float4(worldNormal, 0.0f); // スフィアマップ用など、W成分が0であるべき場合

    // 視線ベクトル (ワールド空間の頂点からカメラ位置へ)
    output.ray = normalize(eye - worldPos.xyz);

    // 必要に応じて additionalUVs や edge もパススルー
    // output.additionalUV0 = input.additionalUV0;
    // output.edge = input.edge;

    return output;
}