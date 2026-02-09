#include "BossSpinningState.h"
#include "00_MeshObject/00_Character/02_Boss/Boss.h"
#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"
#include "..//04_Time/Time.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Utility/FileManager/FileManager.h"
#include <Windows.h>
#include "System/Utility/Math/Easing/Easing.h"

BossSpinningState::BossSpinningState(Boss* owner)
    : BossAttackStateBase(owner)
    , m_List(enSpinning::None)
    , m_IsSpun(false)
    , m_SecondSlashTimer(0.f)
    , m_SecondSlashTiming(3.f)
    , m_SecondSlashed(false)
{
}

BossSpinningState::~BossSpinningState()
{
}

void BossSpinningState::Enter()
{
    BossAttackStateBase::Enter();
    m_CurrentTime = 0.0f;
    m_pOwner->SetIsLoop(false);
    m_pOwner->SetAnimTime(0.0);
    m_pOwner->SetAnimSpeed(1.0);
    // 初期は "Charge" を再生（下から順に Charge -> ChargeAttack -> ChargeToIdol）
    m_pOwner->ChangeAnim(Boss::enBossAnim::Charge);
    m_List = enSpinning::Anim;
    m_IsSpun = false;
    SoundManager::GetInstance().Play("AttackCharge", false);
    SoundManager::GetInstance().SetVolume("AttackCharge", 9000);
}

void BossSpinningState::Update()
{
    BossAttackStateBase::Update();
    const float dt = m_pOwner->GetDelta();
    UpdateBaseLogic(dt);
    m_SecondSlashTimer += dt;

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
            DirectX::XMFLOAT3 end{ 0.0f, m_RotateTotalDeg, 0.0f };
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
        if (m_pOwner->IsAnimEnd(Boss::enBossAnim::ChargeToIdol) )
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

    // 基底 Update は先頭で呼んでいるためここでは不要
}

void BossSpinningState::LateUpdate()
{
    BossAttackStateBase::LateUpdate();
}

void BossSpinningState::Draw()
{
    BossAttackStateBase::Draw();
}

void BossSpinningState::Exit()
{
    BossAttackStateBase::Exit();
}

std::pair<Boss::enBossAnim, float> BossSpinningState::GetParryAnimPair()
{
    return std::pair(Boss::enBossAnim::ChargeAttack, 0.215f);
}

void BossSpinningState::DrawImGui()
{
#if _DEBUG
    ImGui::Begin(IMGUI_JP("BossSpinning State"));
    CImGuiManager::Slider<float>(IMGUI_JP("合計回転角度 (deg)"), m_RotateTotalDeg, 0.0f, 3600.0f, true);

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

void BossSpinningState::LoadSettings()
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
    if (j.contains("RotateTotalDeg")) m_RotateTotalDeg = j["RotateTotalDeg"].get<float>();
    if (j.contains("EasingType")) m_EasingType = static_cast<MyEasing::Type>(j["EasingType"].get<int>());
}

void BossSpinningState::SaveSettings() const
{
    // Serialize base + derived fields and save once.
    json j = SerializeSettings();
    j["RotateTotalDeg"] = m_RotateTotalDeg;
    j["EasingType"] = static_cast<int>(m_EasingType);

    auto filePath = GetSettingsFileName();
    if (!filePath.is_absolute()) {
        auto dir = std::filesystem::current_path() / "Data" / "Json" / "Boss";
        std::filesystem::create_directories(dir);
        filePath = dir / filePath;
    }
    FileManager::JsonSave(filePath, j);
}
