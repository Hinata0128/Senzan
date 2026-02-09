#include "GameMain.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/00_Ground/Ground.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/05_SkyDome/SkyDome.h"
    
#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/Boss.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"

#include "Game/01_GameObject/02_UIObject/UIGameMain/UIGameMain.h"
#include "Game/01_GameObject/02_UIObject/UIGameOver/UIGameOver.h"
#include "Game/01_GameObject/02_UIObject/UIEnding/UIEnding.h"

#include "Game/02_Camera/CameraBase.h"
#include "Game/02_Camera/LockOnCamera/LockOnCamera.h"

#include "Game/04_Time/Time.h"

#include "Game/05_InputDevice/Input.h"

#include "Graphic/Shadow/Shadow.h"
#include "Graphic/Light/DirectionLight/DirectionLight.h"
#include "Graphic/Light/LightManager.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"

#include "System/Singleton/CameraManager/CameraManager.h"
#include "System/Singleton/CollisionDetector/CollisionDetector.h"
#include "System/Singleton/Debug/CollisionVisualizer/CollisionVisualizer.h"
#include "System/Singleton/SceneManager/SceneManager.h"
#include "System/Singleton/PostEffectManager/PostEffectManager.h"
#include "System/Singleton/SnowBallManager/SnowBallManager.h"
#include "System/Singleton/CombatCoordinator/CombatCoordinator.h"
#include "System/Singleton/FrameCaptureManager/FrameCaptureManager.h"

#if _DEBUG || ENABLE_FRAMECAPTURE_IMGUI
#include "System/Singleton/ImGui/CImGuiManager.h"
#endif

#include <algorithm> // std::min のために必要

// コンテニュー回数の静的変数定義
int GameMain::s_ContinueCount = 0;

// コンテニュー回数に応じたBoss HP倍率を取得
float GameMain::GetBossHPMultiplier()
{
	switch (s_ContinueCount)
	{
	case 0:  return 1.0f;   // 100%
	case 1:  return 0.9f;   // 90%
	case 2:  return 0.8f;   // 80%
	case 3:  return 0.6f;   // 60%
	default: return 0.5f;   // 4回以降は50%
	}
}

// コンストラクタ.
GameMain::GameMain()
    : SceneBase        ()
    , m_spCamera    ( nullptr )
    , m_upGround    (std::make_unique<Ground>())
    , m_upSkyDome   (std::make_unique<SkyDome>())
    , m_spLight        (std::make_shared<DirectionLight>())
    , m_upBoss        (std::make_unique<Boss>())
    , m_upPlayer    (std::make_unique<Player>())
    , m_upUI        (std::make_shared<UIGameMain>())
    , m_TimeLimit   (10800.0f*Time::GetInstance().GetDeltaTime())
    , m_upUIOver    ()
    , m_upUIEnding  ()
{
	Initialize();
	Time::GetInstance().StartTimer(m_TimeLimit);
}

// デストラクタ.
GameMain::~GameMain()
{
    // シーン終了時に CombatCoordinator の参照をクリア
    CombatCoordinator::GetInstance().Clear();

    SoundManager::GetInstance().AllStop();
}

void GameMain::Initialize()
{
    // コンテニュー回数に応じてBossのHPを設定
    m_upBoss->SetHPMultiplier(GetBossHPMultiplier());

    // ライト設定.
    m_spLight->SetDirection(DirectX::XMFLOAT3(1.5f, 1.f, -1.f));
    LightManager::AttachDirectionLight(m_spLight);

    // カメラ設定.
    m_spCamera = std::make_shared<LockOnCamera>(std::ref(*m_upPlayer), std::ref(*m_upBoss));
    CameraManager::GetInstance().SetCamera(m_spCamera);
    CameraManager::GetInstance().SetPosition({ 0.f, 3.f, 40.f });

    // 雪玉設定.
    SnowBallManager::GetInstance().Init();

    // CombatCoordinator に Player と Boss の参照を設定
    CombatCoordinator::GetInstance().Initialize(m_upPlayer.get(), m_upBoss.get());

    // ポストエフェクトのキャンセル.
    PostEffectManager::GetInstance().SetGray(false);
    PostEffectManager::GetInstance().SetBlurEnabled(false);

    FrameCaptureManager::GetInstance().ResetCapture();

    SoundManager::GetInstance().Play("Main", true);
    SoundManager::GetInstance().SetVolume("Main", 8000);
}

void GameMain::Create()
{
    // フレームキャプチャのロールキャプチャを開始(Gameシーン開始から常に保存)
    FrameCaptureManager::GetInstance().StartRollingCapture(10, 60);
}

void GameMain::Update()
{
    auto& fcm = FrameCaptureManager::GetInstance();
    
    // 巻き戻し完了後のシーンリロード要求をチェック
    if (fcm.ConsumeReloadRequest())
    {
        // キャプチャ状態をリセットしてシーンをリロード
        fcm.ResetCapture();
        SceneManager::LoadScene(eList::GameMain, false);
        return;
    }

    // 巻き戻し再生中はゲームロジックをスキップ（FrameCaptureManagerが描画を担当）
    if (fcm.IsPlaying())
    {
        // 巻き戻し再生の更新のみ実行
        fcm.Update(Time::GetInstance().GetDeltaTime());
        return;
    }

    Input::Update();
    if (m_upBoss->GetHP() <= 0 && !m_upUIEnding) {
        m_upUIEnding = std::make_shared<UIEnding>();
        // エンディング画面ではキャプチャを停止
        FrameCaptureManager::GetInstance().PauseRollingCapture();
        SoundManager::Stop("Main");
        SoundManager::Play("Ending");
        SoundManager::SetVolume("Ending", 8000);
    }
    else if ((m_upPlayer->GetHP() <= 0 || Time::GetInstance().IsTimerJustFinished()) && !m_upUIOver) {
        m_upUIOver = std::make_shared<UIGameOver>();
        // コンテニュー画面ではキャプチャを停止（バッファに入れない）
        FrameCaptureManager::GetInstance().PauseRollingCapture();
        SoundManager::Stop("Main");
        SoundManager::Play("Over");
        SoundManager::SetVolume("Over", 8000);
    }

    if (m_upUIOver || m_upUIEnding)
    {
        UIGameOver::Items selectedItem = UIGameOver::Items::End; // デフォルトは終了
        if (m_upUIOver) {
            m_upUIOver->Update();
            selectedItem = m_upUIOver->GetSelected();
        }
        else {
            m_upUIEnding->Update();
        }

        if (Input::IsKeyDown(VK_SPACE)
            || Input::IsKeyDown('C')
            || Input::IsButtonDown(XInput::Key::B))
        {
            if (selectedItem == UIGameOver::Items::Continue) {
                // コンテニュー選択 → 巻き戻し開始
                SoundManager::GetInstance().Play("Decide");
                SoundManager::GetInstance().SetVolume("Decide", 8000);

                // 巻き戻し開始（FrameCaptureManagerが巻き戻し完了後にシーンをリセットする）
                FrameCaptureManager::GetInstance().SetPlaybackTriggerKey(true);
                // コンテニュー回数をインクリメント（次回シーンロード時にBoss HPに反映）
                IncrementContinueCount();

                return;
            }
            else {
                // 終了選択 → タイトルへ
                SoundManager::GetInstance().Play("Decide");
                SoundManager::GetInstance().SetVolume("Decide", 8000);

                // タイトルへ戻る際にコンテニュー回数をリセット
                ResetContinueCount();

                SceneManager::LoadScene(eList::Title);
            }
        }
        UIUpdate();
        return;
    }

    m_upSkyDome->Update();
    m_upGround->Update();
    m_upBoss->Update();
    m_upBoss->SetTargetPos(m_upPlayer->GetPosition());

    SnowBallManager::GetInstance().Update();

    m_upPlayer->SetIsJustDodgeTiming(m_upBoss->IsAnyAttackJustWindow());

    m_upPlayer->SetTargetPos(m_upBoss.get()->GetPosition());
    m_upPlayer->Update();

    // ポストエフェクト更新
    PostEffectManager::GetInstance().Update(Time::GetInstance().GetDeltaTime());

    UIUpdate();

#if _DEBUG || ENABLE_FRAMECAPTURE_IMGUI
    ImGui::Begin("Gamemain Debug");
    bool gray = PostEffectManager::GetInstance().IsGray();
    if (ImGui::Checkbox("GrayScale", &gray)) {
        PostEffectManager::GetInstance().SetGray(gray);
    }
    ImGui::Text("Time: %.3f", Time::GetInstance().GetTimerProgress());
    if (ImGui::Button("Restart Timer")) {
        Time::GetInstance().StartTimer(m_TimeLimit);
    }
    
    ImGui::End();

    // フレームキャプチャマネージャのデバッグUI
    FrameCaptureManager::GetInstance().DebugImGui();
#endif
}

void GameMain::LateUpdate()
{
    m_upPlayer->LateUpdate();
    m_upBoss->LateUpdate();
    CameraManager::GetInstance().LateUpdate();

    m_upUI->LateUpdate();
    CollisionDetector::GetInstance().ExecuteCollisionDetection();
}


void GameMain::Draw()
{
    // 巻き戻し再生中はキャプチャフレームを描画
    auto& fcm = FrameCaptureManager::GetInstance();
    if (fcm.IsPlaying())
    {
        fcm.RenderPlayback(Time::GetInstance().GetDeltaTime());
        return;
    }

    Shadow::Begin();
    m_upGround->DrawDepth();
    Shadow::End();

    const bool useGray = PostEffectManager::GetInstance().IsGray();
    const bool useCircleGray = PostEffectManager::GetInstance().IsCircleGrayActive();
    const bool useBlur = PostEffectManager::GetInstance().IsBlurEnabled();
    if (useGray || useCircleGray || useBlur) {
        PostEffectManager::GetInstance().BeginSceneRender();
    }
   
    m_upGround->Draw();
    m_upSkyDome->Draw();
    m_upBoss->Draw();
    m_upPlayer->Draw();

    SnowBallManager::GetInstance().Draw();

    if (useGray || useCircleGray || useBlur) {
        PostEffectManager::GetInstance().DrawToBackBuffer();
    }

    m_upUI->Draw();

    if (m_upUIOver || m_upUIEnding)
    {
        if (m_upUIOver) { m_upUIOver->Draw(); }
        else { m_upUIEnding->Draw(); }
    }

#if _DEBUG
    CollisionVisualizer::GetInstance().Draw();
#endif // _DEBUG
}

HRESULT GameMain::LoadData()
{
    return S_OK; // 成功を返す
}

void GameMain::UIUpdate()
{
    m_upUI->SetBossHP(m_upBoss->GetMaxHP(), m_upBoss->GetHP());
    m_upUI->SetCombo(m_upPlayer->GetCombo());
    m_upUI->SetPlayerHP(m_upPlayer->GetMaxHP(), m_upPlayer->GetHP());
    m_upUI->SetPlayerUlt(m_upPlayer->GetMaxUltValue(), m_upPlayer->GetUltValue());
    m_upUI->SetTime(Time::GetInstance().GetTimerProgress());
    m_upUI->Update();
}
