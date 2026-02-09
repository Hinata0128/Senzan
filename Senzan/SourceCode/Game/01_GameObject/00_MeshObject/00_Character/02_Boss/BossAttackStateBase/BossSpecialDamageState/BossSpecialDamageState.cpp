#include "BossSpecialDamageState.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/Boss.h"
#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Utility/FileManager/FileManager.h"

BossSpecialDamageState::BossSpecialDamageState(Boss* owner)
    : BossAttackStateBase(owner)
{
    try { LoadSettings(); } catch (...) {}
}

BossSpecialDamageState::~BossSpecialDamageState()
{
}

void BossSpecialDamageState::Enter()
{
    try { LoadSettings(); } catch (...) {}
    BossAttackStateBase::Enter();

    m_Phase = Phase::TakeDamage;
    m_PhaseTimer = 0.0f;
    m_ReplayTimer = 0.0f;

    m_pOwner->SetAnimSpeed(m_TakeDamageAnimSpeed);
    m_pOwner->ChangeAnim(Boss::enBossAnim::Hit);
}

void BossSpecialDamageState::Update()
{
    BossAttackStateBase::Update();
    float dt = m_pOwner->GetDelta();

    switch (m_Phase)
    {
    case Phase::TakeDamage:
    {
        m_PhaseTimer += dt;
        m_ReplayTimer += dt;

        // ダメージアニメーションをリプレイ（Playerが終わるまで繰り返す）
        if (m_TakeDamageReplayEnabled && m_ReplayTimer >= m_TakeDamageReplayTime)
        {
            m_pOwner->SetAnimSpeed(m_TakeDamageAnimSpeed);
            m_pOwner->SetAnimTime(0.0);
            m_pOwner->ChangeAnim(Boss::enBossAnim::Hit);
            m_ReplayTimer = 0.0f;
        }
        // Downへの遷移はForceTransitionToDown()で行うため、自動遷移は無効
    }
    break;

    case Phase::Down:
    {
        if (m_pOwner->IsAnimEnd(Boss::enBossAnim::FlinchParis))
        {
            m_Phase = Phase::DownContinue;
            m_PhaseTimer = 0.0f;
            m_pOwner->SetIsLoop(true);
            m_pOwner->SetAnimSpeed(m_DownContinueSpeed);
            m_pOwner->SetAnimTime(0.0);
            m_pOwner->ChangeAnim(Boss::enBossAnim::Flinch);
        }
    }
    break;

    case Phase::DownContinue:
    {
        m_PhaseTimer += dt;
        if (m_PhaseTimer >= m_DownContinueDuration)
        {
            m_Phase = Phase::GetUp;
            m_PhaseTimer = 0.0f;
            m_pOwner->SetAnimSpeed(m_GetUpAnimSpeed);
            m_pOwner->ChangeAnim(Boss::enBossAnim::FlinchToIdol);
        }
    }
    break;

    case Phase::GetUp:
    {
        if (m_pOwner->IsAnimEnd(Boss::enBossAnim::FlinchToIdol))
        {
            m_Phase = Phase::End;
        }
    }
    break;

    case Phase::End:
    {
        m_pOwner->GetStateMachine()->ChangeState(std::make_shared<BossIdolState>(m_pOwner));
    }
    break;
    }
}

void BossSpecialDamageState::LateUpdate()
{
}

void BossSpecialDamageState::Draw()
{
}

void BossSpecialDamageState::ForceTransitionToDown()
{
    // TakeDamageフェーズ中の場合のみDownに遷移
    if (m_Phase == Phase::TakeDamage)
    {
        m_Phase = Phase::Down;
        m_PhaseTimer = 0.0f;
        m_pOwner->SetAnimSpeed(m_DownAnimSpeed);
        m_pOwner->SetIsLoop(false);
        m_pOwner->SetAnimTime(0.0);
        m_pOwner->ChangeAnim(Boss::enBossAnim::FlinchParis);
    }
}

void BossSpecialDamageState::Exit()
{
    try { SaveSettings(); } catch (...) {}
}

void BossSpecialDamageState::DrawImGui()
{
#if _DEBUG
    ImGui::Begin(IMGUI_JP("ボス：特殊被ダメージ"));
    ImGui::Text(IMGUI_JP("フェーズ: %d"), static_cast<int>(m_Phase));
    ImGui::Text(IMGUI_JP("フェーズ経過時間: %.2f"), m_PhaseTimer);
    ImGui::Text(IMGUI_JP("再生タイマー: %.2f"), m_ReplayTimer);

    ImGui::Separator();
    ImGui::Text(IMGUI_JP("--- 被ダメ時設定 ---"));
    CImGuiManager::Slider<float>(IMGUI_JP("ダメージ持続時間(秒)"), m_TakeDamageDuration, 0.0f, 10.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("被ダメアニメ速度"), m_TakeDamageAnimSpeed, 0.1f, 5.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("再生成間隔(秒)"), m_TakeDamageReplayTime, 0.1f, 5.0f, true);
    ImGui::Checkbox(IMGUI_JP("再生成を有効にする"), &m_TakeDamageReplayEnabled);

    ImGui::Separator();
    ImGui::Text(IMGUI_JP("--- ダウン開始設定 ---"));
    CImGuiManager::Slider<float>(IMGUI_JP("ダウン入りアニメ速度"), m_DownAnimSpeed, 0.1f, 5.0f, true);

    ImGui::Text(IMGUI_JP("--- ダウン継続設定 ---"));
    CImGuiManager::Slider<float>(IMGUI_JP("ダウン継続アニメ速度"), m_DownContinueSpeed, 0.1f, 5.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("ダウン継続時間(秒)"), m_DownContinueDuration, 0.0f, 10.0f, true);

    ImGui::Separator();
    ImGui::Text(IMGUI_JP("--- 立ち上がり設定 ---"));
    CImGuiManager::Slider<float>(IMGUI_JP("立ち上がりアニメ速度"), m_GetUpAnimSpeed, 0.1f, 5.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("立ち上がり遷移時間(秒)"), m_GetUpTransitionTime, 0.0f, 2.0f, true);

    ImGui::Separator();
    if (ImGui::Button(IMGUI_JP("読み込み"))) { try { LoadSettings(); } catch (...) {} }
    ImGui::SameLine();
    if (ImGui::Button(IMGUI_JP("保存"))) { try { SaveSettings(); } catch (...) {} }
    BossAttackStateBase::DrawImGui();
    ImGui::End();
#endif
}

void BossSpecialDamageState::LoadSettings()
{
    auto filePath = GetSettingsFileName();
    if (!filePath.is_absolute()) {
        filePath = std::filesystem::current_path() / "Data" / "Json" / "Boss" / filePath;
    }
    if (!std::filesystem::exists(filePath)) return;

    json j = FileManager::JsonLoad(filePath);
    if (j.contains("TakeDamageDuration")) m_TakeDamageDuration = j["TakeDamageDuration"].get<float>();
    if (j.contains("TakeDamageAnimSpeed")) m_TakeDamageAnimSpeed = j["TakeDamageAnimSpeed"].get<float>();
    if (j.contains("TakeDamageReplayTime")) m_TakeDamageReplayTime = j["TakeDamageReplayTime"].get<float>();
    if (j.contains("TakeDamageReplayEnabled")) m_TakeDamageReplayEnabled = j["TakeDamageReplayEnabled"].get<bool>();
    if (j.contains("DownAnimSpeed")) m_DownAnimSpeed = j["DownAnimSpeed"].get<float>();
    if (j.contains("DownTransitionTime")) m_DownTransitionTime = j["DownTransitionTime"].get<float>();
    if (j.contains("DownContinueDuration")) m_DownContinueDuration = j["DownContinueDuration"].get<float>();
    if (j.contains("GetUpAnimSpeed")) m_GetUpAnimSpeed = j["GetUpAnimSpeed"].get<float>();
    if (j.contains("GetUpTransitionTime")) m_GetUpTransitionTime = j["GetUpTransitionTime"].get<float>();
}

void BossSpecialDamageState::SaveSettings() const
{
    auto filePath = GetSettingsFileName();
    if (!filePath.is_absolute()) {
        auto dir = std::filesystem::current_path() / "Data" / "Json" / "Boss";
        std::filesystem::create_directories(dir);
        filePath = dir / filePath;
    }
    json j = SerializeSettings();
    j["TakeDamageDuration"] = m_TakeDamageDuration;
    j["TakeDamageAnimSpeed"] = m_TakeDamageAnimSpeed;
    j["TakeDamageReplayTime"] = m_TakeDamageReplayTime;
    j["TakeDamageReplayEnabled"] = m_TakeDamageReplayEnabled;
    j["DownAnimSpeed"] = m_DownAnimSpeed;
    j["DownTransitionTime"] = m_DownTransitionTime;
    j["DownContinueDuration"] = m_DownContinueDuration;
    j["GetUpAnimSpeed"] = m_GetUpAnimSpeed;
    j["GetUpTransitionTime"] = m_GetUpTransitionTime;
    FileManager::JsonSave(filePath, j);
}

