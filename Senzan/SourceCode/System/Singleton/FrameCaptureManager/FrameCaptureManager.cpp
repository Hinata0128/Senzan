#include "FrameCaptureManager.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Game/05_InputDevice/Input.h"
#include "Game/04_Time/Time.h"
#include "System/Singleton/PostEffectManager/PostEffectManager.h"

#include <d3dcompiler.h>
#include <fstream>

#pragma comment(lib, "d3dcompiler.lib")

// FrameCaptureManager の ImGui を Release でも有効にする場合は 1 にする
#define ENABLE_FRAMECAPTURE_IMGUI 0

#if _DEBUG || ENABLE_FRAMECAPTURE_IMGUI
#include "System/Singleton/ImGui/CImGuiManager.h"
#endif

// FileManagerを利用してjson読み書き
#include "System/Utility/FileManager/FileManager.h"
// ログ出力
#include "System/Singleton/Debug/Log/DebugLog.h"
// シーンマネージャ
#include "System/Singleton/SceneManager/SceneManager.h"

namespace
{
	// フルスクリーンクワッド頂点構造体
	struct FullscreenVertex
	{
		float x, y, z;
		float u, v;
	};

	// フルスクリーンクワッド頂点データ（2つの三角形）
	const FullscreenVertex g_FullscreenQuadVertices[] =
	{
		// 左上三角形
		{ -1.0f,  1.0f, 0.0f, 0.0f, 0.0f },
		{  1.0f,  1.0f, 0.0f, 1.0f, 0.0f },
		{ -1.0f, -1.0f, 0.0f, 0.0f, 1.0f },
		// 右下三角形
		{  1.0f,  1.0f, 0.0f, 1.0f, 0.0f },
		{  1.0f, -1.0f, 0.0f, 1.0f, 1.0f },
		{ -1.0f, -1.0f, 0.0f, 0.0f, 1.0f },
	};

	// 頂点シェーダー（インラインHLSL）
	const char* g_VS_Source = R"(
		struct VS_INPUT
		{
			float3 Pos : POSITION;
			float2 Tex : TEXCOORD0;
		};
		struct PS_INPUT
		{
			float4 Pos : SV_POSITION;
			float2 Tex : TEXCOORD0;
		};
		PS_INPUT main(VS_INPUT input)
		{
			PS_INPUT output;
			output.Pos = float4(input.Pos, 1.0f);
			output.Tex = input.Tex;
			return output;
		}
	)";

	// ピクセルシェーダー（インラインHLSL）
	const char* g_PS_Source = R"(
	Texture2D g_Texture : register(t0);
	SamplerState g_Sampler : register(s0);
	struct PS_INPUT
	{
		float4 Pos : SV_POSITION;
		float2 Tex : TEXCOORD0;
	};
	float4 main(PS_INPUT input) : SV_Target
	{
		float4 c = g_Texture.Sample(g_Sampler, input.Tex);
		return float4(c.rgb, 1.0f);
	}
)";

	// 巻き戻し用ピクセルシェーダー（歪み + クロマティックアベレーション + グレースケール）
	const char* g_PS_Rewind_Source = R"(
	Texture2D g_Texture : register(t0);
	SamplerState g_Sampler : register(s0);
	cbuffer RewindCB : register(b0)
	{
		float Time;
		float Intensity;
		float Chromatic;
		float Grayscale; // グレースケール強度 (0.0 = カラー, 1.0 = 完全白黒)
	};
	struct PS_INPUT
	{
		float4 Pos : SV_POSITION;
		float2 Tex : TEXCOORD0;
	};

	float2 CurlWarp(float2 uv, float t, float intensity)
	{
		float2 center = uv - 0.5;
		float len = length(center);
		float angle = atan2(center.y, center.x);
		float offset = sin(len * 10.0 - t * 8.0) * 0.012 * intensity * (1.0 + 3.0 * len);
		float2 result = uv + offset * float2(cos(angle), sin(angle));
		return result;
	}

	float4 main(PS_INPUT input) : SV_Target
	{
		float2 uv = input.Tex;

		float2 uvR = CurlWarp(uv, Time, Intensity) + Chromatic * float2(0.003, 0.0);
		float2 uvG = CurlWarp(uv, Time * 1.1, Intensity * 0.85);
		float2 uvB = CurlWarp(uv, Time * 0.9, Intensity * 0.7) - Chromatic * float2(0.003, 0.0);

		float r = g_Texture.Sample(g_Sampler, uvR).r;
		float g = g_Texture.Sample(g_Sampler, uvG).g;
		float b = g_Texture.Sample(g_Sampler, uvB).b;

		float3 col = float3(r, g, b);

		// vignette（周辺減光）
		float vign = smoothstep(0.9, 0.4, length(uv - 0.5));
		col *= lerp(1.0, 0.6, vign * Intensity);

		// 軽い暗転で戻る感を演出
		col *= lerp(0.7, 1.0, 1.0 - Intensity * 0.5);

		// グレースケール変換（輝度計算）
		float luma = dot(col, float3(0.299, 0.587, 0.114));
		col = lerp(col, float3(luma, luma, luma), Grayscale);

		return float4(col, 1.0);
	}
)";
}

// コンストラクタ
FrameCaptureManager::FrameCaptureManager()
	: m_CaptureDuration(5.0f)
	, m_CaptureFPS(60)
	, m_MaxFrames(0)
	, m_bCapturing(false)
	, m_CaptureTimer(0.0f)
	, m_FrameInterval(1.0f / 30.0f)
	, m_FrameAccumulator(0.0f)
	, m_WriteIndex(0)
	, m_CapturedFrameCount(0)
	, m_bPlaying(false)
	, m_bLoopPlayback(true)
	, m_PlaybackIndex(0)
	, m_PlaybackAccumulator(0.0f)
	, m_IsPlaybackTriggerKey()
	, m_pFullscreenVB(nullptr)
	, m_pSamplerState(nullptr)
	, m_pVertexShader(nullptr)
	, m_pPixelShader(nullptr)
	, m_pRewindPixelShader(nullptr)
	, m_pRewindCB(nullptr)
	, m_pInputLayout(nullptr)
	, m_bInitialized(false)
    , m_bRolling(false)
    , m_bRollingPaused(false)
    , m_bRewindMode(false)
    , m_bReloadOnComplete(false)
    , m_bRequestSceneReload(false)
    , m_SampleIntervalFrames(30)
    , m_AssumedFPS(60)
    , m_FrameCounter(0)
    , m_DownsampleFactor(1)
    , m_PlaybackIntervalBackup(0.0f)
    , m_RewindTime(0.0f)
    , m_RewindIntensity(0.25f)
    , m_RewindChromatic(1.0f)
    , m_RewindGrayscale(1.0f)  // 完全白黒
{
}

// デストラクタ
FrameCaptureManager::~FrameCaptureManager()
{
	Release();
}

// 初期化
void FrameCaptureManager::Initialize()
{
	if (m_bInitialized) return;

	LoadSettings();
	CreateFullscreenQuadResources();

	m_bInitialized = true;
}

// 解放
void FrameCaptureManager::Release()
{
	StopCapture();
	StopPlayback();
	ReleaseCaptureTextures();
	ReleaseFullscreenQuadResources();
	m_bInitialized = false;
}

// キャプチャ開始
void FrameCaptureManager::StartCapture(float seconds, int fps)
{
	if (m_bCapturing || m_bPlaying) return;

	m_CaptureDuration = seconds;
	m_CaptureFPS = fps;
	m_MaxFrames = static_cast<int>(m_CaptureDuration * m_CaptureFPS);
	m_FrameInterval = 1.0f / static_cast<float>(m_CaptureFPS);

	// 既存のテクスチャを解放してから再作成
	ReleaseCaptureTextures();
	CreateCaptureTextures();

	m_CaptureTimer = 0.0f;
	m_FrameAccumulator = 0.0f;
	m_WriteIndex = 0;
	m_CapturedFrameCount = 0;
	m_bCapturing = true;
}

// 常時ロールバッファキャプチャ開始
void FrameCaptureManager::StartRollingCapture(int sampleIntervalFrames, int assumedFPS)
{
    if (m_bRolling) return;
    m_bRolling = true;

    m_SampleIntervalFrames = sampleIntervalFrames;
    m_AssumedFPS = assumedFPS;

    // ダウンサンプリング因子はコンストラクタのデフォルト（m_DownsampleFactor）を使用する

    // バッファサイズ = 3分 * assumedFPS / sampleIntervalFrames + 10
    m_MaxFrames = (3 * 60 * m_AssumedFPS) / std::max(1, m_SampleIntervalFrames) + 10;

    // キャプチャ間隔はサンプル間隔に基づき計算（フレームごとのアキュムレータは still Time-driven）
    // m_FrameInterval は秒単位の目安にしておく（assumedFPS を使用）
    m_FrameInterval = static_cast<float>(m_SampleIntervalFrames) / static_cast<float>(m_AssumedFPS);

    // 既存のテクスチャを解放してから再作成
    ReleaseCaptureTextures();
    CreateCaptureTextures();

    m_CaptureTimer = 0.0f;
    m_FrameAccumulator = 0.0f;
    m_WriteIndex = 0;
    m_CapturedFrameCount = 0;
    m_FrameCounter = 0;

#if _DEBUG
    {
        std::stringstream ss;
        ss << "FrameCapture: StartRolling sampleInterval=" << m_SampleIntervalFrames
           << " assumedFPS=" << m_AssumedFPS
           << " maxFrames=" << m_MaxFrames
           << " downsample=" << m_DownsampleFactor;
        Log::GetInstance().LogInfo(ss.str());
    }
#endif
}

// キャプチャ停止
void FrameCaptureManager::StopCapture()
{
	m_bCapturing = false;
}

// 更新処理
void FrameCaptureManager::Update(float deltaTime)
{
    // 通常の一回キャプチャモード
    if (m_bCapturing)
    {
        m_CaptureTimer += deltaTime;
        m_FrameAccumulator += deltaTime;

        // フレーム間隔ごとにキャプチャ
        while (m_FrameAccumulator >= m_FrameInterval && m_CapturedFrameCount < m_MaxFrames)
        {
            CaptureFrame();
            m_FrameAccumulator -= m_FrameInterval;
        }

        // 指定秒数経過でキャプチャ終了
        if (m_CaptureTimer >= m_CaptureDuration)
        {
            StopCapture();
        }
    }

    // ロールバッファ常時キャプチャ（一時停止中はスキップ）
    if (m_bRolling && !m_bRollingPaused)
    {
        // フレーム単位でカウントして、sample interval毎に1キャプチャを行う
        m_FrameCounter++;
        if (m_FrameCounter >= m_SampleIntervalFrames)
        {
            m_FrameCounter = 0;
            // キャプチャ実行（時刻ベースの間隔は無視）
            CaptureFrame();
            // m_CapturedFrameCount は最大 m_MaxFrames を上限にする
            if (m_CapturedFrameCount < m_MaxFrames) m_CapturedFrameCount++;
        }
    }

	// 再生トリガーキー判定（キャプチャ完了後のみ）
    if (!m_bCapturing && m_CapturedFrameCount > 0 && !m_bPlaying && !m_bRewindMode)
	{
        if (Input::IsKeyDown(VK_F9) || m_IsPlaybackTriggerKey)
        {
            // F9 押下で巻き戻しモード開始（再生を 60fps で行う）
            m_bRewindMode = true;
            m_PlaybackIndex = m_CapturedFrameCount - 1;
            m_PlaybackAccumulator = 0.0f;
            // Backup current frame interval and set to 60 FPS for rewind playback
            m_PlaybackIntervalBackup = m_FrameInterval;
            m_FrameInterval = 1.0f / 60.0f;
            m_bPlaying = true; // reuse playing state for rendering rewind
            m_bLoopPlayback = false; // 巻き戻しはループしない
            m_bReloadOnComplete = true; // シーン再構築を行う
            m_IsPlaybackTriggerKey = false; // トリガーをリセット
        }
	}
}

// バックバッファをコピー
void FrameCaptureManager::CaptureFrame()
{
    // キャプチャモードまたはロールモードのいずれかでない場合は何もしない
    if (!m_bCapturing && !m_bRolling) return;
    if (m_CaptureTextures.empty()) return;

	auto* pDevice = DirectX11::GetInstance().GetDevice();
	auto* pContext = DirectX11::GetInstance().GetContext();
	auto* pSwapChain = DirectX11::GetInstance().GetSwapChain();
	if (!pDevice || !pContext || !pSwapChain) return;

    // PostEffectManager が有効な場合のみリゾルブ済みテクスチャを使う
    auto& pe = PostEffectManager::GetInstance();
    bool usePostEffect = pe.IsGray() || pe.IsCircleGrayActive() || pe.IsBlurEnabled();
    // フェード（円形グレースケール）が進行中の場合はキャプチャを開始せず待機する
    if (pe.IsCircleGrayActive())
    {
#if ENABLE_FRAMECAPTURE_IMGUI
        Log::GetInstance().LogInfo("FrameCapture: Skipping capture while circle fade active");
#endif
        return;
    }
    
    if (m_MaxFrames <= 0) {
        // nothing created
#if _DEBUG
        Log::GetInstance().LogWarning("FrameCapture: m_MaxFrames <= 0, skipping capture");
#endif
        return;
    }

    int index = m_WriteIndex % m_MaxFrames;

    // 目標テクスチャが作成されていない場合は失敗ログを出して書き込みインデックスを進める
    if (index < 0 || index >= static_cast<int>(m_CaptureTextures.size()) || m_CaptureTextures[index] == nullptr)
    {
#if _DEBUG
        std::stringstream ss;
        ss << "FrameCapture: target texture missing at index=" << index << ", skipping write";
        Log::GetInstance().LogWarning(ss.str());
#endif
        m_WriteIndex++;
        if (m_CapturedFrameCount < m_MaxFrames) m_CapturedFrameCount++;
        return;
    }

    if (usePostEffect && pe.GetSceneResolvedTex()) {
        // PostEffect が有効で SceneResolvedTex がある場合はそれを使う
        // リゾルブ済みテクスチャがバックバッファ解像度の場合はダウンサンプリングしてコピー
        D3D11_TEXTURE2D_DESC srcDesc;
        pe.GetSceneResolvedTex()->GetDesc(&srcDesc);
        if (m_DownsampleFactor > 1) {
            // ソフトウェア的ダウンサンプリング：Resolve -> ステージング -> サンプリングは重い
            // ここではシンプルに CopyResource しておき、ダウンサンプリングは未実装（将来の改善点）。
            // If downsampling requested, render the source SRV into the downsampled RTV
            if (m_DownsampleFactor > 1 && m_CaptureRTVs[index]) {
                ID3D11ShaderResourceView* srcSRV = pe.GetSceneSRV();
                // Set render target to downsampled texture
                ID3D11RenderTargetView* prevRTV = nullptr;
                ID3D11DepthStencilView* prevDSV = DirectX11::GetInstance().GetBackBufferDSV();
                // Save current RTV (use ResetRenderTarget to restore later)
                pContext->OMGetRenderTargets(1, &prevRTV, nullptr);
                // Save and set viewport to match downsampled render target so fullscreen quad covers whole target
                D3D11_VIEWPORT prevViewports[16];
                UINT prevNumViewports = 16;
                pContext->RSGetViewports(&prevNumViewports, prevViewports);
                D3D11_VIEWPORT vp = { 0.0f, 0.0f, static_cast<FLOAT>(m_TargetCaptureWidth), static_cast<FLOAT>(m_TargetCaptureHeight), 0.0f, 1.0f };
                pContext->RSSetViewports(1, &vp);
                pContext->OMSetRenderTargets(1, &m_CaptureRTVs[index], nullptr);
                // Draw fullscreen quad sampling srcSRV
                pContext->PSSetShaderResources(0, 1, &srcSRV);
                pContext->VSSetShader(m_pVertexShader, nullptr, 0);
                pContext->PSSetShader(m_pPixelShader, nullptr, 0);
                UINT stride = sizeof(FullscreenVertex);
                UINT offset = 0;
                pContext->IASetVertexBuffers(0, 1, &m_pFullscreenVB, &stride, &offset);
                pContext->IASetInputLayout(m_pInputLayout);
                pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                pContext->PSSetSamplers(0, 1, &m_pSamplerState);
                pContext->Draw(6, 0);
                // Unbind
                ID3D11ShaderResourceView* nullSRV = nullptr;
                pContext->PSSetShaderResources(0, 1, &nullSRV);
                // Restore previous RTV
                pContext->OMSetRenderTargets(1, &prevRTV, prevDSV);
                // Restore previous viewports
                if (prevNumViewports > 0) pContext->RSSetViewports(prevNumViewports, prevViewports);
                if (prevRTV) prevRTV->Release();
            }
            else {
                pContext->CopyResource(m_CaptureTextures[index], pe.GetSceneResolvedTex());
            }
        } else {
            pContext->CopyResource(m_CaptureTextures[index], pe.GetSceneResolvedTex());
        }
#if ENABLE_FRAMECAPTURE_IMGUI
        Log::GetInstance().LogInfo("FrameCapture: Using PostEffect resolved texture");
#endif
    }
    else {
        // バックバッファを取得してコピーまたはダウンサンプリングレンダー
        ID3D11Texture2D* pBackBuffer = nullptr;
        HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
        if (FAILED(hr) || !pBackBuffer) {
#if ENABLE_FRAMECAPTURE_IMGUI
            Log::GetInstance().LogError("FrameCapture: Failed to get back buffer");
#endif
            return;
        }

        D3D11_TEXTURE2D_DESC backDesc;
        pBackBuffer->GetDesc(&backDesc);

#if ENABLE_FRAMECAPTURE_IMGUI
        {
            std::stringstream ss;
            ss << "FrameCapture: BackBuffer SampleCount=" << backDesc.SampleDesc.Count 
               << " Format=" << backDesc.Format 
               << " Size=" << backDesc.Width << "x" << backDesc.Height;
            Log::GetInstance().LogInfo(ss.str());
        }
#endif

        if (m_DownsampleFactor > 1 && m_CaptureRTVs[index]) {
            // Create SRV for backbuffer temporarily
            ID3D11ShaderResourceView* backSRV = nullptr;
            ID3D11Texture2D* pTempResolved = nullptr;
            // If backbuffer is MSAA, CreateShaderResourceView with TEXTURE2D will fail (E_INVALIDARG).
            // Resolve into a non-MSAA texture first, then create SRV from that.
            if (backDesc.SampleDesc.Count > 1)
            {
                D3D11_TEXTURE2D_DESC tmpDesc = {};
                tmpDesc.Width = backDesc.Width;
                tmpDesc.Height = backDesc.Height;
                tmpDesc.MipLevels = 1;
                tmpDesc.ArraySize = 1;
                tmpDesc.Format = backDesc.Format;
                tmpDesc.SampleDesc.Count = 1; // non-MSAA
                tmpDesc.SampleDesc.Quality = 0;
                tmpDesc.Usage = D3D11_USAGE_DEFAULT;
                tmpDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                tmpDesc.CPUAccessFlags = 0;
                tmpDesc.MiscFlags = 0;

                HRESULT hrTmp = pDevice->CreateTexture2D(&tmpDesc, nullptr, &pTempResolved);
                if (FAILED(hrTmp) || pTempResolved == nullptr)
                {
                    std::stringstream ss;
                    ss << "FrameCapture: CreateTexture2D for resolve temp failed hr=0x" << std::hex << hrTmp;
                    Log::GetInstance().LogError(ss.str());
                    pBackBuffer->Release();
                    return;
                }

                // Resolve MSAA backbuffer into non-MSAA texture
                pContext->ResolveSubresource(pTempResolved, 0, pBackBuffer, 0, backDesc.Format);

                D3D11_SHADER_RESOURCE_VIEW_DESC backSrvDesc = {};
                backSrvDesc.Format = tmpDesc.Format;
                backSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                backSrvDesc.Texture2D.MostDetailedMip = 0;
                backSrvDesc.Texture2D.MipLevels = 1;
                HRESULT hrCreateSRV = pDevice->CreateShaderResourceView(pTempResolved, &backSrvDesc, &backSRV);
                if (FAILED(hrCreateSRV) || backSRV == nullptr)
                {
                    std::stringstream ss;
                    ss << "FrameCapture: CreateSRV for resolved backbuffer failed hr=0x" << std::hex << hrCreateSRV;
                    Log::GetInstance().LogError(ss.str());
                    if (pTempResolved) pTempResolved->Release();
                    pBackBuffer->Release();
                    return;
                }
            }
            else
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC backSrvDesc = {};
                backSrvDesc.Format = backDesc.Format;
                backSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                backSrvDesc.Texture2D.MostDetailedMip = 0;
                backSrvDesc.Texture2D.MipLevels = 1;
                HRESULT hrCreateSRV = pDevice->CreateShaderResourceView(pBackBuffer, &backSrvDesc, &backSRV);
                if (FAILED(hrCreateSRV) || backSRV == nullptr) {
                    std::stringstream ss;
                    ss << "FrameCapture: CreateSRV for backbuffer failed hr=0x" << std::hex << hrCreateSRV;
                    Log::GetInstance().LogError(ss.str());
                    pBackBuffer->Release();
                    return;
                }
            }

            // Render sample into downsampled RTV
            ID3D11RenderTargetView* prevRTV = nullptr;
            ID3D11DepthStencilView* prevDSV = nullptr;
            pContext->OMGetRenderTargets(1, &prevRTV, &prevDSV);
            pContext->OMSetRenderTargets(1, &m_CaptureRTVs[index], nullptr);
            // Ensure viewport matches the capture target so the quad fills entire texture
            D3D11_VIEWPORT prevViewports2[16];
            UINT prevNumViewports2 = 16;
            pContext->RSGetViewports(&prevNumViewports2, prevViewports2);
            D3D11_VIEWPORT vp2 = { 0.0f, 0.0f, static_cast<FLOAT>(m_TargetCaptureWidth), static_cast<FLOAT>(m_TargetCaptureHeight), 0.0f, 1.0f };
            pContext->RSSetViewports(1, &vp2);
            pContext->PSSetShaderResources(0, 1, &backSRV);
            pContext->VSSetShader(m_pVertexShader, nullptr, 0);
            pContext->PSSetShader(m_pPixelShader, nullptr, 0);
            UINT stride = sizeof(FullscreenVertex);
            UINT offset = 0;
            pContext->IASetVertexBuffers(0, 1, &m_pFullscreenVB, &stride, &offset);
            pContext->IASetInputLayout(m_pInputLayout);
            pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            pContext->PSSetSamplers(0, 1, &m_pSamplerState);
            pContext->Draw(6, 0);
            ID3D11ShaderResourceView* nullSRV = nullptr;
            pContext->PSSetShaderResources(0, 1, &nullSRV);
            pContext->OMSetRenderTargets(1, &prevRTV, prevDSV);
            if (prevNumViewports2 > 0) pContext->RSSetViewports(prevNumViewports2, prevViewports2);
            if (prevRTV) prevRTV->Release();
            if (prevDSV) prevDSV->Release();
            if (backSRV) backSRV->Release();
            if (pTempResolved) pTempResolved->Release();
        }
        else {
            if (backDesc.SampleDesc.Count > 1) {
                // MSAA の場合は ResolveSubresource を使って非MSAAテクスチャへ解決
                pContext->ResolveSubresource(m_CaptureTextures[index], 0, pBackBuffer, 0, backDesc.Format);
            }
            else {
                // 通常はコピーで良い
                pContext->CopyResource(m_CaptureTextures[index], pBackBuffer);
            }
        }

        pBackBuffer->Release();
    }

#if ENABLE_FRAMECAPTURE_IMGUI
    // デバッグ: コピー直後のキャプチャテクスチャの先頭ピクセルをCPU読み出ししてログ出力
    if (m_CaptureTextures[index]) {
        D3D11_TEXTURE2D_DESC capturedDesc;
        m_CaptureTextures[index]->GetDesc(&capturedDesc);

        // 対応フォーマットのみチェック
        if (capturedDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM || capturedDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM) {
            D3D11_TEXTURE2D_DESC stagDesc = {};
            stagDesc.Width = capturedDesc.Width;
            stagDesc.Height = capturedDesc.Height;
            stagDesc.MipLevels = 1;
            stagDesc.ArraySize = 1;
            stagDesc.Format = capturedDesc.Format;
            stagDesc.SampleDesc.Count = 1;
            stagDesc.Usage = D3D11_USAGE_STAGING;
            stagDesc.BindFlags = 0;
            stagDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            stagDesc.MiscFlags = 0;

            ID3D11Texture2D* pStaging = nullptr;
            HRESULT hrSt = pDevice->CreateTexture2D(&stagDesc, nullptr, &pStaging);
            if (SUCCEEDED(hrSt) && pStaging) {
                pContext->CopyResource(pStaging, m_CaptureTextures[index]);

                D3D11_MAPPED_SUBRESOURCE mapped = {};
                HRESULT hrMap = pContext->Map(pStaging, 0, D3D11_MAP_READ, 0, &mapped);
                if (SUCCEEDED(hrMap)) {
                    unsigned char* pData = reinterpret_cast<unsigned char*>(mapped.pData);
                    unsigned int r = 0, g = 0, b = 0, a = 0;
                    if (capturedDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM) {
                        r = pData[0]; g = pData[1]; b = pData[2]; a = pData[3];
                    }
                    else { // BGRA
                        b = pData[0]; g = pData[1]; r = pData[2]; a = pData[3];
                    }
                    {
                        std::stringstream ss;
                        ss << "FrameCapture Debug idx=" << index << " firstpixel RGBA=" << r << "," << g << "," << b << "," << a;
                        Log::GetInstance().LogInfo(ss.str());

                        m_CapturedFrameCount = index;
                    }
                    pContext->Unmap(pStaging, 0);
                }
                else {
                    OutputDebugStringA("FrameCapture Debug: Map staging failed\n");
                }
                pStaging->Release();
            }
            else {
                OutputDebugStringA("FrameCapture Debug: Create staging failed\n");
            }
        }
        else {
            OutputDebugStringA("FrameCapture Debug: Unsupported format for quick read\n");
        }
    }
#endif

    m_WriteIndex++;

}

// 再生開始
void FrameCaptureManager::StartPlayback(bool loop)
{
	if (m_bPlaying || m_CapturedFrameCount == 0) return;

	m_bLoopPlayback = loop;
	m_PlaybackIndex = 0;
	m_PlaybackAccumulator = 0.0f;
	// Backup current frame interval so we can restore after special playback modes
	m_PlaybackIntervalBackup = m_FrameInterval;
	m_bPlaying = true;
}

// 再生停止
void FrameCaptureManager::StopPlayback()
{
	m_bPlaying = false;
	// Restore frame interval if it was changed for special playback
	if (m_PlaybackIntervalBackup > 0.0f)
	{
		m_FrameInterval = m_PlaybackIntervalBackup;
		m_PlaybackIntervalBackup = 0.0f;
	}
}

// シーンリロード要求をチェックして消費
bool FrameCaptureManager::ConsumeReloadRequest()
{
    if (m_bRequestSceneReload)
    {
        m_bRequestSceneReload = false;
        return true;
    }
    return false;
}

// バッファ初期化（タイトルへ戻る時などに呼ぶ）
void FrameCaptureManager::ClearBuffer()
{
    StopPlayback();
    m_bCapturing = false;
    m_bRolling = false;
    m_bRewindMode = false;
    m_bReloadOnComplete = false;
    m_bRequestSceneReload = false;
    m_IsPlaybackTriggerKey = false;
    m_CaptureTimer = 0.0f;
    m_FrameAccumulator = 0.0f;
    m_WriteIndex = 0;
    m_CapturedFrameCount = 0;
    m_PlaybackIndex = 0;
    m_PlaybackAccumulator = 0.0f;
    m_FrameCounter = 0;
    m_RewindTime = 0.0f;
    
    // テクスチャは解放して新しいシーンでCreate時に再作成される
    ReleaseCaptureTextures();
}

// 巻き戻しエフェクトの初期化
void FrameCaptureManager::InitRewindEffect(float intensity, float chromatic, float grayscale)
{
    m_RewindTime = 0.0f;
    m_RewindIntensity = intensity;
    m_RewindChromatic = chromatic;
    m_RewindGrayscale = grayscale;
}

// 再生用描画
void FrameCaptureManager::RenderPlayback(float deltaTime)
{
	if (!m_bPlaying || m_CapturedFrameCount == 0) return;


	auto* pContext = DirectX11::GetInstance().GetContext();
	if (!pContext) return;

	// フレーム進行
	m_PlaybackAccumulator += deltaTime;
	while (m_PlaybackAccumulator >= m_FrameInterval)
	{
		m_PlaybackAccumulator -= m_FrameInterval;
        // 巻き戻し中は逆方向に進める
        if (m_bRewindMode)
        {
            m_PlaybackIndex--;
            // 巻き戻し完了チェック
            if (m_PlaybackIndex < 0)
            {
                // 巻き戻し完了
                m_bRewindMode = false;
                m_bPlaying = false;
                m_bRolling = false; // ロールキャプチャも停止
                if (m_bReloadOnComplete)
                {
                    m_bReloadOnComplete = false;
                    // Draw内でシーン破棄は危険なのでフラグを立てて次フレームで処理
                    m_bRequestSceneReload = true;
                }
                return;
            }
        }
        else
        {
            m_PlaybackIndex++;
            if (m_PlaybackIndex >= m_CapturedFrameCount)
            {
                if (m_bLoopPlayback)
                {
                    m_PlaybackIndex = 0;
                }
                else
                {
                    StopPlayback();
                    return;
                }
            }
        }
	}

    // 現在のフレームを描画
    if (m_CapturedFrameCount == 0) {
        // キャプチャが無ければ再生を停止して呼び出し元へ戻す
        StopPlayback();
        return;
    }

    // 巻き戻し完了チェックはwhileループ内で処理済み

    int frameIndex = 0;
    if (m_bRewindMode)
    {
        // 巻き戻し中は範囲内のインデックスをそのまま使う
        frameIndex = std::clamp(m_PlaybackIndex, 0, m_CapturedFrameCount - 1);
    }
    else
    {
        frameIndex = m_PlaybackIndex % m_CapturedFrameCount;
    }

    if (m_CaptureSRVs.empty() || !m_CaptureSRVs[frameIndex]) {
        // 無効なSRVなら再生を停止してデフォルト描画にフォールバック
        StopPlayback();
        return;
    }

    // バックバッファをクリア
    DirectX11::GetInstance().ClearBackBuffer();
    DirectX11::GetInstance().ResetRenderTarget();

    // 深度テストOFF、アルファブレンドOFF
    DirectX11::GetInstance().SetDepth(false);
    DirectX11::GetInstance().SetAlphaBlend(false);

    if (m_bRewindMode && m_pRewindPixelShader)
    {
        // 巻き戻し専用描画（歪みシェーダを使用）
        m_RewindTime += deltaTime;

        // 強度は巻き戻しの進行に応じて減衰させる（開始時に強め、終盤は弱め）
        float norm = 0.0f;
        if (m_CapturedFrameCount > 1)
            norm = static_cast<float>(frameIndex) / static_cast<float>(m_CapturedFrameCount - 1);
        // norm: 0 (最初のフレーム) .. 1 (最後のフレーム)
        // Intensity: 高 -> 低
        float intensity = m_RewindIntensity * (1.0f - norm * 0.7f);

        // シェーダバインド
        pContext->VSSetShader(m_pVertexShader, nullptr, 0);
        pContext->PSSetShader(m_pRewindPixelShader, nullptr, 0);
        pContext->IASetInputLayout(m_pInputLayout);

        // 頂点バッファ設定
        UINT stride = sizeof(FullscreenVertex);
        UINT offset = 0;
        pContext->IASetVertexBuffers(0, 1, &m_pFullscreenVB, &stride, &offset);
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // テクスチャとサンプラー設定
        pContext->PSSetShaderResources(0, 1, &m_CaptureSRVs[frameIndex]);
        pContext->PSSetSamplers(0, 1, &m_pSamplerState);

        // 定数バッファ更新
        if (m_pRewindCB)
        {
            D3D11_MAPPED_SUBRESOURCE mapped = {};
            if (SUCCEEDED(pContext->Map(m_pRewindCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            {
                float cb[4] = { m_RewindTime, intensity, m_RewindChromatic, m_RewindGrayscale };
                memcpy(mapped.pData, cb, sizeof(cb));
                pContext->Unmap(m_pRewindCB, 0);
            }
            pContext->PSSetConstantBuffers(0, 1, &m_pRewindCB);
        }

        // 描画
        pContext->Draw(6, 0);

        // アンバインド
        ID3D11ShaderResourceView* nullSRV = nullptr;
        pContext->PSSetShaderResources(0, 1, &nullSRV);
    }
    else if (m_bRewindMode)
    {
        // フォールバック: PostEffectManager 経由のグレースケール（既存実装）
        auto& pe = PostEffectManager::GetInstance();
        bool prevGray = pe.IsGray();
        pe.SetGray(true);
        pe.RenderSRVWithPostEffects(m_CaptureSRVs[frameIndex], m_TargetCaptureWidth, m_TargetCaptureHeight, true);
        pe.SetGray(prevGray);
    }
    else
    {
        // シェーダー設定
        pContext->VSSetShader(m_pVertexShader, nullptr, 0);
        pContext->PSSetShader(m_pPixelShader, nullptr, 0);
        pContext->IASetInputLayout(m_pInputLayout);

        // 頂点バッファ設定
        UINT stride = sizeof(FullscreenVertex);
        UINT offset = 0;
        pContext->IASetVertexBuffers(0, 1, &m_pFullscreenVB, &stride, &offset);
        pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // テクスチャとサンプラー設定
        pContext->PSSetShaderResources(0, 1, &m_CaptureSRVs[frameIndex]);
        pContext->PSSetSamplers(0, 1, &m_pSamplerState);

        // 描画
        pContext->Draw(6, 0);

        // シェーダーリソースをアンバインド
        ID3D11ShaderResourceView* nullSRV = nullptr;
        pContext->PSSetShaderResources(0, 1, &nullSRV);
    }

    // 深度テストを戻す
    DirectX11::GetInstance().SetDepth(true);
}

// フレーム保存用テクスチャの作成
void FrameCaptureManager::CreateCaptureTextures()
{
	auto* pDevice = DirectX11::GetInstance().GetDevice();
	auto* pSwapChain = DirectX11::GetInstance().GetSwapChain();
	if (!pDevice || !pSwapChain) return;

	// バックバッファからフォーマットとサイズを取得
	ID3D11Texture2D* pBackBuffer = nullptr;
	HRESULT hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr) || !pBackBuffer) return;

	D3D11_TEXTURE2D_DESC backBufferDesc;
	pBackBuffer->GetDesc(&backBufferDesc);
	pBackBuffer->Release();

    // ダウンサンプリング解像度計算
    int outWidth = backBufferDesc.Width / std::max(1, m_DownsampleFactor);
    int outHeight = backBufferDesc.Height / std::max(1, m_DownsampleFactor);
    m_TargetCaptureWidth = outWidth;
    m_TargetCaptureHeight = outHeight;

    // キャプチャ用テクスチャを作成（ダウンサンプリング解像度）
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = outWidth;
    texDesc.Height = outHeight;
    m_CaptureWidth = backBufferDesc.Width;
    m_CaptureHeight = backBufferDesc.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = backBufferDesc.Format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	m_CaptureTextures.resize(m_MaxFrames);
	m_CaptureSRVs.resize(m_MaxFrames);
    m_CaptureRTVs.resize(m_MaxFrames);

    for (int i = 0; i < m_MaxFrames; ++i)
    {
        // Create texture with RTV bind so we can render downsampled content into it
        D3D11_TEXTURE2D_DESC texDescRT = texDesc;
        texDescRT.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        hr = pDevice->CreateTexture2D(&texDescRT, nullptr, &m_CaptureTextures[i]);
        if (FAILED(hr))
        {
            m_CaptureTextures[i] = nullptr;
            m_CaptureSRVs[i] = nullptr;
            // Failed to create texture -> stop creating further to avoid repeated allocation failures
            std::stringstream ss;
            ss << "FrameCapture: CreateTexture2D failed at i=" << i << " hr=" << std::hex << hr;
            Log::GetInstance().LogError(ss.str());
            // shrink vectors to created count
            int created = i;
            m_CaptureTextures.resize(created);
            m_CaptureSRVs.resize(created);
            m_CaptureRTVs.resize(created);
            m_MaxFrames = created;
            break;
        }

		// SRV作成
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

        hr = pDevice->CreateShaderResourceView(m_CaptureTextures[i], &srvDesc, &m_CaptureSRVs[i]);
        if (FAILED(hr))
        {
            m_CaptureSRVs[i] = nullptr;
            std::stringstream ss; ss << "FrameCapture: CreateSRV failed at i=" << i << " hr=" << std::hex << hr; Log::GetInstance().LogError(ss.str());
        }
        else
        {
            // デバッグ用: SRV が作成できたことをログ出力
            // (Releaseビルドでは無効化するため _DEBUG に限定)
#if ENABLE_FRAMECAPTURE_IMGUI
            Log::GetInstance().LogInfo("FrameCapture: Created SRV for frame");
#endif
        }
        // Create RTV for rendering into (downsample target)
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = texDesc.Format;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        ID3D11RenderTargetView* pRTV = nullptr;
        hr = pDevice->CreateRenderTargetView(m_CaptureTextures[i], &rtvDesc, &pRTV);
        if (SUCCEEDED(hr)) {
            m_CaptureRTVs[i] = pRTV;
        } else {
            m_CaptureRTVs[i] = nullptr;
            std::stringstream ss; ss << "FrameCapture: CreateRTV failed at i=" << i << " hr=" << std::hex << hr; Log::GetInstance().LogWarning(ss.str());
        }
	}

    // Ensure m_MaxFrames matches actually created count
    if (static_cast<int>(m_CaptureTextures.size()) < m_MaxFrames) {
        m_MaxFrames = static_cast<int>(m_CaptureTextures.size());
    }

    // ダウンサンプリング用レンダーターゲット（SRV 作成済みのテクスチャをレンダーターゲットとしても使う場合）
    // ただし、CreateTexture2D で BindFlags に RTV を加えて作る必要があるため、ここでは必要なら追加実装。
}

// フレーム保存用テクスチャの解放
void FrameCaptureManager::ReleaseCaptureTextures()
{
	for (auto* pSRV : m_CaptureSRVs)
	{
		if (pSRV) pSRV->Release();
	}
	m_CaptureSRVs.clear();

	for (auto* pTex : m_CaptureTextures)
	{
		if (pTex) pTex->Release();
	}
	m_CaptureTextures.clear();

    for (auto* pRTV : m_CaptureRTVs)
    {
        if (pRTV) pRTV->Release();
    }
    m_CaptureRTVs.clear();

	m_CapturedFrameCount = 0;
	m_WriteIndex = 0;
}

// フルスクリーンクワッド用リソースの作成
void FrameCaptureManager::CreateFullscreenQuadResources()
{
	auto* pDevice = DirectX11::GetInstance().GetDevice();
	if (!pDevice) return;

	HRESULT hr;

	// 頂点バッファ作成
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.ByteWidth = sizeof(g_FullscreenQuadVertices);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = g_FullscreenQuadVertices;

	hr = pDevice->CreateBuffer(&vbDesc, &vbData, &m_pFullscreenVB);
	if (FAILED(hr)) return;

	// サンプラーステート作成
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = pDevice->CreateSamplerState(&sampDesc, &m_pSamplerState);
	if (FAILED(hr)) return;

	// 頂点シェーダーコンパイル
	ID3DBlob* pVSBlob = nullptr;
	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompile(
		g_VS_Source, strlen(g_VS_Source), "VS",
		nullptr, nullptr, "main", "vs_4_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0,
		&pVSBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob) pErrorBlob->Release();
		return;
	}

	hr = pDevice->CreateVertexShader(
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		nullptr, &m_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return;
	}

	// 入力レイアウト作成
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = pDevice->CreateInputLayout(
		layout, 2,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		&m_pInputLayout);
	pVSBlob->Release();
	if (FAILED(hr)) return;

	// ピクセルシェーダーコンパイル
	ID3DBlob* pPSBlob = nullptr;
	hr = D3DCompile(
		g_PS_Source, strlen(g_PS_Source), "PS",
		nullptr, nullptr, "main", "ps_4_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0,
		&pPSBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob) pErrorBlob->Release();
		return;
	}

	hr = pDevice->CreatePixelShader(
		pPSBlob->GetBufferPointer(),
		pPSBlob->GetBufferSize(),
		nullptr, &m_pPixelShader);
	pPSBlob->Release();

	// 巻き戻し用ピクセルシェーダーのコンパイル
	ID3DBlob* pRewindPSBlob = nullptr;
	hr = D3DCompile(
		g_PS_Rewind_Source, strlen(g_PS_Rewind_Source), "PSRewind",
		nullptr, nullptr, "main", "ps_4_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0,
		&pRewindPSBlob, &pErrorBlob);
	if (SUCCEEDED(hr) && pRewindPSBlob)
	{
		hr = pDevice->CreatePixelShader(
			pRewindPSBlob->GetBufferPointer(),
			pRewindPSBlob->GetBufferSize(),
			nullptr, &m_pRewindPixelShader);
		pRewindPSBlob->Release();
	}
	else
	{
		if (pErrorBlob) pErrorBlob->Release();
	}

	// 巻き戻し用定数バッファ作成
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.ByteWidth = sizeof(float) * 4; // Time, Intensity, Chromatic, Padding
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	pDevice->CreateBuffer(&cbDesc, nullptr, &m_pRewindCB);
}

// フルスクリーンクワッド用リソースの解放
void FrameCaptureManager::ReleaseFullscreenQuadResources()
{
	if (m_pInputLayout) { m_pInputLayout->Release(); m_pInputLayout = nullptr; }
	if (m_pPixelShader) { m_pPixelShader->Release(); m_pPixelShader = nullptr; }
	if (m_pRewindPixelShader) { m_pRewindPixelShader->Release(); m_pRewindPixelShader = nullptr; }
	if (m_pRewindCB) { m_pRewindCB->Release(); m_pRewindCB = nullptr; }
	if (m_pVertexShader) { m_pVertexShader->Release(); m_pVertexShader = nullptr; }
	if (m_pSamplerState) { m_pSamplerState->Release(); m_pSamplerState = nullptr; }
	if (m_pFullscreenVB) { m_pFullscreenVB->Release(); m_pFullscreenVB = nullptr; }
}

// 設定の読み込み
void FrameCaptureManager::LoadSettings(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
	{
		// デフォルト値を使用
		return;
	}

	try
	{
		json j;
		file >> j;

		if (j.contains("CaptureDuration"))
			m_CaptureDuration = j["CaptureDuration"].get<float>();
		if (j.contains("CaptureFPS"))
			m_CaptureFPS = j["CaptureFPS"].get<int>();
		if (j.contains("PlaybackTriggerKey"))
            m_IsPlaybackTriggerKey = j["PlaybackTriggerKey"].get<bool>();
	}
	catch (...)
	{
		// パースエラー時はデフォルト値を使用
	}
}

// 設定の保存
void FrameCaptureManager::SaveSettings(const std::string& filePath)
{
	json j;
	j["CaptureDuration"] = m_CaptureDuration;
	j["CaptureFPS"] = m_CaptureFPS;
	j["PlaybackTriggerKey"] = m_IsPlaybackTriggerKey;

	std::ofstream file(filePath);
	if (file.is_open())
	{
		file << j.dump(2);
	}
}

// ImGuiデバッグ表示
void FrameCaptureManager::DebugImGui()
{
#if _DEBUG || ENABLE_FRAMECAPTURE_IMGUI
	if (ImGui::Begin(IMGUI_JP("フレームキャプチャ")))
	{
		ImGui::Text(IMGUI_JP("状態: %s"), 
			m_bCapturing ? IMGUI_JP("キャプチャ中") : 
			(m_bPlaying ? IMGUI_JP("再生中") : IMGUI_JP("待機中")));

		ImGui::Text(IMGUI_JP("キャプチャ済みフレーム: %d / %d"), m_CapturedFrameCount, m_MaxFrames);

		if (m_bCapturing)
		{
			float progress = m_CaptureTimer / m_CaptureDuration;
			ImGui::ProgressBar(progress, ImVec2(-1, 0), IMGUI_JP("キャプチャ進行"));
		}

		if (m_bPlaying)
		{
			float progress = static_cast<float>(m_PlaybackIndex) / static_cast<float>(m_CapturedFrameCount);
			ImGui::ProgressBar(progress, ImVec2(-1, 0), IMGUI_JP("再生進行"));
		}

		ImGui::Separator();
		ImGui::Text(IMGUI_JP("設定"));

		float duration = m_CaptureDuration;
		if (ImGui::SliderFloat(IMGUI_JP("キャプチャ秒数"), &duration, 1.0f, 10.0f))
		{
			m_CaptureDuration = duration;
		}

		int fps = m_CaptureFPS;
		if (ImGui::SliderInt(IMGUI_JP("キャプチャFPS"), &fps, 10, 60))
		{
			m_CaptureFPS = fps;
		}

		// メモリ見積もり表示
		int estimatedFrames = static_cast<int>(m_CaptureDuration * m_CaptureFPS);
		float estimatedMemoryMB = estimatedFrames * (1920.0f * 1080.0f * 4.0f) / (1024.0f * 1024.0f);
		ImGui::Text(IMGUI_JP("推定メモリ使用量: %.1f MB"), estimatedMemoryMB);

		ImGui::Separator();

		if (!m_bCapturing && !m_bPlaying)
		{
			if (ImGui::Button(IMGUI_JP("キャプチャ開始")))
			{
				StartCapture(m_CaptureDuration, m_CaptureFPS);
			}
		}
		else if (m_bCapturing)
		{
			if (ImGui::Button(IMGUI_JP("キャプチャ停止")))
			{
				StopCapture();
			}
		}

		if (!m_bCapturing && m_CapturedFrameCount > 0 && !m_bPlaying)
		{
			if (ImGui::Button(IMGUI_JP("再生開始")))
			{
				StartPlayback(true);
			}
		}
		else if (m_bPlaying)
		{
			if (ImGui::Button(IMGUI_JP("再生停止")))
			{
				StopPlayback();
			}
		}

		ImGui::Separator();
		if (ImGui::Button(IMGUI_JP("設定を保存")))
		{
			SaveSettings();
		}
		ImGui::SameLine();
		if (ImGui::Button(IMGUI_JP("設定を読込")))
		{
			LoadSettings();
		}

        // デバッグ用: 特定フレーム表示
        ImGui::Separator();
        ImGui::Text(IMGUI_JP("解像度: %d x %d"), m_CaptureWidth, m_CaptureHeight); 

        // ダウンサンプル因子を実行時に調整できるようにする
        int downsample = m_DownsampleFactor;
        if (ImGui::SliderInt(IMGUI_JP("ダウンサンプル因子 (1=フル, 大きいほど粗く)"), &downsample, 1, 8))
        {
            if (downsample != m_DownsampleFactor)
            {
                // 変更中はキャプチャ／再生を停止してテクスチャを再作成
                StopCapture();
                StopPlayback();
                m_DownsampleFactor = downsample;
                ReleaseCaptureTextures();
                CreateCaptureTextures();
#if ENABLE_FRAMECAPTURE_IMGUI
                std::stringstream ss;
                ss << "FrameCapture: Downsample factor changed to " << m_DownsampleFactor;
                Log::GetInstance().LogInfo(ss.str());
#endif
            }
        }

        // 作成されたキャプチャテクスチャのターゲット解像度を表示
        ImGui::Text(IMGUI_JP("キャプチャターゲット解像度: %d x %d"), m_TargetCaptureWidth, m_TargetCaptureHeight);

        if (m_CapturedFrameCount > 0)
        {
            if (ImGui::SliderInt(IMGUI_JP("表示フレーム選択"), &m_DebugSelectedFrame, 0, m_CapturedFrameCount - 1))
            {
                // 選択変更
            }
            // Image 表示 (ImGui::Image expects ImTextureID)
            // DirectX11 SRV を ImTextureID として渡すために uintptr_t キャスト
            if (m_DebugSelectedFrame >= 0 && m_DebugSelectedFrame < m_CapturedFrameCount)
            {
                ID3D11ShaderResourceView* srv = m_CaptureSRVs[m_DebugSelectedFrame];
                if (srv)
                {
                    ImVec2 size(WND_WF, WND_HF);
                    ImGui::Image((void*)srv, size);
                }
                else
                {
                    ImGui::Text(IMGUI_JP("選択したフレームのSRVが無効です"));
                }
            }
        }
	}
	ImGui::End();
#endif // _DEBUG || ENABLE_FRAMECAPTURE_IMGUI
}
