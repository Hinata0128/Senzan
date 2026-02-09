#pragma once
#include "System/Utility/StateMachine/StateBase.h"

class Boss;

/*********************************************************************
*	BossDeadClass
**/

class BossDeadState final
	: public StateBase<Boss>
{
public:
	BossDeadState(Boss* owner);
	~BossDeadState();

	//Å‰‚É“ü‚é.
	void Enter() override;
	//“®ì.
	void Update() override;
	//‚©‚©‚È‚­‚Ä‚¢‚¢.
	void LateUpdate() override;
	//•`‰æ.
	void Draw() override;
	//I‚í‚é‚Æ‚«‚Éˆê‰ñ‚¾‚¯“ü‚é.
	void Exit() override;

private:
};