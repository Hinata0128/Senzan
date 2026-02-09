#include "Combat.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"
#include "Game/05_InputDevice/VirtualPad.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "Utility/FileManager/FileManager.h"
#include "System/Utility/Math/Math.h"

namespace PlayerState {
Combat::Combat(Player* owner)
	: Action			( owner )
    , m_Distance            ( 0.0f )
    , m_AnimSpeed           ( 0.0f )
    , m_MinComboTransTime   ( 0.0f )
    , m_currentTime         ( 0.0f )
    , m_ComboStartTime      ( 0.0f )
    , m_ComboEndTime        ( 0.0f )
    , m_IsComboAccepted     ( false )
{
}

Combat::~Combat()
{
}

void Combat::Enter()
{
	Action::Enter();
	m_currentTime = 0.f;

    m_IsComboAccepted = false; 

	// clear collider windows by default
	m_ColliderWindows.clear();
	// Ensure attack collider is disabled on enter
	if (m_pOwner) m_pOwner->SetAttackColliderActive(false);

	// Allow derived states to load their settings on enter
	LoadSettings();
}

void Combat::Update()
{
	Action::Update();

	// 時間を先に更新してからウィンドウ判定
	if (m_pOwner) {
		m_currentTime += m_pOwner->GetDelta();
	}

	ProcessColliderWindows(m_currentTime);
}

void Combat::LateUpdate()
{
	Action::LateUpdate();

	// Keep facing the move direction while in combat if a move vector is set
	if (m_pOwner)
	{
		const DirectX::XMFLOAT3& mv = m_pOwner->m_MoveVec;
		if (!MyMath::IsVector3NearlyZero(mv))
		{
			m_pOwner->GetTransform()->RotateToDirection(mv);
		}
	}
}

void Combat::Draw()
{
	Action::Draw();
}

void Combat::Exit()
{
    // 当たり判定を無効化.
    for (auto& Collider : m_ColliderWindows) {
        Collider.IsAct = false;
        Collider.IsEnd = false;
    }
    if (m_pOwner) m_pOwner->SetAttackColliderActive(false);

	Action::Exit();
}

void Combat::LoadSettings()
{
    const std::string fileName = GetSettingsFileName();
    if (fileName == "") return;

    try {
    //C:\Users\green\source\C++\Senzan\Senzan\Data\Json\Player\AttackCombo
        std::filesystem::path filePath = std::filesystem::current_path() / fileName;
        if (std::filesystem::exists(filePath)) {
            json j = FileManager::JsonLoad(filePath);
            if (j.contains("m_AnimSpeed")) m_AnimSpeed = j["m_AnimSpeed"].get<float>();
            if (j.contains("m_MinComboTransTime")) m_MinComboTransTime = j["m_MinComboTransTime"].get<float>();
            if (j.contains("m_ComboStartTime")) m_ComboStartTime = j["m_ComboStartTime"].get<float>();
            if (j.contains("m_ComboEndTime")) m_ComboEndTime = j["m_ComboEndTime"].get<float>();

            if (j.contains("ColliderWindows") && j["ColliderWindows"].is_array()) {
                m_ColliderWindows.clear();
                for (const auto& entry : j["ColliderWindows"]) {
                    if (entry.contains("start") && entry.contains("duration")) {
                        AddColliderWindow(entry["start"].get<float>(), entry["duration"].get<float>());
                    }
                }
            }
        }
    }
    catch (...) {}
}

void Combat::AddColliderWindow(float start, float duration)
{
	// normalize incoming values
	if (start < 0.0f) start = 0.0f;
	if (duration < 0.0f) duration = 0.0f;
	// ensure flags are initialized
	ColliderWindow w;
	w.Start = start;
	w.Duration = duration;
	w.IsAct = false;
	w.IsEnd = false;
	m_ColliderWindows.push_back(w);
}

void Combat::ProcessColliderWindows(float currentTime)
{
    // 当たり判定のSetActive.
    for (auto& Collider : m_ColliderWindows) {
        if (Collider.IsEnd) continue;
        if (Collider.IsAct) {
            float end = Collider.Start + Collider.Duration;
            if (m_currentTime >= end) {
                m_pOwner->SetAttackColliderActive(false);
                Collider.IsEnd = true;
            }
        }
        else {
            if (m_currentTime >= Collider.Start) {
                m_pOwner->SetAttackColliderActive(true);
                Collider.IsAct = true;
            }
        }
    }
}

void Combat::RenderColliderWindowsUI(const char* title)
{
	ImGui::Separator();
	ImGui::Text("%s", title);

	for (size_t i = 0; i < m_ColliderWindows.size(); ++i)
	{
		auto &w = m_ColliderWindows[i];
		ImGui::PushID((int)i);
		ImGui::DragFloat(IMGUI_JP("開始時刻 (秒)"), &w.Start, 0.01f, 0.0f, m_ComboEndTime);
		ImGui::SameLine();
		ImGui::DragFloat(IMGUI_JP("継続時間 (秒)"), &w.Duration, 0.01f, 0.0f, m_ComboEndTime);
		ImGui::SameLine();
		if (ImGui::Button(IMGUI_JP("削除"))) { m_ColliderWindows.erase(m_ColliderWindows.begin() + i); ImGui::PopID(); break; }
		ImGui::PopID();
	}

	if (ImGui::Button(IMGUI_JP("ウィンドウを追加"))) { m_ColliderWindows.push_back({0.0f, 0.1f}); }

	// Provide a convenience button to re-enter the current state (calls derived Enter)
	if (ImGui::Button(IMGUI_JP("リセットして再実行"))) {
		// Call Enter on this state instance (virtual -> derived override will be invoked)
		this->Enter();
	}
}

} // PlayerState.

