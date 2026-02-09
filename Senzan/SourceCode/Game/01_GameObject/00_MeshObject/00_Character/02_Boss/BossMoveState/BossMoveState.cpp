#include "BossMoveState.h"
#include "Game/04_Time/Time.h"
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossJumpOnlState/BossJumpOnlState.h"
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossSlashState/BossSlashState.h"
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossShoutState/BossShoutState.h"
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossStompState/BossStompState.h"
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossThrowingState/BossThrowingState.h"
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossSpinningState/BossSpinningState.h"
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossLaserState/BossLaserState.h"
#include "00_MeshObject/00_Character/02_Boss/BossAttackStateBase/BossMoveContinueState/BossMoveContinueState.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include <algorithm>
#include <random>
#include <Windows.h>

static constexpr double Move_Run_AnimSpeed = 5.0;

BossMoveState::BossMoveState(Boss* owner)
	: StateBase<Boss>(owner)
	, m_RotationAngle(0.0f)
	, m_RotationSpeed(0.1f) 
	, m_rotationDirection(1.0f)
	, m_AnimNo()
	, m_AnimTimer()
	, m_BonePos()
	, m_InitBossPos()
	, m_Phase(MovePhase::Start)
{
}

BossMoveState::~BossMoveState()
{
}

void BossMoveState::Enter()
{
	m_Timer = 0.0f;
	m_RotationAngle = 0.0f;
	m_rotationDirection = 1.0f;
	m_Phase = MovePhase::Start;

	m_pOwner->SetAnimSpeed(Move_Run_AnimSpeed);
	m_pOwner->ChangeAnim(Boss::enBossAnim::IdolToRun);

    // Load persisted settings
    LoadSettings();
}

void BossMoveState::Update()
{
	using namespace DirectX;

	// === ImGui Debug Window ===
#if _DEBUG
    ImGui::Begin(IMGUI_JP("Boss Attack Debug"));

    ImGui::Text(IMGUI_JP("=== Distance Settings ==="));
    ImGui::SliderFloat(IMGUI_JP("Near Range"), &s_NearRange, 5.0f, 50.0f);
    ImGui::SliderFloat(IMGUI_JP("Mid Range"), &s_MidRange, 20.0f, 80.0f);
    ImGui::SliderFloat(IMGUI_JP("Attack Delay(sec)"), &s_AttackDelay, 0.1f, 5.0f);

    ImGui::Separator();
    ImGui::Text(IMGUI_JP("=== Attack Settings ==="));
    // 各攻撃の UI: 個別に IMGUI_JP マクロで日本語ラベルを指定（動的文字列に IMGUI_JP は使えないため）
    // Distance view selector for ImGui (0:Near,1:Mid)
    static int viewDistance = 0;
    const char* distLabels[] = { IMGUI_JP("近距離"), IMGUI_JP("中距離") };
    ImGui::Combo(IMGUI_JP("表示距離"), &viewDistance, distLabels, IM_ARRAYSIZE(distLabels));

    // 合計での正規化表示用に現在表示している距離の割合を常に表示する
    int curDist = viewDistance;

    // 表示用にパーセンテージに正規化しておく（UIは常に最新の割合を表示する）
    float displayWeight[Count];
    {
        float sum = 0.0f;
        for (int k = 0; k < Count; ++k) sum += s_Weight[curDist][k];
        if (sum > 1e-6f)
        {
            for (int k = 0; k < Count; ++k) displayWeight[k] = s_Weight[curDist][k] / sum * 100.0f;
        }
        else
        {
            for (int k = 0; k < Count; ++k) displayWeight[k] = 100.0f / static_cast<float>(Count);
        }
    }

    // 重みが合計100を超えた場合、直前に操作したスライダー以外の値を調整して合計を100に収める（表示配列用）
    auto normalizeDisplayWeights = [&](int changedIndex){
        float total = 0.0f;
        for (int k = 0; k < Count; ++k) total += displayWeight[k];
        if (total <= 100.0f) return;
        float excess = total - 100.0f;
        float sumOthers = total - displayWeight[changedIndex];
        if (sumOthers > 1e-6f)
        {
            for (int k = 0; k < Count; ++k)
            {
                if (k == changedIndex) continue;
                float reduction = (displayWeight[k] / sumOthers) * excess;
                displayWeight[k] -= reduction;
                if (displayWeight[k] < 0.0f) displayWeight[k] = 0.0f;
            }
        }
        else
        {
            displayWeight[changedIndex] = 100.0f;
        }
        // 再補正（丸め誤差対策）
        float newTotal = 0.0f;
        for (int k = 0; k < Count; ++k) newTotal += displayWeight[k];
        if (newTotal > 100.0f + 1e-4f)
        {
            float remainExcess = newTotal - 100.0f;
            if (displayWeight[changedIndex] > remainExcess)
            {
                displayWeight[changedIndex] -= remainExcess;
            }
            else
            {
                displayWeight[changedIndex] = 0.0f;
            }
        }
    };

    bool anyDisplayChanged = false;

    // 0: Jump（ジャンプ）
    ImGui::PushID(0);
    ImGui::Checkbox(IMGUI_JP("ジャンプ"), &s_Enable[0]); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::SliderFloat(IMGUI_JP("重み"), &displayWeight[0], 0.0f, 100.0f)) { normalizeDisplayWeights(0); anyDisplayChanged = true; }
    {
        ImGui::SameLine(); ImGui::Text("%0.0f%%", displayWeight[0]);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::SliderFloat(IMGUI_JP("CD"), &s_CooldownDefault[0], 0.0f, 10.0f);
    ImGui::PopID();

    // 1: Shout（叫び）
    ImGui::PushID(1);
    ImGui::Checkbox(IMGUI_JP("叫び"), &s_Enable[1]); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::SliderFloat(IMGUI_JP("重み"), &displayWeight[1], 0.0f, 100.0f)) { normalizeDisplayWeights(1); anyDisplayChanged = true; }
    {
        ImGui::SameLine(); ImGui::Text("%0.0f%%", displayWeight[1]);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::SliderFloat(IMGUI_JP("CD"), &s_CooldownDefault[1], 0.0f, 10.0f);
    ImGui::PopID();

    // 2: Slash（通常）
    ImGui::PushID(2);
    ImGui::Checkbox(IMGUI_JP("通常"), &s_Enable[2]); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::SliderFloat(IMGUI_JP("重み"), &displayWeight[2], 0.0f, 100.0f)) { normalizeDisplayWeights(2); anyDisplayChanged = true; }
    {
        ImGui::SameLine(); ImGui::Text("%0.0f%%", displayWeight[2]);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::SliderFloat(IMGUI_JP("CD"), &s_CooldownDefault[2], 0.0f, 10.0f);
    ImGui::PopID();

    // 3: Spinning（回転）
    ImGui::PushID(3);
    ImGui::Checkbox(IMGUI_JP("回転"), &s_Enable[3]); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::SliderFloat(IMGUI_JP("重み"), &displayWeight[3], 0.0f, 100.0f)) { normalizeDisplayWeights(3); anyDisplayChanged = true; }
    {
        ImGui::SameLine(); ImGui::Text("%0.0f%%", displayWeight[3]);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::SliderFloat(IMGUI_JP("CD"), &s_CooldownDefault[3], 0.0f, 10.0f);
    ImGui::PopID();

    // 4: Stomp（とびかかり）
    ImGui::PushID(4);
    ImGui::Checkbox(IMGUI_JP("とびかかり"), &s_Enable[4]); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::SliderFloat(IMGUI_JP("重み"), &displayWeight[4], 0.0f, 100.0f)) { normalizeDisplayWeights(4); anyDisplayChanged = true; }
    {
        ImGui::SameLine(); ImGui::Text("%0.0f%%", displayWeight[4]);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::SliderFloat(IMGUI_JP("CD"), &s_CooldownDefault[4], 0.0f, 10.0f);
    ImGui::PopID();

    // 5: Throwing（岩投げ）
    ImGui::PushID(5);
    ImGui::Checkbox(IMGUI_JP("岩投げ"), &s_Enable[5]); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::SliderFloat(IMGUI_JP("重み"), &displayWeight[5], 0.0f, 100.0f)) { normalizeDisplayWeights(5); anyDisplayChanged = true; }
    {
        ImGui::SameLine(); ImGui::Text("%0.0f%%", displayWeight[5]);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::SliderFloat(IMGUI_JP("CD"), &s_CooldownDefault[5], 0.0f, 10.0f);
    ImGui::PopID();

    // 6: レーザー
    ImGui::PushID(6);
    ImGui::Checkbox(IMGUI_JP("レーザー"), &s_Enable[6]); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::SliderFloat(IMGUI_JP("重み"), &displayWeight[6], 0.0f, 100.0f)) { normalizeDisplayWeights(6); anyDisplayChanged = true; }
    {
        ImGui::SameLine(); ImGui::Text("%0.0f%%", displayWeight[6]);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::SliderFloat(IMGUI_JP("CD"), &s_CooldownDefault[6], 0.0f, 10.0f);
    ImGui::PopID();

    // 7: MoveContinue (そのまま移動3秒)
    ImGui::PushID(7);
    ImGui::Checkbox(IMGUI_JP("そのまま移動(3s)"), &s_Enable[7]); ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    if (ImGui::SliderFloat(IMGUI_JP("重み"), &displayWeight[7], 0.0f, 100.0f)) { normalizeDisplayWeights(7); anyDisplayChanged = true; }
    {
        ImGui::SameLine(); ImGui::Text("%0.0f%%", displayWeight[7]);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::SliderFloat(IMGUI_JP("CD"), &s_CooldownDefault[7], 0.0f, 10.0f);
    ImGui::PopID();

    // Apply display changes back to internal weights (keep internal representation consistent with displayed percentages)
    if (anyDisplayChanged)
    {
        for (int k = 0; k < Count; ++k)
        {
            s_Weight[curDist][k] = displayWeight[k];
        }
    }

    ImGui::Separator();
    ImGui::Text(IMGUI_JP("=== 強制攻撃 ==="));
    const char* attackNames[] = { IMGUI_JP("ランダム"), IMGUI_JP("ジャンプ"), IMGUI_JP("叫び"), IMGUI_JP("通常"), IMGUI_JP("回転"), IMGUI_JP("とびかかり"), IMGUI_JP("岩投げ"), IMGUI_JP("レーザー") };
    // Use a display variable so the stored index isn't decremented every frame.
    int displayForce = s_ForceAttackIndex + 1; // map internal -1..6 -> display 0..7
    ImGui::Combo(IMGUI_JP("強制攻撃"), &displayForce, attackNames, IM_ARRAYSIZE(attackNames));
    s_ForceAttackIndex = displayForce - 1; // store back (-1 = Random, 0-6 = Each attack)

    ImGui::Separator();
    XMFLOAT3 debugTargetPos = m_pOwner->GetTargetPos();
    XMFLOAT3 debugBossPos = m_pOwner->GetPosition();
    float debugDist = XMVectorGetX(XMVector3Length(XMVectorSubtract(XMLoadFloat3(&debugTargetPos), XMLoadFloat3(&debugBossPos))));
    ImGui::Text(IMGUI_JP("現在の距離: %.2f"), debugDist);
    if (debugDist < s_NearRange) ImGui::TextColored(ImVec4(1,0,0,1), IMGUI_JP("-> 近距離"));
    else ImGui::TextColored(ImVec4(1,1,0,1), IMGUI_JP("-> 中距離"));
    
    // 読み込み / 保存 ボタン
    if (ImGui::Button(IMGUI_JP("BossMoveState 読み込み")))
    {
        LoadSettings();
    }
    ImGui::SameLine();
    if (ImGui::Button(IMGUI_JP("BossMoveState 保存")))
    {
        SaveSettings();
    }

    // Enter キーで読み込み (キー押下の立ち上がりで一度だけ実行)
    {
        static bool prevEnterDown = false;
        SHORT st = GetAsyncKeyState(VK_RETURN);
        bool enterDown = (st & 0x8000) != 0;
        if (enterDown && !prevEnterDown)
        {
            LoadSettings();
        }
        prevEnterDown = enterDown;
    }

    ImGui::End();
#endif

	float delta = m_pOwner-> GetDelta();

	// Attack timer
	m_Timer += delta;

    // Decrease cooldowns
    for (size_t i = 0; i < m_CooldownRemaining.size(); ++i)
    {
        m_CooldownRemaining[i] -= delta;
        if (m_CooldownRemaining[i] < 0.0f) m_CooldownRemaining[i] = 0.0f;
    }

	// 1. Get position info
	XMVECTOR vBossPos = XMLoadFloat3(&m_pOwner->GetPosition());
	XMFLOAT3 playerPosF = m_pOwner->GetTargetPos();
	XMVECTOR vTarget = XMLoadFloat3(&playerPosF);

	// Direction vector and distance to player
	XMVECTOR vToPlayer = XMVectorSubtract(vTarget, vBossPos);
	vToPlayer = XMVectorSetY(vToPlayer, 0.0f);
	float distanceToPlayer = XMVectorGetX(XMVector3Length(vToPlayer));

	constexpr float STRAFE_RANGE = 20.0f;

	// --------------------------------------------------------
	// 2. Phase-based movement and animation
	// --------------------------------------------------------
	switch (m_Phase)
	{
	case MovePhase::Start:
		if (m_pOwner->IsAnimEnd(Boss::enBossAnim::IdolToRun))
		{
			m_pOwner->ChangeAnim(Boss::enBossAnim::Run);
			m_Phase = MovePhase::Run;
		}
		break;

	case MovePhase::Run:
	{
		constexpr float APPROACH_SPEED = 10.0f;
		XMVECTOR vMoveDir = XMVector3Normalize(vToPlayer);
		XMVECTOR vNewPos = XMVectorAdd(vBossPos, XMVectorScale(vMoveDir, APPROACH_SPEED * delta));

		XMFLOAT3 newPosF;
		XMStoreFloat3(&newPosF, vNewPos);
		m_pOwner->SetPosition(newPosF);
		vBossPos = vNewPos;

		if (distanceToPlayer <= STRAFE_RANGE)
		{
			m_Phase = MovePhase::Stop;
			m_pOwner->ChangeAnim(Boss::enBossAnim::RunToIdol);
		}
	}
	break;

	case MovePhase::Stop:
		if (m_pOwner->IsAnimEnd(Boss::enBossAnim::RunToIdol))
		{
			m_Phase = MovePhase::Strafe;
			XMVECTOR vDirFromPlayer = XMVectorSubtract(vBossPos, vTarget);
			m_BaseAngle = atan2f(XMVectorGetX(vDirFromPlayer), XMVectorGetZ(vDirFromPlayer));
			m_RotationAngle = 0.0f;
		}
		break;

	case MovePhase::Strafe:
	{
		m_RotationAngle += static_cast<float>(m_RotationSpeed) * delta * m_rotationDirection;
		const float MAX_SWAY = XM_PIDIV4;
		if (fabsf(m_RotationAngle) > MAX_SWAY)
		{
			m_rotationDirection *= -1.0f;
			m_RotationAngle = std::clamp(m_RotationAngle, -MAX_SWAY, MAX_SWAY);
		}

		float finalAngle = m_BaseAngle + m_RotationAngle;
		XMVECTOR vOffset = XMVectorSet(
			sinf(finalAngle) * STRAFE_RANGE,
			0.0f,
			cosf(finalAngle) * STRAFE_RANGE,
			0.0f
		);
		XMVECTOR vIdealPos = XMVectorAdd(vTarget, vOffset);

		constexpr float TRACKING_DELAY = 0.7f;

		XMVECTOR vCurrentPos = XMLoadFloat3(&m_pOwner->GetPosition());
		float lerpFactor = TRACKING_DELAY * delta;
		if (lerpFactor > 1.0f) lerpFactor = 1.0f;

		XMVECTOR vNextPos = XMVectorLerp(vCurrentPos, vIdealPos, lerpFactor);

		XMFLOAT3 finalPosF;
		XMStoreFloat3(&finalPosF, vNextPos);
		finalPosF.y = m_pOwner->GetPosition().y;
		m_pOwner->SetPosition(finalPosF);

		m_pOwner->SetAnimSpeed(3.0);
		if (m_rotationDirection > 0)
			m_pOwner->ChangeAnim(Boss::enBossAnim::LeftMove);
		else
			m_pOwner->ChangeAnim(Boss::enBossAnim::RightMove);
	}
	break;
	}

	// --------------------------------------------------------
	// 3. Always face player
	// --------------------------------------------------------
	XMVECTOR vFinalBossPos = XMLoadFloat3(&m_pOwner->GetPosition());
	XMVECTOR vLookAt = XMVectorSubtract(vTarget, vFinalBossPos);
	float dx = XMVectorGetX(vLookAt);
	float dz = XMVectorGetZ(vLookAt);
	float angle = atan2f(dx, dz) + DirectX::XM_PI;
	m_pOwner->SetRotationY(angle);

    if (m_Timer >= s_AttackDelay)
	{
		float dist = XMVectorGetX(XMVector3Length(vLookAt));
        struct Candidate { std::function<std::unique_ptr<StateBase<Boss>>() > factory; float weight; int id; };
        std::vector<Candidate> weighted;

        auto pushCandidate = [&](int id, std::function<std::unique_ptr<StateBase<Boss>>() > factory, int distIndex){
            if (!s_Enable[id]) return;
            // If on cooldown, skip
            if (m_CooldownRemaining[id] > 0.0f) return;
            float w = s_Weight[distIndex][id];
            // apply repeat penalty
            if (m_LastAttackId == id) w *= s_RepeatPenalty;
            if (w <= 0.0f) return;
            weighted.push_back({ factory, w, id });
        };

        int distIndex = (dist < s_NearRange) ? Near : Mid;
        if (s_ForceAttackIndex >= 0 && s_ForceAttackIndex < Count)
        {
            int id = s_ForceAttackIndex;
            switch (id)
            {
            case AttackId::Jump: pushCandidate(AttackId::Jump, [this]() { return std::make_unique<BossJumpOnlState>(m_pOwner); }, distIndex); break;
            case AttackId::Shout: pushCandidate(AttackId::Shout, [this]() { return std::make_unique<BossShoutState>(m_pOwner); }, distIndex); break;
            case AttackId::Slash: pushCandidate(AttackId::Slash, [this]() { return std::make_unique<BossSlashState>(m_pOwner); }, distIndex); break;
            case AttackId::Spinning: pushCandidate(AttackId::Spinning, [this]() { return std::make_unique<BossSpinningState>(m_pOwner); }, distIndex); break;
            case AttackId::Stomp: pushCandidate(AttackId::Stomp, [this]() { return std::make_unique<BossStompState>(m_pOwner); }, distIndex); break;
            case AttackId::Throwing: pushCandidate(AttackId::Throwing, [this]() { return std::make_unique<BossThrowingState>(m_pOwner); }, distIndex); break;
            case AttackId::Laser: pushCandidate(AttackId::Laser, [this]() { return std::make_unique<BossLaserState>(m_pOwner); }, distIndex); break;
            case AttackId::MoveContinue: pushCandidate(AttackId::MoveContinue, [this]() { return std::make_unique<BossMoveContinueState>(m_pOwner); }, distIndex); break;
            default: break;
            }
        }
        else
        {
            pushCandidate(AttackId::Jump, [this]() { return std::make_unique<BossJumpOnlState>(m_pOwner); }, distIndex);
            pushCandidate(AttackId::Shout, [this]() { return std::make_unique<BossShoutState>(m_pOwner); }, distIndex);
            pushCandidate(AttackId::Slash, [this]() { return std::make_unique<BossSlashState>(m_pOwner); }, distIndex);
            pushCandidate(AttackId::Spinning, [this]() { return std::make_unique<BossSpinningState>(m_pOwner); }, distIndex);
            pushCandidate(AttackId::Stomp, [this]() { return std::make_unique<BossStompState>(m_pOwner); }, distIndex);
            pushCandidate(AttackId::Throwing, [this]() { return std::make_unique<BossThrowingState>(m_pOwner); }, distIndex);
            pushCandidate(AttackId::Laser, [this]() { return std::make_unique<BossLaserState>(m_pOwner); }, distIndex);
            pushCandidate(AttackId::MoveContinue, [this]() { return std::make_unique<BossMoveContinueState>(m_pOwner); }, distIndex);
        }

        if (!weighted.empty())
        {
            float total = 0.0f;
            for (auto &c : weighted) total += c.weight;

            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dis(0.0f, total);
            float r = dis(gen);
            float acc = 0.0f;
            int chosenId = -1;
            std::function<std::unique_ptr<StateBase<Boss>>() > chosenFactory = nullptr;
            for (auto &c : weighted)
            {
                acc += c.weight;
                if (r <= acc)
                {
                    chosenId = c.id;
                    chosenFactory = c.factory;
                    break;
                }
            }

            if (chosenFactory)
            {
                if (chosenId >= 0 && chosenId < Count)
                {
                    m_CooldownRemaining[chosenId] = s_CooldownDefault[chosenId];
                }

                m_LastAttackId = chosenId;
                m_Timer = 0.0f; 
                m_pOwner->GetStateMachine()->ChangeState(std::move(chosenFactory()));
                return;
            }
        }
	}
}

void BossMoveState::LateUpdate()
{
}

void BossMoveState::Draw() 
{
}

void BossMoveState::Exit() 
{
}

void BossMoveState::SetInitialAngle(float angle)
{
	m_RotationAngle = angle;
}

void BossMoveState::LoadSettings()
{
    auto dataDir = std::filesystem::path("Data") / "Json" / "Boss";
    auto filePath = dataDir / std::filesystem::path("BossMoveState.json");
    if (!std::filesystem::exists(filePath)) return;
    json j = FileManager::JsonLoad(filePath);
    if (j.contains("RepeatPenalty")) s_RepeatPenalty = j["RepeatPenalty"].get<float>();
    if (j.contains("AttackDelay")) s_AttackDelay = j["AttackDelay"].get<float>();
    if (j.contains("NearRange")) s_NearRange = j["NearRange"].get<float>();
    if (j.contains("MidRange")) s_MidRange = j["MidRange"].get<float>();
    for (int i = 0; i < Count; ++i)
    {
        // JSON では英語 ID をキーに使う (Enable / Cooldown are per-attack)
        std::string keyEnable = std::string(s_AttackIds[i]) + "_Enable";
        std::string keyCD = std::string(s_AttackIds[i]) + "_Cooldown";
        if (j.contains(keyEnable)) s_Enable[i] = j[keyEnable].get<bool>();
        if (j.contains(keyCD)) s_CooldownDefault[i] = j[keyCD].get<float>();
        // per-distance weights
        for (int d = 0; d < DistCount; ++d)
        {
            std::string keyWeight = std::string(s_AttackIds[i]) + "_" + std::string(s_DistanceIds[d]) + "_Weight";
            if (j.contains(keyWeight)) s_Weight[d][i] = j[keyWeight].get<float>();
        }
    }
}

void BossMoveState::SaveSettings() const
{
    json j;
    j["RepeatPenalty"] = s_RepeatPenalty;
    j["AttackDelay"] = s_AttackDelay;
    j["NearRange"] = s_NearRange;
    j["MidRange"] = s_MidRange;
    // 保存: Enable (per-attack), Cooldown (per-attack), Weight (per-distance per-attack)
    for (int i = 0; i < Count; ++i)
    {
        std::string keyEnable = std::string(s_AttackIds[i]) + "_Enable";
        std::string keyCD = std::string(s_AttackIds[i]) + "_Cooldown";
        j[keyEnable] = s_Enable[i];
        j[keyCD] = s_CooldownDefault[i];
        // per-distance normalize and write
        for (int d = 0; d < DistCount; ++d)
        {
            float sum = 0.0f;
            for (int k = 0; k < Count; ++k) sum += s_Weight[d][k];
            float outWeight = 0.0f;
            if (sum > 1e-6f) outWeight = s_Weight[d][i] / sum * 100.0f;
            else outWeight = 100.0f / static_cast<float>(Count);
            std::string keyWeight = std::string(s_AttackIds[i]) + "_" + std::string(s_DistanceIds[d]) + "_Weight";
            j[keyWeight] = outWeight;
        }
    }
    auto dataDir = std::filesystem::path("Data") / "Json" / "Boss";
    std::error_code ec;
    std::filesystem::create_directories(dataDir, ec);
    auto filePath = dataDir / std::filesystem::path("BossMoveState.json");
    FileManager::JsonSave(filePath, j);
}
