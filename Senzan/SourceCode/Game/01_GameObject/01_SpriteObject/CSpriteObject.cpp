#include "CSpriteObject.h"
#include "Resource/Mesh/00_Sprite/Sprite3D/Sprite3D.h"

CSpriteObject::CSpriteObject()
	: m_pSprite(nullptr)
{
}

CSpriteObject::~CSpriteObject()
{
	DetachSprite();
}

void CSpriteObject::Update()
{
	if (m_pSprite == nullptr) {
		return;
	}
}

void CSpriteObject::LateUpdate()
{
}

void CSpriteObject::Draw()
{
	if (m_pSprite == nullptr) {
		return;
	}

	//描画直前で座標や回転情報などを更新.
	m_pSprite->SetPosition(m_spTransform->Position);
	m_pSprite->SetRotation(m_spTransform->Rotation);
	m_pSprite->SetScale(m_spTransform->Scale);

	//レンダリング.
	m_pSprite->Render();
}
