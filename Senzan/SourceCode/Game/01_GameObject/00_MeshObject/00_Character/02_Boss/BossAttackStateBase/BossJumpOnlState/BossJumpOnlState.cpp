#include "BossJumpOnlState.h"
#include "Game/04_Time/Time.h"
#include "00_MeshObject/00_Character/02_Boss/Boss.h"
#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"
#include <DirectXMath.h>
#include <algorithm>

using namespace DirectX;

BossJumpOnlState::BossJumpOnlState(Boss* owner)
	: BossAttackStateBase(owner)
	, m_List(enSpecial::None)
	, m_Velocity(0.0f, 0.0f, 0.0f)
	, m_SpecialPower(1.5f)
	, m_Gravity(0.089f)
	, m_SpecialFrag(false)
	, m_GroundedFrag(true)
	, m_Timer(0.0f)
	, m_TransitionTimer(5.0f)
	, m_AttackTimer(0.0f)
	, m_UpSpeed(0.15f)
	, m_TargetDirection(0.0f, 0.0f, 0.0f)
	, m_MaxTrackingAngle(3.0f)
	, m_AttackMoveSpeed(80.5f)
	, m_AttackDistance(9999.0f)
	, m_DistanceTraveled(0.0f)
    , m_RiseHeight(6.0f)
    , m_RiseSpeed(120.0f)
    , m_ReappearDelay(1.6f)
    , m_FallSpeed(80.0f)
    , m_RiseTargetY(0.0f)
    , m_RiseStarted(false)
    , m_IsFalling(false)
    , m_WaitingReappear(false)
    , m_HasPlayedPreFallEffect(false)
    , m_PreFallSeconds(0.5f)
{
}

void BossJumpOnlState::DrawImGui()
{
#if _DEBUG
    if (!ImGui::Begin(IMGUI_JP("Boss JumpOn State"))) { ImGui::End(); return; }

    ImGui::Text(IMGUI_JP("JumpOn Parameters"));
    ImGui::SliderFloat(IMGUI_JP("上昇量"), &m_RiseHeight, 0.0f, 20.0f);
    ImGui::SliderFloat(IMGUI_JP("上昇速度"), &m_RiseSpeed, 0.1f, 50.0f);
    ImGui::SliderFloat(IMGUI_JP("再出現遅延"), &m_ReappearDelay, 0.0f, 5.0f);
    ImGui::SliderFloat(IMGUI_JP("落下速度"), &m_FallSpeed, 1.0f, 500.0f);
    ImGui::SliderFloat(IMGUI_JP("落下モード前の秒数"), &m_PreFallSeconds, 0.0f, 5.0f);

    if (ImGui::Button(IMGUI_JP("即座に実行(デバッグ)"))) {
        // デバッグ用: すぐに上昇→消失→落下を強制する
        m_List = enSpecial::Jump;
        m_RiseStarted = false;
        m_Timer = 0.0f;
        DirectX::XMFLOAT3 pos = m_pOwner->GetPosition();
        m_RiseTargetY = pos.y + m_RiseHeight;
    }

    ImGui::End();
#endif
}

BossJumpOnlState::~BossJumpOnlState()
{
}

void BossJumpOnlState::Enter()
{
	m_Timer = 0.0f;
	m_Velocity = {};
	m_DistanceTraveled = 0.0f;
	m_GroundedFrag = true;
    m_RiseStarted = false;
    m_IsFalling = false;
    Isfast = false;
    m_HasPlayedPreFallEffect = false;
    m_pOwner->SetAnimSpeed(3.0);
    m_pOwner->ChangeAnim(Boss::enBossAnim::Special_0);
}

void BossJumpOnlState::Update()
{
	float deltaTime = m_pOwner->GetDelta();

	switch (m_List)
	{
	case BossJumpOnlState::enSpecial::None:
		m_Velocity = {};
		// 当たり判定を有効化.
		m_List = enSpecial::Charge;
		break;

	case BossJumpOnlState::enSpecial::Charge:
		ChargeTime();
		break;

	case BossJumpOnlState::enSpecial::Jump:
		JumpTime();
		break;

	case BossJumpOnlState::enSpecial::Attack:
		BossAttack();
		break;

	case BossJumpOnlState::enSpecial::CoolTime:
		m_Timer += deltaTime;

        if (auto* col = m_pOwner->GetLaserCollider()) {
            col->SetActive(false);
        }

		// SpecialToIdolアニメーションが終了したら遷移
		if (m_pOwner->IsAnimEnd(Boss::enBossAnim::LaserEnd))
		{
			m_List = enSpecial::Trans;
		}
		break;

	case BossJumpOnlState::enSpecial::Trans:
		m_pOwner->GetStateMachine()->ChangeState(std::make_shared<BossIdolState>(m_pOwner));
		break;

	default:
		break;
	}
}

void BossJumpOnlState::LateUpdate()
{
}

void BossJumpOnlState::Draw()
{
}

void BossJumpOnlState::Exit()
{
    m_GroundedFrag = true;
    m_SpecialFrag = false;
    m_AttackTimer = 0.0f;

    m_pOwner->SetIsRenderActive(true);

    // 終了時にプレイヤーの方を向き直す
    const DirectX::XMFLOAT3 BossPosF = m_pOwner->GetPosition();
    DirectX::XMVECTOR BossPosXM = DirectX::XMLoadFloat3(&BossPosF);

    const DirectX::XMFLOAT3 PlayerPosF = m_pOwner->GetTargetPos();
    DirectX::XMVECTOR PlayerPosXM = DirectX::XMLoadFloat3(&PlayerPosF);

    DirectX::XMVECTOR Direction = DirectX::XMVectorSubtract(PlayerPosXM, BossPosXM);
    Direction = DirectX::XMVectorSetY(Direction, 0.0f);

    if (XMVectorGetX(XMVector3LengthSq(Direction)) > 0.0001f)
    {
        float dx = DirectX::XMVectorGetX(Direction);
        float dz = DirectX::XMVectorGetZ(Direction);
        // プレイヤーの方向を正面にする
        float angle_radian = std::atan2f(dx, dz) + XM_PI;
        m_pOwner->SetRotationY(angle_radian);
    }
}

std::pair<Boss::enBossAnim, float> BossJumpOnlState::GetParryAnimPair()
{
    return std::pair(Boss::enBossAnim::none, 0.0f);
}

void BossJumpOnlState::ChargeTime()
{
	float deltaTime = m_pOwner->GetDelta();
	m_Timer += deltaTime;

	if (m_Timer > m_AttackTimer)
	{
		m_Timer = 0.0f;
		m_Velocity = { 0.0f, 0.0f, 0.0f };
        // 溜め完了: 溜めモーション -> 上昇処理 (Jump フェーズ)
        // 設定: 上昇目標Y を現在のY + m_RiseHeight
        DirectX::XMFLOAT3 pos = m_pOwner->GetPosition();
        m_RiseTargetY = pos.y + m_RiseHeight;
        m_RiseStarted = false;
        m_List = enSpecial::Jump;
	}
}

void BossJumpOnlState::JumpTime()
{
 
    // Jump フェーズ: Special_0 を最初に再生し、0.9 秒後に上昇を開始。上昇後は一旦消えて delay の後プレイヤー真上から登場
    float deltaTime = m_pOwner->GetDelta();

    if (!m_RiseStarted && !m_WaitingReappear && !m_IsFalling)
    {
        // 初回: Special_0 を再生し、0.9 秒待って上昇開始
        // 再生開始フラグを立ててアニメ再生
        // カウントを使って 0.9 秒待つ
        m_Timer += deltaTime;
        if (m_Timer >= 0.9f)
        {
            m_Timer = 0.0f;
            m_RiseStarted = true;
            // rise target は現在位置 + height
            DirectX::XMFLOAT3 pos = m_pOwner->GetPosition();
            m_RiseTargetY = pos.y + m_RiseHeight;
            SoundManager::GetInstance().Play("Jump", false);
            SoundManager::GetInstance().SetVolume("Jump", 9500);
        }
        return;
    }

    // 上昇中
    if (m_RiseStarted && !m_WaitingReappear)
    {
        DirectX::XMFLOAT3 pos = m_pOwner->GetPosition();
        if (pos.y < m_RiseTargetY)
        {
            pos.y += m_RiseSpeed * deltaTime;
            if (pos.y > m_RiseTargetY) pos.y = m_RiseTargetY;
            m_pOwner->SetPosition(pos);

            // 上昇中は常にプレイヤー方向を向く（Yaw のみ）
            {
                DirectX::XMFLOAT3 BossPosF = m_pOwner->GetPosition();
                DirectX::XMVECTOR BossPosXM = DirectX::XMLoadFloat3(&BossPosF);
                DirectX::XMFLOAT3 PlayerPosF = m_pOwner->GetTargetPos();
                DirectX::XMVECTOR PlayerPosXM = DirectX::XMLoadFloat3(&PlayerPosF);
                DirectX::XMVECTOR Direction = DirectX::XMVectorSubtract(PlayerPosXM, BossPosXM);
                Direction = DirectX::XMVectorSetY(Direction, 0.0f);
                if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(Direction)) > 0.0001f)
                {
                    float dx = DirectX::XMVectorGetX(Direction);
                    float dz = DirectX::XMVectorGetZ(Direction);
                    float angle_radian = std::atan2f(dx, dz);
                    m_pOwner->SetRotationY(angle_radian);
                }
            }
            return;
        }

        // 上昇完了: 一旦不可視化して再出現待ちに入る
        m_pOwner->SetIsRenderActive(false);
        m_WaitingReappear = true;
        m_HasPlayedPreFallEffect = false;
        m_Timer = 0.0f;
        return;
    }

    // 再出現待ち: delay 経過後にプレイヤー真上から出現
    if (m_WaitingReappear && !m_IsFalling)
    {
        m_Timer += deltaTime;
        // 再出現待ちの間もプレイヤー方向を向き続ける（Yaw のみ）
        {
            DirectX::XMFLOAT3 BossPosF = m_pOwner->GetPosition();
            DirectX::XMVECTOR BossPosXM = DirectX::XMLoadFloat3(&BossPosF);
            DirectX::XMFLOAT3 PlayerPosF = m_pOwner->GetTargetPos();
            DirectX::XMVECTOR PlayerPosXM = DirectX::XMLoadFloat3(&PlayerPosF);
            DirectX::XMVECTOR Direction = DirectX::XMVectorSubtract(PlayerPosXM, BossPosXM);
            Direction = DirectX::XMVectorSetY(Direction, 0.0f);
            if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(Direction)) > 0.0001f)
            {
                float dx = DirectX::XMVectorGetX(Direction);
                float dz = DirectX::XMVectorGetZ(Direction);
                float angle_radian = std::atan2f(dx, dz) + XM_PI;
                m_pOwner->SetRotationY(angle_radian);
            }
        }
        // 落下モードに入る m_PreFallSeconds 秒前のエフェクト再生
        float prePlayTime = std::max(0.0f, m_ReappearDelay - m_PreFallSeconds);
        if (!m_HasPlayedPreFallEffect && m_Timer >= prePlayTime)
        {
            // プレイヤーの真上に出現してから落下するので、エフェクトはプレイヤー位置で再生
            DirectX::XMFLOAT3 effectPos = m_pOwner->GetTargetPos();
            m_pOwner->PlayEffectAtWorldPos("BossJumpUp", effectPos, 5.f);
            m_HasPlayedPreFallEffect = true;
        }
        if (m_Timer >= m_ReappearDelay)
        {
            DirectX::XMFLOAT3 playerPos = m_pOwner->GetTargetPos();
            // 着地点をプレイヤーの位置からボスの方へ10.f移動させた位置に設定
            DirectX::XMFLOAT3 prevBossPos = m_pOwner->GetPosition();
            DirectX::XMVECTOR BossPosXM_forLanding = DirectX::XMLoadFloat3(&prevBossPos);
            DirectX::XMVECTOR PlayerPosXM_forLanding = DirectX::XMLoadFloat3(&playerPos);
            DirectX::XMVECTOR toBoss = DirectX::XMVectorSubtract(BossPosXM_forLanding, PlayerPosXM_forLanding);
            toBoss = DirectX::XMVectorSetY(toBoss, 0.0f);
            DirectX::XMFLOAT3 landingPos = playerPos;
            if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(toBoss)) > 0.0001f)
            {
                DirectX::XMVECTOR dirToBoss = DirectX::XMVector3Normalize(toBoss);
                DirectX::XMVECTOR offset = DirectX::XMVectorScale(dirToBoss, 10.0f);
                DirectX::XMVECTOR landingXM = DirectX::XMVectorAdd(PlayerPosXM_forLanding, offset);
                DirectX::XMStoreFloat3(&landingPos, landingXM);
            }

            DirectX::XMFLOAT3 spawnPos = landingPos;
            spawnPos.y += 12.0f; // 十分上に出現
            // 出現時にプレイヤーの方を向く（Yaw のみ）
            // 出現前のボス位置とプレイヤー位置のXZで向きを計算する
            DirectX::XMVECTOR PrevBossPosXM = DirectX::XMLoadFloat3(&prevBossPos);
            DirectX::XMVECTOR PlayerPosXM = DirectX::XMLoadFloat3(&playerPos);
            DirectX::XMVECTOR Dir = DirectX::XMVectorSubtract(PlayerPosXM, PrevBossPosXM);
            Dir = DirectX::XMVectorSetY(Dir, 0.0f);
            if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(Dir)) > 0.0001f)
            {
                float dx = DirectX::XMVectorGetX(Dir);
                float dz = DirectX::XMVectorGetZ(Dir);
                float angle_radian = std::atan2f(dx, dz) + XM_PI;
                m_pOwner->SetRotationY(angle_radian);
            }
            m_pOwner->SetPosition(spawnPos);
            m_pOwner->SetIsRenderActive(true);
            m_IsFalling = true;
            // 落下開始時にプレイヤーの方向を向く（基底のヘルパーを使用）
            FacePlayerInstantYaw();
            m_HasPlayedPreFallEffect = false;
            m_WaitingReappear = false;
            m_RiseStarted = false;
            m_Timer = 0.0f;
            m_pOwner->ChangeAnim(Boss::enBossAnim::Special_1);
            m_List = enSpecial::Attack;
        }
        return;
    }
}

void BossJumpOnlState::BossAttack()
{
	float deltaTime = m_pOwner->GetDelta();
	const float floorY = 0.0f;
	const float PlayerYOffset = 1.0f;
	const float One = 1.0f;
	const float MinusOne = -1.0f;

    DirectX::XMFLOAT3 CurrentPos = m_pOwner->GetPosition();
    DirectX::XMFLOAT3 TargetPosF = m_pOwner->GetTargetPos();
	TargetPosF.y += PlayerYOffset;

    DirectX::XMVECTOR BossPosVec = XMLoadFloat3(&CurrentPos);
    DirectX::XMVECTOR PlayerPosVec = XMLoadFloat3(&TargetPosF);

	// 距離判定と高さ判定による終了処理
	float DistanceToPlayer = DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMVectorSubtract(PlayerPosVec, BossPosVec)));

	// 修正：プレイヤーとの距離が近い(2.5f以下)か、地面に付いたら攻撃終了
	if (CurrentPos.y <= floorY + 0.1f || DistanceToPlayer <= 2.5f)
	{
		CurrentPos.y = 0.f;
		m_pOwner->SetPosition(CurrentPos);
		m_Timer = 0.0f;
		m_pOwner->SetAnimSpeed(3.0);
		m_pOwner->ChangeAnim(Boss::enBossAnim::LaserEnd);
		m_List = enSpecial::CoolTime;
		return;
	}

    // Attack フェーズは、JumpOn の場合は落下処理
    if (m_IsFalling)
    {
        if (auto* col = m_pOwner->GetLaserCollider()) {
            if (!Isfast)
            {
                Isfast = true;
                col->SetActive(true);
                col->SetAttackAmount(2.5f);
                col->SetPositionOffset(0.f, 0.f, 0.f);
            }
        }
        // 真っ直ぐ下に落下
        CurrentPos.y -= m_FallSpeed * deltaTime;
        if (CurrentPos.y <= floorY)
        {
            CurrentPos.y = floorY;
            m_pOwner->SetPosition(CurrentPos);
            // 着地エフェクトやダメージ判定などをここで実行
            m_pOwner->PlayEffectAtWorldPos("boss_stomp", CurrentPos);
            m_pOwner->SetAnimSpeed(3.0);
            m_pOwner->ChangeAnim(Boss::enBossAnim::LaserEnd);
            m_List = enSpecial::CoolTime;
            m_IsFalling = false;
            SoundManager::GetInstance().Play("Landing", false);
            SoundManager::GetInstance().SetVolume("Landing", 10000);
            return;
        }
        m_pOwner->SetPosition(CurrentPos);
        return;
    }

    // 追尾（ホーミング）計算
    DirectX::XMVECTOR CurrentDir = DirectX::XMLoadFloat3(&m_TargetDirection);
    DirectX::XMVECTOR finalMoveDir;

	float trackingThreshold = 15.0f;

	if (DistanceToPlayer > trackingThreshold)
	{
        DirectX::XMVECTOR ToPlayerDir = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(PlayerPosVec, BossPosVec));

		float lerpFactor = 1.0f * deltaTime;
        DirectX::XMVECTOR TargetStepDir = DirectX::XMVectorLerp(CurrentDir, ToPlayerDir, lerpFactor);
		TargetStepDir = DirectX::XMVector3Normalize(TargetStepDir);

        DirectX::XMVECTOR DotProduct = DirectX::XMVector3Dot(CurrentDir, TargetStepDir);
		float cosTheta = DirectX::XMVectorGetX(DotProduct);
		float AngleRad = acosf(std::max(MinusOne, std::min(One, cosTheta)));

		float maxTurnRadians = DirectX::XMConvertToRadians(m_MaxTrackingAngle) * deltaTime;

		if (AngleRad > maxTurnRadians)
		{
			XMVECTOR rotationAxis = DirectX::XMVector3Cross(CurrentDir, TargetStepDir);
			if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(rotationAxis)) > 0.0001f)
			{
				rotationAxis = DirectX::XMVector3Normalize(rotationAxis);
                DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationAxis(rotationAxis, maxTurnRadians);
				finalMoveDir = DirectX::XMVector3TransformNormal(CurrentDir, rotationMatrix);
			}
			else { finalMoveDir = CurrentDir; }
		}
		else { finalMoveDir = TargetStepDir; }
	}
	else
	{
		// 一定距離内ならホーミングを停止し、慣性で突っ込む
		finalMoveDir = CurrentDir;
	}

	finalMoveDir = DirectX::XMVector3Normalize(finalMoveDir);
	DirectX::XMStoreFloat3(&m_TargetDirection, finalMoveDir);

	// 移動実行
    DirectX::XMVECTOR moveVector = DirectX::XMVectorScale(finalMoveDir, m_AttackMoveSpeed * deltaTime);
    DirectX::XMVECTOR newBossPosVec = DirectX::XMVectorAdd(BossPosVec, moveVector);

    DirectX::XMFLOAT3 newBossPos;
    DirectX::XMStoreFloat3(&newBossPos, newBossPosVec);

	// 地面を突き抜けないように制限
	if (newBossPos.y < floorY) newBossPos.y = floorY;

	m_pOwner->SetPosition(newBossPos);
	m_DistanceTraveled += m_AttackMoveSpeed * deltaTime;
}
