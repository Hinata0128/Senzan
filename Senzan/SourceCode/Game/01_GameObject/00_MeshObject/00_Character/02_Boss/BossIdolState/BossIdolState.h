#pragma once

//基底クラス.
#include "System//Utility//StateMachine//StateBase.h"
#include "Game\03_Collision\00_Core\01_Capsule\CapsuleCollider.h"


#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossJumpOnlState/BossJumpOnlState.h"

#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossSlashState/BossSlashState.h"
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossShoutState/BossShoutState.h"
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossStompState/BossStompState.h"
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossThrowingState/BossThrowingState.h"

//前方宣言.
class Boss;
class BossMoveState;

/******************************************************************************
*	ボスの待機状態クラス.
**/

class BossIdolState final
	//ボスクラスを使用できるようになる.
	: public StateBase<Boss>
{
public:
	BossIdolState(Boss* owner);
	~BossIdolState();

	//最初に入る.
	void Enter() override;
	//動作.
	void Update() override;
	//かかなくていい.
	void LateUpdate() override;
	//描画.
	void Draw() override;
	//終わるときに一回だけ入る.
	void Exit() override;

	void DrawBone();

private:
	//	std::shared_ptr<CapsuleCollider> m_pColl;

	int							m_AnimNo;	//アニメーション番号.
	double						m_AnimSpeed;// アニメーション速度.
	double						m_AnimTimer;// アニメ―ション経過時間.
	DirectX::XMFLOAT3			m_BonePos;	// ボーン座標.

};
