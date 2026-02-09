#include "Ground.h"

Ground::Ground()
	: MeshObject()
{
	// メッシュのアタッチ.
	auto mesh = ResourceManager::GetStaticMesh("Ground");
	_ASSERT_EXPR(mesh != nullptr, "メッシュの取得に失敗");
	AttachMesh(mesh);
	// スケールの設定.
	m_spTransform->Scale = { 100.0f, 1.0f, 100.0f };
	m_IsLight = false;
	m_IsShadow = false;
}

Ground::~Ground()
{
}

void Ground::Update()
{
	MeshObject::Update();
}

void Ground::Draw()
{
	MeshObject::Draw();
}
