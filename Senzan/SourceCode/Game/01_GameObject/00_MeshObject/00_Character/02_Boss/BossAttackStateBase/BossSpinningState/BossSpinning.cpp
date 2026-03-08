#include "BossSpinning.h"
#include "00_MeshObject/00_Character/02_Boss/Boss.h"
#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"
#include"Game/04_Time/Time.h"
#include "System/Utility/FileManager/FileManager.h"
#include "System/Singleton/ImGui/CImGuiManager.h"

BossSpinning::BossSpinning(Boss* owner)
	: BossAttackStateBase(owner)

	, m_List (enSpinning::None)
	, m_IsSpun(false)

	, m_RotateToTalDeg(360.0f)
	, m_EasingType(MyEasing::Type::InOutSine)
	, m_LastEasedAngle(0.0f)

	, m_SecondSlashTimer(0.f)
	, m_SecondSlashTiming(3.f)
	, m_SecondSlashed(false)
{
}

BossSpinning::~BossSpinning()
{
}

void BossSpinning::Enter()
{
	BossAttackStateBase::Enter();
	m_CurrentTime = 0.0f;
	//アニメーションの再生を一回に設定する.
	m_pOwner->SetIsLoop(false);
	m_pOwner->SetAnimTime(0.0);
	m_pOwner->SetAnimSpeed(1.0);
	//回転の準備をする.
	m_pOwner->ChangeAnim(Boss::enBossAnim::Charge);
	m_List = enSpinning::Anim;
	m_IsSpun = false;
	//音の再生.
	SoundManager::GetInstance().Play("AttackCharge", false);
	SoundManager::GetInstance().SetVolume("AttackCharge", 9000);
}

void BossSpinning::Update()
{
	BossAttackStateBase::Update();
	const float deltaTime = m_pOwner->GetDelta();
	UpdateBaseLogic(deltaTime);
	m_SecondSlashTimer += deltaTime;

    FacePlayerInstantYaw();
    switch (m_List)
    {
    case enSpinning::None:
        m_List = enSpinning::Anim;
        break;

    case enSpinning::Anim:
        // start attack when charge animation ends or after charge time
        if (m_CurrentTime >= m_ChargeTime)
        {
            m_List = enSpinning::Attack;
            // 攻撃アニメへ
            m_pOwner->ChangeAnim(Boss::enBossAnim::ChargeAttack);
        }
        break;

    case enSpinning::Attack:
    {
        if (!m_IsSpun)
        {
            m_IsSpun = true;
            SoundManager::GetInstance().Play("BossSwing", false);
            SoundManager::GetInstance().SetVolume("BossSwing", 9000);
            SoundManager::GetInstance().Play("Throw", false);
            SoundManager::GetInstance().SetVolume("Throw", 8000);
        }
        if (m_SecondSlashTiming < m_SecondSlashTimer && !m_SecondSlashed)
        {
            SoundManager::GetInstance().Play("BossSwing", false);
            SoundManager::GetInstance().SetVolume("BossSwing", 9000);
            SoundManager::GetInstance().Play("Throw", false);
            SoundManager::GetInstance().SetVolume("Throw", 8000);
            m_SecondSlashed = true;
        }
        // 攻撃フェーズの進行率 t in [0,1]
        float phaseStart = m_ChargeTime;
        float phaseEnd = m_ChargeTime + m_AttackTime;
        float t = 0.0f;
        if (phaseEnd > phaseStart) t = (m_CurrentTime - phaseStart) / (phaseEnd - phaseStart);
        if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;

        // イージングで目標角度を求める
        float easedAngle = 0.0f;
        {
            DirectX::XMFLOAT3 start{ 0.0f, 0.0f, 0.0f };
            DirectX::XMFLOAT3 end{ 0.0f, m_RotateToTalDeg, 0.0f };
            DirectX::XMFLOAT3 out;
            MyEasing::UpdateEasing(m_EasingType, t, 1.0f, start, end, out);
            easedAngle = out.y; // Y 成分
        }

        // 差分回転を適用
        float deltaDeg = easedAngle - m_LastEasedAngle;
        m_LastEasedAngle = easedAngle;
        if (m_pOwner && m_pOwner->GetTransform() && fabsf(deltaDeg) > 1e-6f) {
            DirectX::XMFLOAT3 rotDeg{ 0.0f, deltaDeg, 0.0f };
            m_pOwner->GetTransform()->RotateDegrees(rotDeg);
        }

        if (m_CurrentTime >= m_ChargeTime + m_AttackTime)
        {
            m_List = enSpinning::CoolDown;
            // 攻撃終了から待機遷移用アニメ
            m_pOwner->ChangeAnim(Boss::enBossAnim::ChargeToIdol);
        }
        break;
    }
    case enSpinning::CoolDown:
        if (m_pOwner->IsAnimEnd(Boss::enBossAnim::ChargeToIdol))
        {
            m_List = enSpinning::Trans;
        }
        break;

    case enSpinning::Trans:
        if (!m_IsDebugStop)
        {
            m_LastEasedAngle = 0.0f; // reset
            m_pOwner->GetStateMachine()->ChangeState(std::make_shared<BossIdolState>(m_pOwner));
        }
        break;
    }


}

void BossSpinning::LateUpdate()
{
}

void BossSpinning::Draw()
{
}

void BossSpinning::Exit()
{
}

std::pair<Boss::enBossAnim, float> BossSpinning::GetParryAnimPair()
{
    return std::pair(Boss::enBossAnim::ChargeAttack, 0.215f);
}

void BossSpinning::DrawImGui()
{
#if _DEBUG
    ImGui::Begin(IMGUI_JP("BossSpinning State"));
    CImGuiManager::Slider<float>(IMGUI_JP("合計回転角度 (deg)"), m_RotateToTalDeg, 0.0f, 3600.0f, true);

    // Easing selector
    static const char* easingNames[] = {
        "Liner","InSine","OutSine","InOutSine","InQuad","OutQuad","InOutQuad",
        "InCubic","OutCubic","InOutCubic","InQuart","OutQuart","InOutQuart",
        "InQuint","OutQuint","InOutQuint","InExpo","OutExpo","InOutExpo",
        "InCirc","OutCirc","InOutCirc","InBack","OutBack","InOutBack",
        "InElastic","OutElastic","InOutElastic","InBounce","OutBounce","InOutBounce"
    };
    int easingIndex = static_cast<int>(m_EasingType);
    if (ImGui::Combo(IMGUI_JP("イージング"), &easingIndex, easingNames, IM_ARRAYSIZE(easingNames))) {
        m_EasingType = static_cast<MyEasing::Type>(easingIndex);
    }

    ImGui::Separator();
    if (ImGui::Button(IMGUI_JP("読み込み (Enter またはボタン)"))) { LoadSettings(); }

    // Enter キーで JSON を読み込む (既存の処理)
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

    BossAttackStateBase::DrawImGui();
    ImGui::End();
#endif
}

void BossSpinning::LoadSettings()
{
    // Load base settings (this reads Data/Json/Boss/<file>)
    BossAttackStateBase::LoadSettings();

    // Load additional fields from the same location as other boss states
    auto filePath = GetSettingsFileName();
    if (!filePath.is_absolute()) {
        auto dir = std::filesystem::current_path() / "Data" / "Json" / "Boss";
        filePath = dir / filePath;
    }
    if (!std::filesystem::exists(filePath)) return;
    json j = FileManager::JsonLoad(filePath);
    if (j.contains("RotateTotalDeg")) m_RotateToTalDeg = j["RotateTotalDeg"].get<float>();
    if (j.contains("EasingType")) m_EasingType = static_cast<MyEasing::Type>(j["EasingType"].get<int>());

}

void BossSpinning::SaveSettings() const
{
    // Serialize base + derived fields and save once.
    json j = SerializeSettings();
    j["RotateTotalDeg"] = m_RotateToTalDeg;
    j["EasingType"] = static_cast<int>(m_EasingType);

    auto filePath = GetSettingsFileName();
    if (!filePath.is_absolute()) {
        auto dir = std::filesystem::current_path() / "Data" / "Json" / "Boss";
        std::filesystem::create_directories(dir);
        filePath = dir / filePath;
    }
    FileManager::JsonSave(filePath, j);

}
