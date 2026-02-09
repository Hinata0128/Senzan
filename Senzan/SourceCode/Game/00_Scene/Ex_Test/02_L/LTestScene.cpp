#include "LTestScene.h"

#include "Game/05_InputDevice/Input.h"
#include "Game/05_InputDevice/VirtualPad.h" 

#include "System/Singleton/CameraManager/CameraManager.h"
#include "Game/02_Camera/CameraBase.h"
#include "Game/02_Camera/ThirdPersonCamera/ThirdPersonCamera.h"
#include "Game/02_Camera/ThirdPersonCamera/PlayerThirdPersonCamera.h"

#include "Graphic/Shadow/Shadow.h"
#include "Graphic/Light/DirectionLight/DirectionLight.h"
#include "Graphic/Light/LightManager.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/00_Ground/Ground.h"	// 地面Static.

#include "Game//01_GameObject//00_MeshObject//00_Character//02_Boss//Boss.h"
#include "Game//01_GameObject//00_MeshObject//00_Character//01_Player//Player.h"

#include "Game/03_Collision/00_Core/01_Capsule/CapsuleCollider.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossSlashState/BossSlashState.h"

#include "System/Singleton/ImGui/CImGuiManager.h"

#include "System/Singleton/SceneManager/SceneManager.h"

#include "00_MeshObject/00_Character/03_SnowBall/SnowBall.h"

#include "00_MeshObject/00_Character/04_BackGround/BackGround.h"



#include <algorithm> // std::min のために必要


// コンストラクタ.
LTestScene::LTestScene()
	: SceneBase()
	, m_pCamera()
	, m_pLight(std::make_shared<DirectionLight>())
	, m_pPlayer(std::make_unique<Player>())
	, m_ptransform(std::make_shared<Transform>())

	, m_pSnowBall(std::make_shared<SnowBall>())

	, m_pSkyBox(std::make_unique<BackGround>())
{
	Initialize();
}

// デストラクタ.
LTestScene::~LTestScene()
{
}

void LTestScene::Initialize()
{
	// カメラ設定.
	m_pCamera = std::make_shared<PlayerThirdPersonCamera>(*m_pPlayer.get());
	m_pCamera->SetPosition(DirectX::XMFLOAT3(0.0f, 5.0f, -30.0f));
	m_pCamera->SetPosition(DirectX::XMFLOAT3(0.0f, 5.0f, -5.0f));
	m_pCamera->SetLook(DirectX::XMFLOAT3(0.0f, 2.0f, 5.0f));
	CameraManager::GetInstance().SetCamera(m_pCamera);

	// ライト設定.
	m_pLight->SetDirection(DirectX::XMFLOAT3(1.5f, 1.f, -1.f));
	LightManager::AttachDirectionLight(m_pLight);

	m_pGround = std::make_unique<Ground>();
	m_pBoss = std::make_unique<Boss>();
}

void LTestScene::Create()
{
}

void LTestScene::Update()
{
	Input::Update();
	m_pGround->Update();
	m_pPlayer->Update();
	m_pBoss->SetTargetPos(m_pPlayer->GetPosition());
	m_pCamera->SetLook(m_pPlayer->GetPosition());
	m_pCamera->Update();
	m_pBoss->Update();
}

void LTestScene::LateUpdate()
{
	m_pPlayer->LateUpdate();

	CameraManager::GetInstance().LateUpdate();
}


void LTestScene::Draw()
{
	Shadow::Begin();
	m_pSkyBox->DrawDepth();
	m_pGround->DrawDepth();
	Shadow::End();
	m_pSkyBox->Draw();
	m_pGround->Draw();

	m_pBoss->Draw();
	m_pSnowBall->Draw();

	m_pPlayer->Draw();
}

HRESULT LTestScene::LoadData()
{
	// このメソッドは今回は使いませんが、SceneBaseの規定通りS_OKを返します。
	return S_OK;
}
