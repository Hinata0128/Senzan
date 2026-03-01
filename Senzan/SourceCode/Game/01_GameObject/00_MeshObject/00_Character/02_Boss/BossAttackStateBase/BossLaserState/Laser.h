#pragma once
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossAttackStateBase.h"


class BossIdolState;

class Laser
	: public BossAttackStateBase
{
public:
	enum class enAnimChange : byte
	{
		Charge,		//ため中.
		Attack,		//ため攻撃発射中.
		ChargeEnd,	//ため攻撃から待機.

		none,		//何もしない.
	};
public:
	Laser(Boss* owner);
	~Laser() override;

	void Enter() override;
	void Update() override;
	void LateUpdate() override;
	void Draw() override;
	void Exit() override;
private:
	std::shared_ptr<BossIdolState> m_pBossIdol;

	//遷移のタイミング取得用のタイム変数.
	float m_Timer = 0.0f;

	//遷移の時間のタイム変数.
	float m_TransitionTimer = 120.0f;

	enAnimChange m_AnimChange;
};
