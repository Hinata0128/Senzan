#include "BossIdolState.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/Boss.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/BossMoveState/BossMoveState.h"



BossIdolState::BossIdolState(Boss* owner)
	: StateBase<Boss>(owner)
	, m_AnimSpeed	(25.0f)
{
}

BossIdolState::~BossIdolState()
{
}

void BossIdolState::Enter()
{
	m_pOwner->SetIsLoop(true);
	m_pOwner->SetAnimSpeed(m_AnimSpeed);
	//待機アニメションを再生.
	m_pOwner->ChangeAnim(Boss::enBossAnim::Idol);
}

void BossIdolState::Update()
{
    // 1. ボスとプレイヤーの座標を取得
    const DirectX::XMFLOAT3& BossPosF = m_pOwner->GetPosition();
    DirectX::XMVECTOR BossPosXM = DirectX::XMLoadFloat3(&BossPosF);

    const DirectX::XMFLOAT3& PlayerPosF = m_pOwner->m_PlayerPos;
    DirectX::XMVECTOR PlayerPosXM = DirectX::XMLoadFloat3(&PlayerPosF);

    // --------------------------------------------------------
    // 2. プレイヤーの方を向く処理 (常に実行)
    // --------------------------------------------------------
    // ボスからプレイヤーへのベクトルを計算
    DirectX::XMVECTOR LookAtVec = DirectX::XMVectorSubtract(PlayerPosXM, BossPosXM);

    float dx = DirectX::XMVectorGetX(LookAtVec);
    float dz = DirectX::XMVectorGetZ(LookAtVec);

    // 前回の修正を反映：モデルが逆を向く場合は -dx, -dz にする
    // もしこれで正解ならこのまま、まだ逆なら dx, dz に戻してください
    float angle_radian = std::atan2f(-dx, -dz);

    m_pOwner->SetRotationY(angle_radian);
    // --------------------------------------------------------

    // 3. 距離の判定
    DirectX::XMVECTOR DistancePosXM = DirectX::XMVectorSubtract(PlayerPosXM, BossPosXM);
    float DistanceSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DistancePosXM));


    // 既に上で回転は計算しているので、initalAngleとして再利用可能
    auto MoveState = std::make_shared<BossMoveState>(m_pOwner);
    // MoveState->SetInitialAngle(angle_radian); // 必要であれば渡す

    m_pOwner->GetStateMachine()->ChangeState(MoveState);
    return;

}

void BossIdolState::LateUpdate()
{
}

void BossIdolState::Draw()
{
	DrawBone();
}

void BossIdolState::Exit()
{
}

void BossIdolState::DrawBone()
{
	// メッシュとボーンのチェック.
	if (m_pOwner->GetAttachMesh().expired()) return;

	std::shared_ptr<SkinMesh> staticMesh = std::dynamic_pointer_cast<SkinMesh>(m_pOwner->GetAttachMesh().lock());
	if (!staticMesh) return;

	const std::string TargetBoneName = "Bone002";

	DirectX::XMFLOAT3 BonePos{};
	if (staticMesh->GetPosFromBone(TargetBoneName.c_str(), &BonePos))
	{
		DirectX::XMFLOAT3 ForWard = m_pOwner->GetTransform()->GetForward();
		float OffSetDist = 0.0f;
		DirectX::XMVECTOR FwdVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&ForWard));

		DirectX::XMVECTOR BoneVec = DirectX::XMLoadFloat3(&BonePos);
		DirectX::XMVECTOR OffSetPos = DirectX::XMVectorAdd(BoneVec, DirectX::XMVectorScale(FwdVec, OffSetDist));

		XMStoreFloat3(&BonePos, OffSetPos);
		BonePos.y = 2.5f;

		m_pOwner->GetTransform()->SetPosition(BonePos);
	}
	else return;

	// m_pColl->Draw(transform);
}
