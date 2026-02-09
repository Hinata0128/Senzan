#include "BackGround.h"

BackGround::BackGround()
	: MeshObject()
{
	//SeyBoxメッシュのアタッチ.
	auto mesh = ResourceManager::GetStaticMesh("SkyBox");
	_ASSERT_EXPR(mesh != nullptr, "メッシュの取得に失敗");
	AttachMesh(mesh);
	//スケールの設定.
	m_spTransform->Scale = { 1000.0f, 1000.0f, 1000.0f };
	m_IsLight = false;
	m_IsShadow = false;
}

BackGround::~BackGround()
{
}

void BackGround::Update()
{
	MeshObject::Update();
}

void BackGround::Draw()
{
	MeshObject::Draw();
}
