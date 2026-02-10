#pragma once

#include "../00_Base/SceneBase.h"

class Ground;
class SkyDome;
class Player;
class MeshBase;
class Boss;
class CameraBase;
class DirectionLight;
class UIGameMain;
class UIGameOver;
class UIEnding;

#include <vector>


/*********************************************
*	ゲームメイン画像クラス.
**/

class GameMain
	: public SceneBase
{
public:
	GameMain();
	~GameMain() override;

	void Initialize() override;
	void Create() override;
	void Update() override;
	void LateUpdate() override;
	void Draw() override;

	HRESULT LoadData();


private:
	void UIUpdate();
private:

	std::shared_ptr<CameraBase>			m_spCamera;			// カメラ.
	std::shared_ptr<DirectionLight>		m_spLight;			// ライト

	std::unique_ptr<Ground>		m_upGround;
	std::unique_ptr<SkyDome>	m_upSkyDome;
	
	//一時的な表示用.
	std::unique_ptr<Boss>		m_upBoss;
	std::unique_ptr<Player>		m_upPlayer;

	std::shared_ptr<UIGameMain>	m_upUI;
	std::shared_ptr<UIGameOver> m_upUIOver;
	std::shared_ptr<UIEnding>   m_upUIEnding;
	float m_TimeLimit;
};
