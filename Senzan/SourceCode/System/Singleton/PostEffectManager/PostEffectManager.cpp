#include "PostEffectManager.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Graphic/Shader/ShaderResource.h"
#include "Graphic/Shader/ShaderCompile.h"


namespace {
    constexpr char PS_FILE_PATH[] = "Data\\Shader\\Monochrome\\MonochromePS.hlsl";
    constexpr char VS_FILE_PATH[] = "Data\\Shader\\Monochrome\\MonochromeVS.hlsl";
    
    // 新規: ブラー用 runtime PS
    constexpr char BLUR_RUNTIME_PS[] = "Data\\Shader\\Blur\\GaussianBlurRuntime.hlsl";
}

// Render an external SRV through post effects and output to backbuffer.
void PostEffectManager::RenderSRVWithPostEffects(ID3D11ShaderResourceView* srcSRV, int srcW, int srcH, bool forceFullGray)
{
    if (!srcSRV) return;
    auto& dx = DirectX11::GetInstance();
    auto ctx = dx.GetContext();

    // We'll render the provided SRV into our blur / accumulation pipeline similar to DrawToBackBuffer.
    // First, if blur is requested, run blur passes using srcSRV as input into m_BlurRTV[0]/[1].
    ID3D11ShaderResourceView* finalSRV = srcSRV;

    if (m_BlurEnabled && m_BlurSRV[0] && m_BlurSRV[1])
    {
        // Unbind to avoid RTV/SRV race
        ID3D11ShaderResourceView* nullsrvs[8] = { nullptr };
        ctx->PSSetShaderResources(0, 8, nullsrvs);

        // Horizontal pass -> m_BlurRTV[0]
        ctx->OMSetRenderTargets(1, &m_BlurRTV[0], nullptr);
        ctx->IASetInputLayout(nullptr);
        ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        ctx->VSSetShader(m_pVertexShader->GetVertexShader(), nullptr, 0);
        ctx->PSSetShader(m_pBlurPixelShader->GetPixelShader(), nullptr, 0);

        float texW = (float)srcW / std::max(1.0f, m_BlurRadiusFactor);
        float texH = (float)srcH / std::max(1.0f, m_BlurRadiusFactor);
        UpdateBlurCB(texW, texH, true);
        ctx->PSSetConstantBuffers(0, 1, &m_BlurCB);

        ctx->PSSetShaderResources(0, 1, &srcSRV);
        ctx->PSSetSamplers(0, 1, &m_Sampler);
        ctx->Draw(4, 0);

        // Vertical pass -> m_BlurRTV[1]
        ctx->PSSetShaderResources(0, 8, nullsrvs);
        ctx->OMSetRenderTargets(1, &m_BlurRTV[1], nullptr);
        ctx->PSSetShader(m_pBlurPixelShader->GetPixelShader(), nullptr, 0);

        UpdateBlurCB(texW, texH, false);
        ctx->PSSetConstantBuffers(0, 1, &m_BlurCB);

        ID3D11ShaderResourceView* srv0 = m_BlurSRV[0];
        ctx->PSSetShaderResources(0, 1, &srv0);
        ctx->PSSetSamplers(0, 1, &m_Sampler);
        ctx->Draw(4, 0);

        finalSRV = m_BlurSRV[1];
        ctx->PSSetShaderResources(0, 8, nullsrvs);
    }

    // Motion blur accumulation if enabled
    if (m_MotionBlurEnabled && m_AccumRTV && m_AccumSRV && m_pBlendPixelShader)
    {
        if (!m_IsAccumInitialized)
        {
            ctx->OMSetRenderTargets(1, &m_AccumRTV, nullptr);
            ctx->IASetInputLayout(nullptr);
            ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
            ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            ctx->VSSetShader(m_pVertexShader->GetVertexShader(), nullptr, 0);
            ctx->PSSetShader(m_pPixelShader->GetPixelShader(), nullptr, 0);
            ctx->PSSetShaderResources(0, 1, &m_SceneSRV); // use current scene SRV as base
            ctx->PSSetSamplers(0, 1, &m_Sampler);
            ctx->Draw(4, 0);
            m_IsAccumInitialized = true;
        }
        else
        {
            ID3D11ShaderResourceView* prevSRV = m_AccumSRV;
            ctx->OMSetRenderTargets(1, &m_AccumRTV, nullptr);
            ctx->IASetInputLayout(nullptr);
            ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
            ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            ctx->VSSetShader(m_pVertexShader->GetVertexShader(), nullptr, 0);
            ctx->PSSetShader(m_pBlendPixelShader->GetPixelShader(), nullptr, 0);

            D3D11_MAPPED_SUBRESOURCE mapped;
            if (SUCCEEDED(ctx->Map(m_BlendCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            {
                float* v = static_cast<float*>(mapped.pData);
                v[0] = m_MotionBlurAmount;
                v[1] = v[2] = v[3] = 0.0f;
                ctx->Unmap(m_BlendCB, 0);
            }
            ctx->PSSetConstantBuffers(1, 1, &m_BlendCB);

            ctx->PSSetShaderResources(0, 1, &finalSRV);
            ctx->PSSetShaderResources(1, 1, &prevSRV);
            ctx->PSSetSamplers(0, 1, &m_Sampler);
            ctx->Draw(4, 0);

            ID3D11ShaderResourceView* nullsrvs[2] = { nullptr, nullptr };
            ctx->PSSetShaderResources(0, 2, nullsrvs);

            finalSRV = m_AccumSRV;
        }
    }

    // Finally, output finalSRV to backbuffer
    ID3D11RenderTargetView* rtv = dx.GetBackBufferRTV();
    ctx->OMSetRenderTargets(1, &rtv, dx.GetBackBufferDSV());

    ctx->IASetInputLayout(nullptr);
    ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    ctx->VSSetShader(m_pVertexShader->GetVertexShader(), nullptr, 0);
    ctx->PSSetShader(m_pPixelShader->GetPixelShader(), nullptr, 0);

    // If caller requested a forced full-screen gray or global grayscale flag is enabled,
    // temporarily force the circle-gray CB to cover the whole screen so the monochrome shader outputs gray.
    bool prevCircleActive = m_CircleEffectActive;
    float prevCircleRadius = m_CircleRadius;
    bool prevIsExpanding = m_IsExpanding;
    bool changedCB = false;
    if (forceFullGray || m_IsGray)
    {
        m_CircleEffectActive = true;
        m_IsExpanding = true; // ensure shader uses expanding branch (gray inside)
        // large radius ensures full-screen effect regardless of aspect
        m_CircleRadius = 1000.0f;
        changedCB = true;
    }

    UpdateConstantBuffer();
    ctx->PSSetConstantBuffers(0, 1, &m_CircleGrayCB);

    // Debug log: report gray/force state and circle radius
    {
        std::stringstream ss;
        ss << "PostEffect: RenderSRVWithPostEffects forceFullGray=" << (forceFullGray?1:0)
           << " m_IsGray=" << (m_IsGray?1:0)
           << " CircleActive=" << (m_CircleEffectActive?1:0)
           << " CircleRadius=" << m_CircleRadius;
        Log::GetInstance().LogInfo(ss.str());
    }

    ctx->PSSetShaderResources(0, 1, &finalSRV);
    ctx->PSSetSamplers(0, 1, &m_Sampler);
    ctx->Draw(4, 0);

    // restore circle effect state if we modified it
    if (changedCB)
    {
        m_CircleEffectActive = prevCircleActive;
        m_CircleRadius = prevCircleRadius;
        m_IsExpanding = prevIsExpanding;
    }

    ID3D11ShaderResourceView* nullSRV = nullptr;
    ctx->PSSetShaderResources(0, 1, &nullSRV);
}

//-------------------------------------------------------------------------------------------------------------------------------------

PostEffectManager::PostEffectManager()
   : m_pPixelShader(std::make_unique<PixelShaderBase>())
   , m_pVertexShader(std::make_unique<VertexShaderBase>())
   , m_SceneRTV         (nullptr)
   , m_SceneSRV         (nullptr)
   , m_FullscreenVB     (nullptr)
   , m_IsGray           (false)
   , m_Sampler          (nullptr)
   , m_SceneDSMSAA      (nullptr)
   , m_SceneDSV         (nullptr)
   , m_SceneMSAATex     (nullptr)
   , m_SceneResolvedTex (nullptr)
   , m_CircleGrayCB     (nullptr)
   , m_CircleEffectActive(false)
   , m_CircleRadius     (0.0f)
   , m_IsExpanding      (true)
   , m_ExpandDuration   (0.3f)
   , m_HoldDuration     (0.5f)
   , m_ShrinkDuration   (0.3f)
   , m_EffectTimer      (0.0f)
   , m_EffectPhase      (0)
   , m_pBlurPixelShader(std::make_unique<PixelShaderBase>())
   , m_BlurCB(nullptr)
   , m_BlurEnabled(false)
   , m_BlurPasses(1)
    , m_BlurRadiusFactor(2.0f)
    , m_AccumTex(nullptr)
    , m_AccumRTV(nullptr)
    , m_AccumSRV(nullptr)
    , m_pBlendPixelShader(std::make_unique<PixelShaderBase>())
    , m_BlendCB(nullptr)
    , m_MotionBlurEnabled(false)
    , m_MotionBlurAmount(0.5f)
    , m_IsAccumInitialized(false)
{
    m_BlurTex[0] = m_BlurTex[1] = nullptr;
    m_BlurRTV[0] = m_BlurRTV[1] = nullptr;
    m_BlurSRV[0] = m_BlurSRV[1] = nullptr;
}

PostEffectManager::~PostEffectManager()
{
    SAFE_RELEASE(m_SceneRTV);
    SAFE_RELEASE(m_SceneSRV);
    SAFE_RELEASE(m_Sampler);
    SAFE_RELEASE(m_FullscreenVB);
    SAFE_RELEASE(m_CircleGrayCB);
    for (int i = 0; i < 2; ++i)
    {
        SAFE_RELEASE(m_BlurRTV[i]);
        SAFE_RELEASE(m_BlurSRV[i]);
        SAFE_RELEASE(m_BlurTex[i]);
    }
    SAFE_RELEASE(m_BlurCB);
    SAFE_RELEASE(m_AccumRTV);
    SAFE_RELEASE(m_AccumSRV);
    SAFE_RELEASE(m_AccumTex);
    SAFE_RELEASE(m_BlendCB);
}

//-------------------------------------------------------------------------------------------------------------------------------------

void PostEffectManager::Initialize()
{
    auto dev = DirectX11::GetInstance().GetDevice();

    // 1. レンダリング用バッファ (MSAAあり).
    D3D11_TEXTURE2D_DESC td{};

    td.Width = WND_W;
    td.Height = WND_H;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 8;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET; // SRVはここでは付けない(MSAAのため).
    dev->CreateTexture2D(&td, nullptr, &m_SceneMSAATex);
    dev->CreateRenderTargetView(m_SceneMSAATex, nullptr, &m_SceneRTV);

    // 2. Resolvedバッファ (サンプリング用 / MSAAなし).
    td.SampleDesc.Count = 1;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    dev->CreateTexture2D(&td, nullptr, &m_SceneResolvedTex);
    dev->CreateShaderResourceView(m_SceneResolvedTex, nullptr, &m_SceneSRV);

    // 3. 深度バッファ (MSAA数を合わせる).
    D3D11_TEXTURE2D_DESC dsd{};
    dsd.Width = WND_W;
    dsd.Height = WND_H;
    dsd.MipLevels = 1;
    dsd.ArraySize = 1;
    dsd.Format = DXGI_FORMAT_D32_FLOAT;
    dsd.SampleDesc.Count = 8; // RTVと一致させる.
    dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    dev->CreateTexture2D(&dsd, nullptr, &m_SceneDSMSAA);
    dev->CreateDepthStencilView(m_SceneDSMSAA, nullptr, &m_SceneDSV);

    // 4. その他リソース (Sampler, VB).
    D3D11_SAMPLER_DESC sdesc{};
    sdesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sdesc.AddressU = sdesc.AddressV = sdesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    dev->CreateSamplerState(&sdesc, &m_Sampler);

    ID3DBlob* pBlob = nullptr;
    ID3DBlob* pErrorBlob = nullptr;

    // 頂点インプットレイアウトを定義.
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // VSの作成.
    ShaderCompile(new std::string(VS_FILE_PATH), "main", "vs_5_0", pBlob, pErrorBlob);
    m_pVertexShader->ConfigInputLayout(layout);
    m_pVertexShader->Init(pBlob);
    SAFE_RELEASE(pErrorBlob);


    // PSの作成.
    ShaderCompile(new std::string(PS_FILE_PATH), "main", "ps_5_0", pBlob, pErrorBlob);
    m_pPixelShader->Init(pBlob);
    SAFE_RELEASE(pErrorBlob);

    // ブラー用 PS を runtime HLSL からコンパイル
    ID3DBlob* pBlurBlob = nullptr;
    ID3DBlob* pBlurErr = nullptr;
    ShaderCompile(new std::string(BLUR_RUNTIME_PS), "PSMain", "ps_5_0", pBlurBlob, pBlurErr);
    m_pBlurPixelShader->Init(pBlurBlob);
    SAFE_RELEASE(pBlurBlob);
    SAFE_RELEASE(pBlurErr);

    // 定数バッファの作成
    D3D11_BUFFER_DESC cbDesc{};
    cbDesc.ByteWidth = sizeof(CircleGrayBuffer);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    dev->CreateBuffer(&cbDesc, nullptr, &m_CircleGrayCB);

    // ブラー用 CB
    D3D11_BUFFER_DESC blurCbDesc{};
    blurCbDesc.ByteWidth = sizeof(BlurCB);
    blurCbDesc.Usage = D3D11_USAGE_DYNAMIC;
    blurCbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    blurCbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    dev->CreateBuffer(&blurCbDesc, nullptr, &m_BlurCB);

    // ブラー用 ping-pong テクスチャ
    D3D11_TEXTURE2D_DESC blurTd = {};
    blurTd.Width = WND_W;
    blurTd.Height = WND_H;
    blurTd.MipLevels = 1;
    blurTd.ArraySize = 1;
    blurTd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    blurTd.SampleDesc.Count = 1;
    blurTd.Usage = D3D11_USAGE_DEFAULT;
    blurTd.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

    for (int i = 0; i < 2; ++i)
    {
        dev->CreateTexture2D(&blurTd, nullptr, &m_BlurTex[i]);
        dev->CreateRenderTargetView(m_BlurTex[i], nullptr, &m_BlurRTV[i]);
        dev->CreateShaderResourceView(m_BlurTex[i], nullptr, &m_BlurSRV[i]);
    }

    // accumulation target for simple motion blur
    D3D11_TEXTURE2D_DESC accTd = {};
    accTd.Width = WND_W;
    accTd.Height = WND_H;
    accTd.MipLevels = 1;
    accTd.ArraySize = 1;
    accTd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    accTd.SampleDesc.Count = 1;
    accTd.Usage = D3D11_USAGE_DEFAULT;
    accTd.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    dev->CreateTexture2D(&accTd, nullptr, &m_AccumTex);
    dev->CreateRenderTargetView(m_AccumTex, nullptr, &m_AccumRTV);
    dev->CreateShaderResourceView(m_AccumTex, nullptr, &m_AccumSRV);

    // blend pixel shader (simple lerp) - compile runtime same as blur
    ID3DBlob* pBlendBlob = nullptr;
    ID3DBlob* pBlendErr = nullptr;
    // reusing GaussianBlurRuntime.hlsl is fine if entrypoint provided; otherwise create simple file
    ShaderCompile(new std::string(BLUR_RUNTIME_PS), "PSMain", "ps_5_0", pBlendBlob, pBlendErr);
    m_pBlendPixelShader->Init(pBlendBlob);
    SAFE_RELEASE(pBlendBlob);
    SAFE_RELEASE(pBlendErr);

    // blend CB
    D3D11_BUFFER_DESC blendCbDesc{};
    blendCbDesc.ByteWidth = sizeof(float) * 4; // padding
    blendCbDesc.Usage = D3D11_USAGE_DYNAMIC;
    blendCbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    blendCbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    dev->CreateBuffer(&blendCbDesc, nullptr, &m_BlendCB);
}

//-------------------------------------------------------------------------------------------------------------------------------------

void PostEffectManager::BeginSceneRender()  
{
    auto ctx = DirectX11::GetInstance().GetContext();

    // 書き込み競合回避.
    ID3D11ShaderResourceView* nullSRV = nullptr;
    ctx->PSSetShaderResources(0, 1, &nullSRV);

    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ctx->ClearRenderTargetView(m_SceneRTV, clearColor);
    ctx->ClearDepthStencilView(m_SceneDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    ctx->OMSetRenderTargets(1, &m_SceneRTV, m_SceneDSV);

    auto& dx = DirectX11::GetInstance();
    dx.SetDepth(true);
    dx.SetAlphaBlend(true);
}

//-------------------------------------------------------------------------------------------------------------------------------------

void PostEffectManager::DrawToBackBuffer()
{
    auto& dx = DirectX11::GetInstance();
    auto ctx = dx.GetContext();

    // 【重要】MSAAバッファをResolvedバッファに集約.
    ctx->ResolveSubresource(m_SceneResolvedTex, 0, m_SceneMSAATex, 0, DXGI_FORMAT_R8G8B8A8_UNORM);

    // ブラーが有効なら ping-pong で 2 パス（横→縦）
    ID3D11ShaderResourceView* finalSRV = m_SceneSRV;

    if (m_BlurEnabled && m_BlurSRV[0] && m_BlurSRV[1])
    {
        // Unbind shader resources slots to avoid RTV/SRV race
        ID3D11ShaderResourceView* nullsrvs[8] = { nullptr };
        ctx->PSSetShaderResources(0, 8, nullsrvs);

        // 1) 横 -> m_BlurRTV[0]
        ctx->OMSetRenderTargets(1, &m_BlurRTV[0], nullptr);
        ctx->IASetInputLayout(nullptr);
        ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        ctx->VSSetShader(m_pVertexShader->GetVertexShader(), nullptr, 0);
        ctx->PSSetShader(m_pBlurPixelShader->GetPixelShader(), nullptr, 0);

        // use radius factor to exaggerate offsets
        float texW = (float)WND_W / std::max(1.0f, m_BlurRadiusFactor);
        float texH = (float)WND_H / std::max(1.0f, m_BlurRadiusFactor);
        UpdateBlurCB(texW, texH, true);
        ctx->PSSetConstantBuffers(0, 1, &m_BlurCB);

        ctx->PSSetShaderResources(0, 1, &m_SceneSRV);
        ctx->PSSetSamplers(0, 1, &m_Sampler);
        ctx->Draw(4, 0);

        // 2) 縦 -> m_BlurRTV[1]
        ctx->PSSetShaderResources(0, 8, nullsrvs);
        ctx->OMSetRenderTargets(1, &m_BlurRTV[1], nullptr);
        ctx->PSSetShader(m_pBlurPixelShader->GetPixelShader(), nullptr, 0);

        UpdateBlurCB(texW, texH, false);
        ctx->PSSetConstantBuffers(0, 1, &m_BlurCB);

        ID3D11ShaderResourceView* srv0 = m_BlurSRV[0];
        ctx->PSSetShaderResources(0, 1, &srv0);
        ctx->PSSetSamplers(0, 1, &m_Sampler);
        ctx->Draw(4, 0);

        finalSRV = m_BlurSRV[1];

        ctx->PSSetShaderResources(0, 8, nullsrvs);
    }

    // 簡易モーションブラー: accumulation を行う
    if (m_MotionBlurEnabled && m_AccumRTV && m_AccumSRV && m_pBlendPixelShader)
    {
        // if accumulation not initialized, copy current resolved into accumulation
        if (!m_IsAccumInitialized)
        {
            ctx->OMSetRenderTargets(1, &m_AccumRTV, nullptr);
            ctx->IASetInputLayout(nullptr);
            ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
            ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            ctx->VSSetShader(m_pVertexShader->GetVertexShader(), nullptr, 0);
            ctx->PSSetShader(m_pPixelShader->GetPixelShader(), nullptr, 0);
            ctx->PSSetShaderResources(0, 1, &m_SceneSRV);
            ctx->PSSetSamplers(0, 1, &m_Sampler);
            ctx->Draw(4, 0);
            m_IsAccumInitialized = true;
        }
        else
        {
            // Blend: render to accumulation using PSBlend (prev in t1)
            ID3D11ShaderResourceView* prevSRV = m_AccumSRV;
            ctx->OMSetRenderTargets(1, &m_AccumRTV, nullptr);
            ctx->IASetInputLayout(nullptr);
            ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
            ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            ctx->VSSetShader(m_pVertexShader->GetVertexShader(), nullptr, 0);
            ctx->PSSetShader(m_pBlendPixelShader->GetPixelShader(), nullptr, 0);

            // set CB blend factor
            D3D11_MAPPED_SUBRESOURCE mapped;
            if (SUCCEEDED(ctx->Map(m_BlendCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            {
                float* v = static_cast<float*>(mapped.pData);
                v[0] = m_MotionBlurAmount;
                v[1] = v[2] = v[3] = 0.0f;
                ctx->Unmap(m_BlendCB, 0);
            }
            ctx->PSSetConstantBuffers(1, 1, &m_BlendCB);

            // current is in t0, prev in t1
            ctx->PSSetShaderResources(0, 1, &m_SceneSRV);
            ctx->PSSetShaderResources(1, 1, &prevSRV);
            ctx->PSSetSamplers(0, 1, &m_Sampler);
            ctx->Draw(4, 0);

            // unbind prev
            ID3D11ShaderResourceView* nullsrvs[2] = { nullptr, nullptr };
            ctx->PSSetShaderResources(0, 2, nullsrvs);
        }

        // use accumulation as final SRV
        finalSRV = m_AccumSRV;
    }

    // ターゲットをバックバッファへ戻す.
    ID3D11RenderTargetView* rtv = dx.GetBackBufferRTV();
    ctx->OMSetRenderTargets(1, &rtv, dx.GetBackBufferDSV());

    // 描画ステート設定.
    ctx->IASetInputLayout(nullptr);
    ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    ctx->VSSetShader(m_pVertexShader->GetVertexShader(), nullptr, 0);
    ctx->PSSetShader(m_pPixelShader->GetPixelShader(), nullptr, 0);

    // 定数バッファを更新してセット
    UpdateConstantBuffer();
    ctx->PSSetConstantBuffers(0, 1, &m_CircleGrayCB);

    // Resolved または ブラー後のテクスチャを渡す.
    ctx->PSSetShaderResources(0, 1, &finalSRV);
    ctx->PSSetSamplers(0, 1, &m_Sampler);

    ctx->Draw(4, 0);

    // 後片付け.
    ID3D11ShaderResourceView* nullSRV = nullptr;
    ctx->PSSetShaderResources(0, 1, &nullSRV);
}

//-------------------------------------------------------------------------------------------------------------------------------------

void PostEffectManager::SetGray(bool enable)
{
    m_IsGray = enable;
}

bool PostEffectManager::IsGray() const
{
    return m_IsGray;
}

//-------------------------------------------------------------------------------------------------------------------------------------

void PostEffectManager::Update(float deltaTime)
{
    if (!m_CircleEffectActive) return;

    m_EffectTimer += deltaTime;

    switch (m_EffectPhase)
    {
    case 0: // 広がりフェーズ
        m_IsExpanding = true;
        m_CircleRadius = (m_EffectTimer / m_ExpandDuration) * 1.5f;  // 0 -> 1.5
        if (m_EffectTimer >= m_ExpandDuration)
        {
            m_CircleRadius = 1.5f;
            m_EffectTimer = 0.0f;
            m_EffectPhase = 1;
        }
        break;

    case 1: // 維持フェーズ
        m_CircleRadius = 1.5f;
        if (m_EffectTimer >= m_HoldDuration)
        {
            m_EffectTimer = 0.0f;
            m_EffectPhase = 2;
        }
        break;

    case 2: // 戻りフェーズ（中心から通常色が広がる）
        m_IsExpanding = false;
        m_CircleRadius = (m_EffectTimer / m_ShrinkDuration) * 1.5f;  // 0 -> 1.5（中心から広がる）
        if (m_EffectTimer >= m_ShrinkDuration)
        {
            m_CircleRadius = 1.5f;
            m_CircleEffectActive = false;
            m_EffectPhase = 0;
            m_EffectTimer = 0.0f;
        }
        break;
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------

void PostEffectManager::StartCircleGrayEffect(float expandDuration, float holdDuration, float shrinkDuration)
{
    m_CircleEffectActive = true;
    m_CircleRadius = 0.0f;
    m_IsExpanding = true;
    m_ExpandDuration = expandDuration;
    m_HoldDuration = holdDuration;
    m_ShrinkDuration = shrinkDuration;
    m_EffectTimer = 0.0f;
    m_EffectPhase = 0;
}

//-------------------------------------------------------------------------------------------------------------------------------------

void PostEffectManager::UpdateConstantBuffer()
{
    auto ctx = DirectX11::GetInstance().GetContext();

    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(ctx->Map(m_CircleGrayCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        CircleGrayBuffer* cb = static_cast<CircleGrayBuffer*>(mapped.pData);
        cb->CircleRadius = m_CircleRadius;
        cb->IsExpanding = m_IsExpanding ? 1.0f : 0.0f;
        cb->EffectActive = m_CircleEffectActive ? 1.0f : 0.0f;
        cb->AspectRatio = static_cast<float>(WND_W) / static_cast<float>(WND_H);
        ctx->Unmap(m_CircleGrayCB, 0);
    }
}

//-------------------------------------------------------------------------------------------------------------------------------------

void PostEffectManager::UpdateBlurCB(float texW, float texH, bool horizontal)
{
    auto ctx = DirectX11::GetInstance().GetContext();
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(ctx->Map(m_BlurCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        BlurCB* cb = static_cast<BlurCB*>(mapped.pData);
        cb->TexelWidth = 1.0f / texW;
        cb->TexelHeight = 1.0f / texH;
        cb->Horizontal = horizontal ? 1 : 0;
        cb->Padding = 0.0f;
        ctx->Unmap(m_BlurCB, 0);
    }
}
