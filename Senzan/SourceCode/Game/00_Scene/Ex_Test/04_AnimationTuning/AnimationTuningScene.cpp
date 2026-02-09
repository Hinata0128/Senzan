#include "AnimationTuningScene.h"

#include "Game/05_InputDevice/Input.h"

#include "Game//01_GameObject//02_UIObject/UIGameMain/UIGameMain.h"
#include "Game/02_Camera/CameraBase.h"
#include "Game/02_Camera/ThirdPersonCamera/PlayerThirdPersonCamera.h"
#include "Game/02_Camera/LockOnCamera/LockOnCamera.h"

#include "Graphic/Shadow/Shadow.h"
#include "Graphic/Light/DirectionLight/DirectionLight.h"
#include "Graphic/Light/LightManager.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/00_Ground/Ground.h"	// 地面Static.
#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"	// プレイヤー.
#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/Boss.h"	// ボス.

#include "Game/03_Collision/00_Core/ColliderBase.h"	
#include "Game/03_Collision/00_Core/01_Capsule/CapsuleCollider.h"

#include "Game/04_Time/Time.h"

#include "System/Singleton/Debug/CollisionVisualizer/CollisionVisualizer.h"

#include "System/Singleton/CameraManager/CameraManager.h"
#include "System/Singleton/CollisionDetector/CollisionDetector.h"
#include "System/Singleton/ImGui/CImGuiManager.h"

#include "SceneManager/SceneManager.h"

// コンストラクタ.
AnimationTuningScene::AnimationTuningScene()
    : SceneBase()
    , m_spCamera()
    , m_pLight(std::make_shared<DirectionLight>())
    , m_upPlayer(std::make_unique<Player>())
    , m_upBoss(std::make_unique<Boss>())
    , m_upGround(std::make_unique<Ground>())
{
    Initialize();
}

// デストラクタ.
AnimationTuningScene::~AnimationTuningScene()
{
}

void AnimationTuningScene::Initialize()
{
    m_spCamera = std::make_shared<LockOnCamera>(std::ref(*m_upPlayer), std::ref(*m_upBoss));
    // カメラ設定.
    //m_spCamera->SetPosition(DirectX::XMFLOAT3(0.0f, 5.0f, -50.0f));
    //m_spCamera->SetLook(DirectX::XMFLOAT3(0.0f, 2.0f, 5.0f));
    CameraManager::GetInstance().SetCamera(m_spCamera);

    // ライト設定.
    m_pLight->SetDirection(DirectX::XMFLOAT3(1.5f, 1.f, -1.f));
    LightManager::AttachDirectionLight(m_pLight);

}

void AnimationTuningScene::Create()
{
}

void AnimationTuningScene::Update()
{
#if _DEBUG
    // ウィンドウとして独立させる
    ImGui::Begin("d");

    // 1. 基本的なFPS表示
    float fps = ImGui::GetIO().Framerate;
    float ms = 1000.0f / fps;

    ImGui::Text("Average: %.1f FPS (%.3f ms/frame)", fps, ms);

    // 状態に応じた警告表示
    if (fps < 50.0f) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Warning: Low FPS!");
    }
    else {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Stable");
    }

    ImGui::Separator();
    ImGui::End();
#endif // _DEBUG

    Input::Update();

    m_upPlayer->SetTargetPos(m_upBoss.get()->GetPosition());
    //m_upPlayer->Update();
    m_upBoss->Update();

}

void AnimationTuningScene::LateUpdate()
{
    m_upPlayer->LateUpdate();
    m_upBoss->LateUpdate();
    CameraManager::GetInstance().LateUpdate();

    CollisionDetector::GetInstance().ExecuteCollisionDetection();
}


void AnimationTuningScene::Draw()
{
    Shadow::Begin();
    m_upGround->DrawDepth();
    Shadow::End();

    m_upGround->Draw();

    m_upPlayer->Draw();
    m_upBoss->Draw();

    CollisionVisualizer::GetInstance().Draw();
}

HRESULT AnimationTuningScene::LoadData()
{
    return S_OK; // 成功を返す
}
