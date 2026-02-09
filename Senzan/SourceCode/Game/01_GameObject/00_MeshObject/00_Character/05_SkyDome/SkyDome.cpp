#include "SkyDome.h"

SkyDome::SkyDome()
    : MeshObject()
{
    // メッシュのアタッチ.
    auto mesh = ResourceManager::GetStaticMesh("1");
    _ASSERT_EXPR(mesh != nullptr, "メッシュの取得に失敗");
    AttachMesh(mesh);
    // スケールの設定.
    m_spTransform->Scale = { 10.0f, 10.0f, 10.0f };
    m_IsLight = false;
    m_IsShadow = false;
}

SkyDome::~SkyDome()
{
}

void SkyDome::Update()
{
    MeshObject::Update();
}

void SkyDome::Draw()
{
    MeshObject::Draw();
}
