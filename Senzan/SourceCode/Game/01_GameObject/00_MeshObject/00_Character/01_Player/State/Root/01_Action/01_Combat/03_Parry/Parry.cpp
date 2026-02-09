#include "Parry.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"
#include "System/Singleton/PostEffectManager/PostEffectManager.h"
#include "Game/04_Time/Time.h"
#include <cmath>
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Singleton/CombatCoordinator/CombatCoordinator.h"

namespace PlayerState {
Parry::Parry(Player* owner)
	: Combat(owner)
{
    m_ForceOffset = { 0.f,0.f,-15.f };
}

Parry::~Parry()
{
}

// IDの取得.
constexpr PlayerState::eID Parry::GetStateID() const
{
	return PlayerState::eID::Parry;
}

void Parry::Enter()
{
    SoundManager::GetInstance().Play("ReadyParry"); 
    SoundManager::GetInstance().SetVolume("ReadyParry",8500); 

    m_pOwner->SetDamageColliderActive(false);
    m_pOwner->SetParryColliderActive(true);

    m_pOwner->SetTimeScale(1.0f);

    m_ElapsedTime = 0.0f;
    m_IsPaused = false;

    // アニメーション設定
    m_pOwner->SetIsLoop(false);
    m_pOwner->SetAnimTime(0.0f);
    m_pOwner->SetAnimSpeed(1.0f);
    m_pOwner->ChangeAnim(Player::eAnim::Parry);
}

void Parry::Update()
{
    Combat::Update();

#if ENABLE_FRAMECAPTURE_IMGUI
    if (ImGui::Begin(IMGUI_JP("Parry Debug")))
    {
        ImGui::Checkbox(IMGUI_JP("強制オフセットを使用"), &m_UseForceOffset);
        float off[3] = { m_ForceOffset.x, m_ForceOffset.y, m_ForceOffset.z };
        if (ImGui::DragFloat3(IMGUI_JP("Force Offset (x,y,z)"), off, 0.1f, -50.0f, 50.0f))
        {
            m_ForceOffset = { off[0], off[1], off[2] };
        }
        if (ImGui::Button(IMGUI_JP("Apply Offset Now")))
        {
            if (m_UseForceOffset && m_pOwner)
            {
                // ターゲット基準で回転させたオフセットを適用
                DirectX::XMFLOAT3 target_pos = m_pOwner->m_TargetPos;
                DirectX::XMVECTOR v_target_pos = DirectX::XMLoadFloat3(&target_pos);
                DirectX::XMFLOAT3 player_pos = m_pOwner->GetPosition();
                DirectX::XMVECTOR v_player_pos = DirectX::XMLoadFloat3(&player_pos);

                // 水平方向のみで角度を決定
                v_target_pos = DirectX::XMVectorSetY(v_target_pos, 0.f);
                v_player_pos = DirectX::XMVectorSetY(v_player_pos, 0.f);

                DirectX::XMVECTOR v_diff = DirectX::XMVectorSubtract(v_target_pos, v_player_pos);
                v_diff = DirectX::XMVector3Normalize(v_diff);
                DirectX::XMFLOAT3 diff; DirectX::XMStoreFloat3(&diff, v_diff);
                float rad = std::atan2f(diff.x, diff.z);

                // オフセットを Y 回転で回す（高さは保持）
                DirectX::XMVECTOR v_offset = DirectX::XMLoadFloat3(&m_ForceOffset);
                DirectX::XMVECTOR v_off_y = DirectX::XMVectorSplatY(v_offset);
                DirectX::XMVECTOR v_off_xz = DirectX::XMVectorSetY(v_offset, 0.f);
                DirectX::XMMATRIX rot = DirectX::XMMatrixRotationY(rad);
                DirectX::XMVECTOR v_rotated = DirectX::XMVector3Transform(v_off_xz, rot);
                v_rotated = DirectX::XMVectorSetY(v_rotated, DirectX::XMVectorGetY(v_off_y));

                DirectX::XMVECTOR v_new_pos = DirectX::XMVectorAdd(v_target_pos, v_rotated);
                DirectX::XMFLOAT3 new_pos; DirectX::XMStoreFloat3(&new_pos, v_new_pos);
                m_pOwner->SetPosition(new_pos);
            }
        }
        ImGui::End();
    }
#endif
    
    // パリィ成功を待つ間、所定時間経過したらアニメ速度を0にして停止させる
    if (m_pOwner->IsParry())
    {
        if (!m_IsFastTime)
        {
            m_pOwner->SetAnimSpeed(2.3f);
            m_pOwner->SetAnimTime(static_cast<float>(m_PauseThreshold));
            m_IsFastTime = true;
            m_ElapsedTime = 0.f;

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
            float rad = std::atan2f(diff_vec.x, diff_vec.z);            
            m_pOwner->GetTransform()->SetRotationY(rad);

            m_pOwner->m_MoveVec = diff_vec;

                // 雪玉によるパリィだったらオフセット適用をスキップ
            if (CombatCoordinator::GetInstance().WasLastParriedBySnowball())
            {
                CombatCoordinator::GetInstance().ClearLastParriedFlag();
            }
            else
            {
                // ターゲット基準で回転させたオフセットを適用（高さは保持）
                DirectX::XMVECTOR v_offset = DirectX::XMLoadFloat3(&m_ForceOffset);
                DirectX::XMVECTOR v_off_y = DirectX::XMVectorSplatY(v_offset);
                DirectX::XMVECTOR v_off_xz = DirectX::XMVectorSetY(v_offset, 0.f);
                DirectX::XMMATRIX rot = DirectX::XMMatrixRotationY(rad);
                DirectX::XMVECTOR v_rotated = DirectX::XMVector3Transform(v_off_xz, rot);
                v_rotated = DirectX::XMVectorSetY(v_rotated, DirectX::XMVectorGetY(v_off_y));
                DirectX::XMVECTOR v_new_pos = DirectX::XMVectorAdd(v_target_pos, v_rotated);
                DirectX::XMFLOAT3 new_pos; DirectX::XMStoreFloat3(&new_pos, v_new_pos);
                m_pOwner->SetPosition(new_pos);
            }
            

            DirectX::XMFLOAT3 pos = { 0.f, 3.5f, 0.f };
            m_pOwner->PlayEffect("Parry_Attack", pos);
        }
        m_ElapsedTime += m_pOwner->GetDelta();

        // アニメーション終了時の処理
        if (m_ElapsedTime >= 0.6f) {
            m_pOwner->ChangeState(PlayerState::eID::Idle); // 失敗時も Idle に遷移
        }
    }
    else
    {

        m_ElapsedTime += m_pOwner->GetDelta();
        if (!m_IsPaused && m_ElapsedTime >= m_PauseThreshold)
        {
            m_pOwner->SetAnimSpeed(0.0f);
            m_IsPaused = true;
        }
        // パリィが成功しなかった場合、最大待機時間を超えたらステートを抜ける
        if (m_ElapsedTime >= m_MaxWaitTime)
        {
            if (!m_IsAnimEndStart)
            {
                m_pOwner->SetAnimTime(2.926);
                m_pOwner->SetAnimSpeed(1.0f);
                m_IsAnimEndStart = true;
            }
            else
            {
                if (m_ElapsedTime >= m_PauseThreshold + (Time::GetInstance().GetDeltaTime() * 10.f))
                {
                    m_pOwner->ChangeState(PlayerState::eID::Idle); // 失敗時も Idle に遷移
                }
            }
            return;
        }
    }

}

void Parry::LateUpdate()
{
    Combat::LateUpdate();
}

void Parry::Draw()
{
    Combat::Draw();
}

void Parry::Exit()
{
    Combat::Exit();
    // Exit で必ず再生速度を復帰
    m_pOwner->SetAnimSpeed(1.0f);
    m_IsPaused = false;
    m_IsAnimEndStart = false;
    m_IsFastTime = false;
    m_ElapsedTime = 0.0f;
    m_pOwner->m_IsSuccessParry = false;

    m_pOwner->SetDamageColliderActive(true);
    m_pOwner->SetParryColliderActive(false);
    m_pOwner->SetTimeScale(-1.0f);
}
} // PlayerState.
