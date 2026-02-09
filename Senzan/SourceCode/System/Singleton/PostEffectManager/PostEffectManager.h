#pragma once
#pragma once
#include <wrl/client.h> // Microsoft::WRL::ComPtr 用
#include <memory>
#include <algorithm>

class PixelShaderBase;
class VertexShaderBase;

class PostEffectManager final : public Singleton<PostEffectManager>
{
private:
    friend class Singleton<PostEffectManager>;
    PostEffectManager();
public:
    ~PostEffectManager();

    void Initialize();
    void BeginSceneRender();
    void DrawToBackBuffer();
    void Update(float deltaTime);  // 毎フレーム呼び出し

    void SetGray(bool enable);
    bool IsGray() const;

    // 円状グレースケールエフェクト開始
    void StartCircleGrayEffect(float expandDuration = 0.3f, float holdDuration = 0.5f, float shrinkDuration = 0.3f);
    bool IsCircleGrayActive() const { return m_CircleEffectActive; }

    // 背景テクスチャ（Resolve済み）を取得（Effekseer歪み用）
    ID3D11ShaderResourceView* GetSceneSRV() const { return m_SceneSRV; }

    // 内部テクスチャへのアクセス（Resolve用）
    ID3D11Texture2D* GetSceneMSAATex() const { return m_SceneMSAATex; }
    ID3D11Texture2D* GetSceneResolvedTex() const { return m_SceneResolvedTex; }

    // ブラー制御
    void SetBlurEnabled(bool enable) { m_BlurEnabled = enable; }
    bool IsBlurEnabled() const { return m_BlurEnabled; }
    void SetBlurPasses(int p) { m_BlurPasses = std::max(1, p); }
    int GetBlurPasses() const { return m_BlurPasses; }

    // 簡易モーションブラー（フレーム蓄積）API
    void SetMotionBlurEnabled(bool enable) { m_MotionBlurEnabled = enable; }
    bool IsMotionBlurEnabled() const { return m_MotionBlurEnabled; }
    void SetMotionBlurAmount(float a) { m_MotionBlurAmount = a; }
    float GetMotionBlurAmount() const { return m_MotionBlurAmount; }

    // Render an arbitrary SRV through the post-effect pipeline and output to backbuffer.
    // srcW/srcH are the source SRV dimensions and are used for blur/motion blur texel calculations.
    void RenderSRVWithPostEffects(ID3D11ShaderResourceView* srcSRV, int srcW, int srcH, bool forceFullGray = false);

private:
    // シェーダー用定数バッファ構造体
    struct CircleGrayBuffer
    {
        float CircleRadius;
        float IsExpanding;
        float EffectActive;
        float AspectRatio;
    };

    // ブラー用定数バッファ
    struct BlurCB
    {
        float TexelWidth;
        float TexelHeight;
        int   Horizontal; // 1: 横, 0: 縦
        float Padding;
    };

    void UpdateBlurCB(float texW, float texH, bool horizontal);

    void UpdateConstantBuffer();

private:
    ID3D11Texture2D* m_SceneMSAATex;
    ID3D11Texture2D* m_SceneResolvedTex;
    ID3D11Texture2D* m_SceneDSMSAA;
    ID3D11RenderTargetView* m_SceneRTV;
    ID3D11DepthStencilView* m_SceneDSV;
    ID3D11ShaderResourceView* m_SceneSRV;

    ID3D11SamplerState* m_Sampler;
    ID3D11Buffer* m_FullscreenVB;
    ID3D11Buffer* m_CircleGrayCB;  // 定数バッファ

    std::unique_ptr<PixelShaderBase>  m_pPixelShader;
    std::unique_ptr<VertexShaderBase> m_pVertexShader;

    // ブラー用 ping-pong (2)
    ID3D11Texture2D* m_BlurTex[2];
    ID3D11RenderTargetView* m_BlurRTV[2];
    ID3D11ShaderResourceView* m_BlurSRV[2];

    // ブラー用ピクセルシェーダ & CB
    std::unique_ptr<PixelShaderBase> m_pBlurPixelShader;
    ID3D11Buffer* m_BlurCB;

    bool m_IsGray;

    // ブラー設定
    bool m_BlurEnabled;
    int  m_BlurPasses;
    float m_BlurRadiusFactor;

private:
    // accumulation target
    ID3D11Texture2D* m_AccumTex;
    ID3D11RenderTargetView* m_AccumRTV;
    ID3D11ShaderResourceView* m_AccumSRV;

    std::unique_ptr<PixelShaderBase> m_pBlendPixelShader;
    ID3D11Buffer* m_BlendCB; // contains blend factor
    bool m_MotionBlurEnabled;
    float m_MotionBlurAmount; // 0..1 (0 = no trail, 1 = full previous)
    bool m_IsAccumInitialized;

    // 円状エフェクト用変数
    bool  m_CircleEffectActive;
    float m_CircleRadius;
    bool  m_IsExpanding;
    float m_ExpandDuration;
    float m_HoldDuration;
    float m_ShrinkDuration;
    float m_EffectTimer;
    int   m_EffectPhase;  // 0: 広がり, 1: 維持, 2: 戻り
};
