#include "AttackCombo_2.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"
#include "Game/05_InputDevice/VirtualPad.h"
#include "Resource/Mesh/02_Skin/SkinMesh.h"

#include "System/Utility/SingleTrigger/SingleTrigger.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Singleton/CameraManager/CameraManager.h"
#include "System/Utility/FileManager/FileManager.h"

// 攻撃開始までの速度.
static constexpr double AttackCombo_0_ANIM_SPEED_0 = 0.04;

static constexpr float CLOSE_RANGE_THRESHOLD = 20.0f;    // Bossまでの距離に置いて近いと判断する.

namespace PlayerState {
AttackCombo_2::AttackCombo_2(Player* owner)
    : Combat(owner)
{
    LoadSettings();
}
AttackCombo_2::~AttackCombo_2()
{
}

// IDの取得.
constexpr PlayerState::eID AttackCombo_2::GetStateID() const
{
    return PlayerState::eID::AttackCombo_2;
}

void AttackCombo_2::Enter()
{
    SoundManager::GetInstance().Play("Swing");
    SoundManager::GetInstance().SetVolume("Swing", 7000);

    Combat::Enter();

    // アニメーション設定.
    m_pOwner->SetIsLoop(false);
    m_pOwner->SetAnimTime(0.0);
    m_pOwner->SetAnimSpeed(m_AnimSpeed);
    m_pOwner->ChangeAnim(Player::eAnim::Attack_2);

    // 当たり判定を無効化（ステート側で自動切替）.
    m_pOwner->SetAttackColliderActive(false);

    // 距離算出用座標.
    DirectX::XMFLOAT3 target_pos = m_pOwner->m_TargetPos;
    DirectX::XMVECTOR v_target_pos = DirectX::XMLoadFloat3(&target_pos);
    v_target_pos = DirectX::XMVectorSetY(v_target_pos, 0.f);
    DirectX::XMFLOAT3 player_pos = m_pOwner->GetPosition();
    DirectX::XMVECTOR v_player_pos = DirectX::XMLoadFloat3(&player_pos);
    v_player_pos = DirectX::XMVectorSetY(v_player_pos, 0.f);

    // 距離算出.
    DirectX::XMVECTOR v_Lenght = {};
    DirectX::XMFLOAT3 diff_vec = {};
    DirectX::XMVECTOR v_diff_vec = DirectX::XMVectorSubtract(v_target_pos, v_player_pos);
    v_Lenght = DirectX::XMVector3Length(v_diff_vec);
    DirectX::XMStoreFloat(&m_Distance, v_Lenght);
    v_diff_vec = DirectX::XMVector3Normalize(v_diff_vec);
    DirectX::XMStoreFloat3(&diff_vec, v_diff_vec);

    // 敵の方向を向く.
    m_pOwner->GetTransform()->RotateToDirection(diff_vec);

    // 入力を取得.
    DirectX::XMFLOAT2 input_vec = VirtualPad::GetInstance().GetAxisInput(VirtualPad::eGameAxisAction::Move);
    DirectX::XMVECTOR input_vec_v = DirectX::XMVectorSet(input_vec.x, 0.0f, input_vec.y, 0.0f);

    // カメラの前方向と右方向を取得.
    DirectX::XMFLOAT3 cam_forward = CameraManager::GetInstance().GetLookDirection();
    cam_forward.y = 0.0f;
    DirectX::XMVECTOR v_forward = DirectX::XMLoadFloat3(&cam_forward);
    v_forward = DirectX::XMVector3Normalize(v_forward);

    // 右方向を計算.
    DirectX::XMVECTOR v_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR v_right = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(v_up, v_forward));

    // 入力ベクトルをカメラ基準でワールド空間に変換.
    DirectX::XMVECTOR v_move = DirectX::XMVectorZero();
    v_move = DirectX::XMVectorAdd(
        DirectX::XMVectorScale(v_right, input_vec.x),
        DirectX::XMVectorScale(v_forward, input_vec.y)
    );

    float inputLenSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(v_move));
    if (inputLenSq > EPSILON_E4)
    {
        // 入力ベクトルを正規化して m_MoveVec に格納.
        DirectX::XMVECTOR v_in_norm = DirectX::XMVector3Normalize(v_move);
        DirectX::XMFLOAT3 out_move = {};
        DirectX::XMStoreFloat3(&out_move, v_in_norm);
        m_pOwner->m_MoveVec = out_move;
    }
    else
    {
        // 敵方向を使用.
        m_pOwner->m_MoveVec = diff_vec;
    }
}

void AttackCombo_2::Update()
{
    static bool isStop = false;
//#if _DEBUG
//    ImGui::Begin(IMGUI_JP("AttackCombo_2 デバッグ"));
//
//    ImGui::Checkbox(IMGUI_JP("ストップ"), &isStop);
//
//    RenderColliderWindowsUI("AttackCombo_2 Collider Windows");
//
//    // Show whether combo input is currently accepted (green) or not (red)
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
//            std::filesystem::path filePath = std::filesystem::current_path() / "Data" / "Json" / "Player" / "AttackCombo" / "AttackCombo_2.json";
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
//            std::filesystem::path filePath = dir / "AttackCombo_2.json";
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

    if (!isStop)
    {
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
            m_pOwner->ChangeState(PlayerState::eID::AttackCombo_0);
            return;
        }

        // コンボ不成立のまま最後まで到達.
        if (m_currentTime >= m_ComboEndTime)
        {
            m_pOwner->ChangeState(PlayerState::eID::Idle);
        }
    }
}
void AttackCombo_2::LateUpdate()
{
    Combat::LateUpdate();

    // 移動量の算出.
    float delta_time = m_pOwner->GetDelta();
    float movement_speed = m_Distance / m_ComboEndTime;
    float move_amount = movement_speed * delta_time;

    // 移動方向.
    DirectX::XMFLOAT3 moveDirection = { m_pOwner->m_MoveVec.x, 0.0f, m_pOwner->m_MoveVec.z };

    // 移動量加算.
    DirectX::XMFLOAT3 movement = {};
    movement.x = moveDirection.x * move_amount;
    movement.y = 0.f;
    movement.z = moveDirection.z * move_amount;

    m_pOwner->AddPosition(movement);
}
void AttackCombo_2::Draw()
{
    Combat::Draw();
}
void AttackCombo_2::Exit()
{
    Combat::Exit();
}

} // PlayerState.
