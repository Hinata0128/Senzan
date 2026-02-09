#pragma once

#include "../01_GameObject/GameObject.h"

class Sprite3D;
class SpriteManager;

/************************************************************
*	スプライトオブジェクトクラス.
**/
class CSpriteObject
	: public GameObject
{
public:
	CSpriteObject();
	virtual ~CSpriteObject() override;

	virtual void Update() override;
	virtual void LateUpdate() override;
	virtual void Draw();

	//スプライトを接続する.
	void AttachSprite( Sprite3D& pSprite ){
		m_pSprite = &pSprite;
	}
	//スプライトを切り離す.
	void DetachSprite(){
		m_pSprite = nullptr;
	}

protected:
	Sprite3D*	m_pSprite;
};
