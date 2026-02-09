#include "AttackCombo_0.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"
#include "Game/05_InputDevice/VirtualPad.h"
#include "Resource/Mesh/02_Skin/SkinMesh.h"

#include "System/Utility/SingleTrigger/SingleTrigger.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Utility/FileManager/FileManager.h"

static constexpr float CLOSE_RANGE_THRESHOLD = 30.0f;

namespace PlayerState {

AttackCombo_0::AttackCombo_0(Player* owner)
    : Combat(owner)
{
    LoadSettings();
}

AttackCombo_0::~AttackCombo_0() {}

constexpr PlayerState::eID AttackCombo_0::GetStateID() const { return PlayerState::eID::AttackCombo_0; }

void AttackCombo_0::Enter()
{
    Combat::Enter();

    SoundManager::GetInstance().Play("Swing");
    SoundManager::GetInstance().SetVolume("Swing", 7000);

    // 初期設定.
    m_pOwner->SetIsLoop(false);
    m_pOwner->SetAnimTime(0.0);
    m_pOwner->SetAnimSpeed(m_AnimSpeed);
    m_pOwner->ChangeAnim(Player::eAnim::Attack_0);
    m_pOwner->SetAttackColliderActive(false);

    // 目標座標,方向 の算出.
    DirectX::XMFLOAT3 target_pos = m_pOwner->m_TargetPos;
    DirectX::XMVECTOR v_target_pos = DirectX::XMLoadFloat3(&target_pos);
    v_target_pos = DirectX::XMVectorSetY(v_target_pos, 0.f);
    DirectX::XMFLOAT3 player_pos = m_pOwner->GetPosition();
    DirectX::XMVECTOR v_player_pos = DirectX::XMLoadFloat3(&player_pos);
    v_player_pos = DirectX::XMVectorSetY(v_player_pos, 0.f);
    DirectX::XMVECTOR v_diff_vec = DirectX::XMVectorSubtract(v_target_pos, v_player_pos);
    DirectX::XMVECTOR v_Lenght = DirectX::XMVector3Length(v_diff_vec);
    DirectX::XMStoreFloat(&m_Distance, v_Lenght);
    v_diff_vec = DirectX::XMVector3Normalize(v_diff_vec);
    DirectX::XMFLOAT3 diff_vec; DirectX::XMStoreFloat3(&diff_vec, v_diff_vec);
    m_pOwner->GetTransform()->RotateToDirection(diff_vec);

    m_pOwner->m_MoveVec = diff_vec;
}

void AttackCombo_0::Update()
{
//    static bool isStop = false;
//#if _DEBUG
//    ImGui::Begin(IMGUI_JP("AttackCombo_0 デバッグ"));
//
//    ImGui::Checkbox(IMGUI_JP("ストップ"), &isStop);
//
//    RenderColliderWindowsUI("AttackCombo_0 Collider Windows");
//
//    bool isAccepting = (m_currentTime >= m_ComboStartTime && m_currentTime <= m_ComboEndTime);
//    if (isAccepting) {
//        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), IMGUI_JP("入力受付中"));
//    }
//    else {
//        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), IMGUI_JP("受付外"));
//    }
//    ImGui::Separator();
//    ImGui::DragFloat(IMGUI_JP("コンボ入力開始時間 (ComboStart)"), &m_ComboStartTime, 0.01f, 0.0f, m_ComboEndTime);
//    ImGui::DragFloat(IMGUI_JP("次コンボ最低開始時間 (ComboStart)"), &m_MinComboTransTime, 0.01f, 0.0f, m_ComboEndTime);
//    ImGui::Text(IMGUI_JP("入力受付時間: %.3f - %.3f"), m_ComboStartTime, m_ComboEndTime);
//
//
//    ImGui::Separator();
//    ImGui::Text(IMGUI_JP("時間: %.3f / %.3f"), m_currentTime, m_ComboEndTime);
//    ImGui::DragFloat(IMGUI_JP("アニメ速度"), &m_AnimSpeed, 0.1f, 0.1f, 60.0f);
//    ImGui::DragFloat(IMGUI_JP("ステート持続時間 (MaxTime)"), &m_ComboEndTime, 0.01f, 0.1f, 10.0f);
//
//    if (ImGui::Button(IMGUI_JP("Load"))) {
//        try {
//            std::filesystem::path filePath = std::filesystem::current_path() / "Data" / "Json" / "Player" / "AttackCombo" / "AttackCombo_0.json";
//            if (std::filesystem::exists(filePath)) {
//                json j = FileManager::JsonLoad(filePath);
//                if (j.contains("m_AnimSpeed")) m_AnimSpeed = j["m_AnimSpeed"].get<float>();
//                if (j.contains("m_MinComboTransTime")) m_MinComboTransTime = j["m_MinComboTransTime"].get<float>();
//                if (j.contains("m_ComboStartTime")) m_ComboStartTime = j["m_ComboStartTime"].get<float>();
//                if (j.contains("m_ComboEndTime")) m_ComboEndTime = j["m_ComboEndTime"].get<float>();
//
//                if (j.contains("ColliderWindows") && j["ColliderWindows"].is_array()) {
//                    m_ColliderWindows.clear();
//                    for (const auto& entry : j["ColliderWindows"]) {
//                        if (entry.contains("start") && entry.contains("duration")) {
//                            AddColliderWindow(entry["start"].get<float>(), entry["duration"].get<float>());
//                        }
//                    }
//                }
//
//                m_pOwner->SetAnimSpeed(m_AnimSpeed);
//            }
//        }
//        catch (...) {}
//    }
//    ImGui::SameLine();
//    if (ImGui::Button(IMGUI_JP("Save"))) {
//        try {
//            std::filesystem::path dir = std::filesystem::current_path() / "Data" / "Json" / "Player" / "AttackCombo";
//            std::filesystem::create_directories(dir);
//            std::filesystem::path filePath = dir / "AttackCombo_0.json";
//
//            json j;
//            j["m_AnimSpeed"] = m_AnimSpeed;
//            j["m_MinComboTransTime"] = m_MinComboTransTime;
//            j["m_ComboStartTime"] = m_ComboStartTime;
//            j["m_ComboEndTime"] = m_ComboEndTime;
//            j["ColliderWindows"] = json::array();
//            for (const auto& w : m_ColliderWindows) { json e; e["start"] = w.Start; e["duration"] = w.Duration; j["ColliderWindows"].push_back(e); }
//            FileManager::JsonSave(filePath, j);
//        }
//        catch (...) {}
//    }
//    if (ImGui::Button(IMGUI_JP("Restert"))) {
//        this->Enter();
//    }
//
//    ImGui::End();
//#endif

//  if (!isStop)
//    {
    Combat::Update();

        // コンボ入力受付.
    if (m_currentTime >= m_ComboStartTime && m_currentTime <= m_ComboEndTime)
    {
        if (VirtualPad::GetInstance().IsActionPress(VirtualPad::eGameAction::Attack))
        {
            m_IsComboAccepted = true;
        }
    }

        // コンボが確定しており、かつ最短遷移時間を過ぎていれば即座に次へ.
    if (m_IsComboAccepted && m_currentTime >= m_MinComboTransTime)
    {
        m_pOwner->ChangeState(PlayerState::eID::AttackCombo_1);
        return;
    }

        // コンボ不成立のまま最後まで到達.
    if (m_currentTime >= m_ComboEndTime)
    {
        m_pOwner->ChangeState(PlayerState::eID::Idle);
    }
//    }
}

void AttackCombo_0::LateUpdate()
{
    Combat::LateUpdate();

    float dt = m_pOwner->GetDelta();
    DirectX::XMFLOAT3 moveDirection = { m_pOwner->m_MoveVec.x, 0.0f, m_pOwner->m_MoveVec.z };
    float movement_speed = m_Distance / m_ComboEndTime;
    float move_amount = movement_speed * dt;
    DirectX::XMFLOAT3 movement = {};
    movement.x = moveDirection.x * move_amount;
    movement.y = 0.f;
    movement.z = moveDirection.z * move_amount;
    m_pOwner->AddPosition(movement);
}

void AttackCombo_0::Draw()
{
    Combat::Draw();
}

void AttackCombo_0::Exit()
{
    Combat::Exit();
    m_IsComboAccepted = false;
    m_pOwner->SetAttackColliderActive(false);
}

} // namespace PlayerState

