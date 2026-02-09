#include "BossLaserState.h"

#include "Game/04_Time/Time.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/Boss.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"
#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Utility/FileManager/FileManager.h"
#include <cmath>

BossLaserState::BossLaserState(Boss* owner)
    : BossAttackStateBase(owner)
    , m_State(enLaser::None)
    , m_ChargeDuration(1.0f)
    , m_ChargeElapsed(0.0f)
    , m_FireDuration(0.6f)
    , m_FireElapsed(0.0f)
    , m_LaserDamage(10.0f)
    , m_LaserRadius(5.0f)
    , m_LaserRange(200.0f)
    , m_EffectPlayed(false)
{
    try { LoadSettings(); } catch (...) {}
}

BossLaserState::~BossLaserState() {}

void BossLaserState::Enter()
{
    try { LoadSettings(); } catch (...) {}
    BossAttackStateBase::Enter();

    m_State = enLaser::Charge;
    m_ChargeElapsed = 0.0f;
    m_FireElapsed = 0.0f;
    m_EffectPlayed = false;

    FacePlayerInstantYaw();
    m_pOwner->SetAnimSpeed(1.0);
    m_pOwner->ChangeAnim(Boss::enBossAnim::LaserCharge);

    
}

void BossLaserState::Update()
{
    BossAttackStateBase::Update();
    float dt = m_pOwner->GetDelta();
    UpdateBaseLogic(dt);

    if (m_State == enLaser::Fire
        || m_State == enLaser::Cool)
    {
        if (auto* col = m_pOwner->GetLaserCollider()) {
            col->SetActive(true);
            float t = (m_FireElapsed / (m_FireDuration > 0.0f ? m_FireDuration : 1.0f));
            if (t > 1.0f) t = 1.0f;
            float radius = m_LaserRadius * t;
            col->SetRadius(radius);
            col->SetPositionOffset(0.0f, 0.0f,( radius * -30.f ) + (35.f));
        }
    }

    switch (m_State) {
    case enLaser::Charge:
        m_ChargeElapsed += dt;
        // keep facing player while charging
        FacePlayerYawContinuous();
        if (m_ChargeElapsed >= m_ChargeDuration || m_pOwner->IsAnimEnd(Boss::enBossAnim::LaserCharge)) {
            m_State = enLaser::Fire;
            m_pOwner->SetAnimSpeed(1.0);
            m_pOwner->ChangeAnim(Boss::enBossAnim::Laser);
        }
        break;
    case enLaser::Fire:
        m_FireElapsed += dt;
        if (!m_EffectPlayed) {
            if (m_pOwner) {
                DirectX::XMFLOAT3 bossPos = m_pOwner->GetPosition();
                DirectX::XMFLOAT3 localOffset{0.0f, 12.0f, -5.0f};
                float yaw = m_pOwner->GetTransform()->Rotation.y;
                float c = cosf(yaw);
                float s = sinf(yaw);
                DirectX::XMFLOAT3 worldOffset;
                worldOffset.x = c * localOffset.x + s * localOffset.z;
                worldOffset.y = localOffset.y;
                worldOffset.z = -s * localOffset.x + c * localOffset.z;
                DirectX::XMFLOAT3 effectPos{ bossPos.x + worldOffset.x, bossPos.y + worldOffset.y, bossPos.z + worldOffset.z };
                DirectX::XMFLOAT3 eulerRot{ 0.0f, yaw, 0.0f };
                m_pOwner->PlayEffectAtWorldPos("Boss_Laser", effectPos, eulerRot, 2.0f, false);
                SoundManager::GetInstance().Play("Beam");
                SoundManager::GetInstance().SetVolume("Beam", 9000);
            }
            m_EffectPlayed = true;
        }
        if (m_FireElapsed >= m_FireDuration || m_pOwner->IsAnimEnd(Boss::enBossAnim::Laser)) {
          
            m_pOwner->SetAnimSpeed(1.5);
            m_pOwner->ChangeAnim(Boss::enBossAnim::LaserEnd);
            m_State = enLaser::Cool;
        }
        break;
    case enLaser::Cool:
        if (auto* col = m_pOwner->GetLaserCollider()) {
        col->SetActive(false);
    }
        if (m_pOwner->IsAnimEnd(Boss::enBossAnim::LaserEnd)) {
            m_State = enLaser::Trans;
        }
        break;
    case enLaser::Trans:
        m_pOwner->GetStateMachine()->ChangeState(std::make_shared<BossIdolState>(m_pOwner));
        break;
    default:
        break;
    }
}

void BossLaserState::LateUpdate() {}
void BossLaserState::Draw() {}

void BossLaserState::Exit()
{
    try { SaveSettings(); } catch (...) {}
    if (auto* col = m_pOwner->GetShoutCollider()) {
        col->SetActive(false);
    }
}

std::pair<Boss::enBossAnim, float> BossLaserState::GetParryAnimPair()
{
    return std::pair(Boss::enBossAnim::Laser, 0.5f);
}

void BossLaserState::DrawImGui()
{
#if _DEBUG
    ImGui::Begin(IMGUI_JP("BossLaser State"));
    ImGui::Text(IMGUI_JP("Laser State: %d"), static_cast<int>(m_State));
    CImGuiManager::Slider<float>(IMGUI_JP("チャージ時間"), m_ChargeDuration, 0.0f, 5.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("発射時間"), m_FireDuration, 0.0f, 5.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("ダメージ"), m_LaserDamage, 0.0f, 100.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("範囲"), m_LaserRadius, 0.0f, 200.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("射程"), m_LaserRange, 0.0f, 1000.0f, true);

    // Debug controls for forcing state
    if (ImGui::Button(IMGUI_JP("Force Charge"))) { m_State = enLaser::Charge; m_ChargeElapsed = 0.0f; }
    ImGui::SameLine();
    if (ImGui::Button(IMGUI_JP("Force Fire"))) { m_State = enLaser::Fire; m_FireElapsed = 0.0f; if (auto* c = m_pOwner->GetShoutCollider()) { c->SetActive(true); } }
    ImGui::SameLine();
    if (ImGui::Button(IMGUI_JP("Force Cool"))) { m_State = enLaser::Cool; }
    ImGui::SameLine();
    if (ImGui::Button(IMGUI_JP("Force Trans"))) { m_State = enLaser::Trans; }
    BossAttackStateBase::DrawImGui();
    // collider debug
    if (auto* col = m_pOwner->GetShoutCollider()) {
        ImGui::Separator();
        ImGui::Text(IMGUI_JP("Shout Collider: Active=%s Radius=%.2f"), col->GetActive() ? "ON" : "OFF", col->GetRadius());
        DirectX::XMFLOAT3 off = col->GetPositionOffset();
        ImGui::Text(IMGUI_JP("Offset: (%.2f, %.2f, %.2f)"), off.x, off.y, off.z);
    }
    if (ImGui::Button(IMGUI_JP("Load"))) { try { LoadSettings(); } catch (...) {} }
    ImGui::SameLine();
    if (ImGui::Button(IMGUI_JP("Save"))) { try { SaveSettings(); } catch (...) {} }
    ImGui::End();
#endif
}

void BossLaserState::LoadSettings()
{
    auto filePath = GetSettingsFileName();
    if (!filePath.is_absolute()) {
        filePath = std::filesystem::current_path() / "Data" / "Json" / "Boss" / filePath;
    }
    if (!std::filesystem::exists(filePath)) return;

    json j = FileManager::JsonLoad(filePath);
    if (j.contains("ChargeDuration")) m_ChargeDuration = j["ChargeDuration"].get<float>();
    if (j.contains("FireDuration")) m_FireDuration = j["FireDuration"].get<float>();
    if (j.contains("LaserDamage")) m_LaserDamage = j["LaserDamage"].get<float>();
    if (j.contains("LaserRadius")) m_LaserRadius = j["LaserRadius"].get<float>();
    if (j.contains("LaserRange")) m_LaserRange = j["LaserRange"].get<float>();
}

void BossLaserState::SaveSettings() const
{
    auto filePath = GetSettingsFileName();
    if (!filePath.is_absolute()) {
        auto dir = std::filesystem::current_path() / "Data" / "Json" / "Boss";
        std::filesystem::create_directories(dir);
        filePath = dir / filePath;
    }
    json j = SerializeSettings();
    j["ChargeDuration"] = m_ChargeDuration;
    j["FireDuration"] = m_FireDuration;
    j["LaserDamage"] = m_LaserDamage;
    j["LaserRadius"] = m_LaserRadius;
    j["LaserRange"] = m_LaserRange;
    FileManager::JsonSave(filePath, j);
}
