#pragma once

#include "../../00_Base/SceneBase.h"

class DirectionLight;

class Ground;
class Player;
class Boss;
class CameraBase;
class DirectionLight;
class CapsuleCollider;
class UIGameMain;

/*********************************************
*\tアニメーション調整用シーンクラス.
*********************************************/
class AnimationTuningScene
	: public SceneBase
{
public:
	AnimationTuningScene();
	~AnimationTuningScene() override;

	void Initialize() override;
	void Create() override;
	void Update() override;
	void LateUpdate() override;
	void Draw() override;

	HRESULT LoadData();

private:
	std::shared_ptr<DirectionLight> m_pLight;
	std::shared_ptr<CameraBase> m_spCamera;

    std::unique_ptr<Ground>		m_upGround;
    std::unique_ptr<Player>		m_upPlayer;
    std::unique_ptr<Boss>		m_upBoss;
};
