#pragma once

#include "../../00_Base/SceneBase.h"
#include "System/Utility/Transform/Transform.h"

class Ground;
class Player;
class Boss;
class CameraBase;
class DirectionLight;
class CapsuleCollider;
class UIGameMain;

/*********************************************
*	ゲームメイン画像クラス.
**/

class MattyaTestScene
	: public SceneBase
{
public:
	MattyaTestScene();
	~MattyaTestScene() override;

	void Initialize() override;
	void Create() override;
	void Update() override;
	void LateUpdate() override;
	void Draw() override;

	HRESULT LoadData();

private:

	std::shared_ptr<CameraBase>			m_pCamera;			// カメラ.
	std::shared_ptr<DirectionLight>		m_pLight;			// ライト

	std::unique_ptr<Ground>		m_upGround;
	std::unique_ptr<Player>		m_upPlayer;
	std::unique_ptr<Boss>		m_upBoss;

	std::shared_ptr<UIGameMain>	m_upUI;
};
