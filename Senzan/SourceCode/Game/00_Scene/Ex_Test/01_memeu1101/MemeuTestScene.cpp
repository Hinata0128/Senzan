#include "MemeuTestScene.h"

#include "System/Singleton/CameraManager/CameraManager.h"
#include "Game/02_Camera/CameraBase.h"
#include "Game/02_Camera/ThirdPersonCamera/ThirdPersonCamera.h"
#include "ResourceManager/SpriteManager/SpriteManager.h"

#include "Graphic/Shadow/Shadow.h"
#include "Graphic/Light/DirectionLight/DirectionLight.h"
#include "Graphic/Light/LightManager.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/00_Ground/Ground.h"	// 地面Static.
#include "Game/01_GameObject/02_UIObject/UIGameMain/UIGameMain.h"
#include "Game/01_GameObject/02_UIObject/UIGameOver/UIGameOver.h"
#include "Game/01_GameObject/02_UIObject/UIEnding/UIEnding.h"
#include "SceneManager/SceneManager.h" 
#include "Game/05_InputDevice/Input.h"

#include <algorithm> // std::min のために必要

// コンストラクタ.
MemeuTestScene::MemeuTestScene()
	: SceneBase()
	, m_pCamera     (std::make_shared<ThirdPersonCamera>())
	, m_pLight      (std::make_shared<DirectionLight>())
	, m_TestSprite  (std::make_shared<UIObject>())
	, m_upUIMain    (std::make_shared<UIGameMain>())
	, m_upUIOver    ()
    , m_upUIEnding  ()
	, m_TimeLimit   (600.0f * Time::GetInstance().GetDeltaTime())
{
	Initialize();
	Time::GetInstance().StartTimer(m_TimeLimit);
}

// デストラクタ.
MemeuTestScene::~MemeuTestScene()
{
}

void MemeuTestScene::Initialize()
{
	// カメラ設定.
	m_pCamera->SetPosition(DirectX::XMFLOAT3(0.0f, 5.0f, -5.0f));
	m_pCamera->SetLook(DirectX::XMFLOAT3(0.0f, 2.0f, 5.0f));
	CameraManager::GetInstance().SetCamera(m_pCamera);

	// ライト設定.
	m_pLight->SetDirection(DirectX::XMFLOAT3(1.5f, 1.f, -1.f));
	LightManager::AttachDirectionLight(m_pLight);

	m_pGround = std::make_unique<Ground>();

	SpriteManager::LoadSprites();
}

void MemeuTestScene::Create()
{
}

void MemeuTestScene::Update()
{
	Input::Update();
	m_upUIMain->SetTime(Time::GetInstance().GetTimerProgress());
	m_upUIMain->Update();

	m_pGround->Update();


	if (Time::GetInstance().IsTimerJustFinished()) {
		m_upUIOver = std::make_shared<UIGameOver>();
	}

#if _DEBUG
	if ( m_upUIOver )
    {
        m_upUIOver->Update();
        if (Input::IsKeyDown(VK_SPACE)
            || Input::IsKeyDown('C')
            || Input::IsButtonDown(XInput::Key::B))
        {
            if (m_upUIOver->GetSelected() == m_upUIOver->Items::End) {
                SceneManager::LoadScene(eList::Memeu);
            }
        }
    }
#endif // _DEBUG.
}

void MemeuTestScene::LateUpdate()
{
	CameraManager::GetInstance().LateUpdate();
	m_upUIMain->LateUpdate();

    if (m_upUIEnding) { m_upUIEnding->LateUpdate(); }
	if (m_upUIOver) { m_upUIOver->LateUpdate(); }
}


void MemeuTestScene::Draw()
{
	Shadow::Begin();
	m_pGround->DrawDepth();
	Shadow::End();
	m_pGround->Draw();
	m_upUIMain->Draw();
    if (m_upUIEnding) { m_upUIEnding->Draw(); }
    if (m_upUIOver) { m_upUIOver->Draw(); }
}

HRESULT MemeuTestScene::LoadData()
{
	// ここで実際のロード処理を行うか、Create()に集約されているのであればE_NOTIMPLのままでもよい
	// 現在のGameMainではCreate()でほとんどのInit/Load処理が行われているようです
	return S_OK; // 成功を返す
}
