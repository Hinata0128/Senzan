#include "Run.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"
#include "../../../Root.h"

#include "Game/02_Camera/CameraBase.h"

#include "Game/05_InputDevice/VirtualPad.h"    
#include "Game/04_Time/Time.h"   

#include "System/Singleton/CameraManager/CameraManager.h"
#include "System/Singleton/ResourceManager/ResourceManager.h"
#include "System/Singleton/Debug/Log/DebugLog.h"


static constexpr double RUN_ANIM_SPEED = 3.0;

namespace PlayerState {
Run::Run(Player* owner)
	: Movement  (owner)
    , m_DartEffect(nullptr)
    , m_EffectHandle(-1)
{
    // ResourceManagerからエフェクトを取得
    m_DartEffect = ResourceManager::GetEffect("Dart");
}

Run::~Run()
{
}

// IDの取得.
constexpr PlayerState::eID Run::GetStateID() const
{
    return PlayerState::eID::Run;
}

void Run::Enter()
{
    Movement::Enter();

    m_pOwner->SetIsLoop(true);
    m_pOwner->SetAnimSpeed(RUN_ANIM_SPEED);
    m_pOwner->ChangeAnim(Player::eAnim::Run);

    // エフェクトを再生
    if (m_DartEffect != nullptr)
    {
        auto manager = EffekseerManager::GetInstance().GetManager();
        DirectX::XMFLOAT3 pos = m_pOwner->GetPosition();
        m_EffectHandle = manager->Play(m_DartEffect, pos.x, pos.y, pos.z);
    }
}

void Run::Update()
{
    Movement::Update();

    // 移動ベクトルの算出.
    CalculateMoveVec();

    // エフェクトの更新と位置追従
    if (m_EffectHandle != -1)
    {
        EffekseerManager::GetInstance().UpdateHandle(m_EffectHandle);
        
        // エフェクトの位置をプレイヤーに追従
        auto manager = EffekseerManager::GetInstance().GetManager();
        DirectX::XMFLOAT3 pos = m_pOwner->GetPosition();
        manager->SetLocation(m_EffectHandle, pos.x, pos.y, pos.z);
    }
}

void Run::LateUpdate()
{
    Movement::LateUpdate(); 
    
    m_pOwner->AddPosition(m_pOwner->m_MoveVec.x, 0.f, m_pOwner->m_MoveVec.y);

    // エフェクトの描画
    if (m_EffectHandle != -1)
    {
        auto* camera = CameraManager::GetInstance().GetCurrentCamera().get();
        EffekseerManager::GetInstance().RenderHandle(m_EffectHandle, camera);
    }
}

void Run::Draw()
{
    Movement::Draw();
}

void Run::Exit()
{
    Movement::Exit();

    // エフェクトを停止
    if (m_EffectHandle != -1)
    {
        auto manager = EffekseerManager::GetInstance().GetManager();
        manager->StopEffect(m_EffectHandle);
        m_EffectHandle = -1;
    }
}

void Run::CalculateMoveVec()
{
    // 入力を取得.
    DirectX::XMFLOAT2 input_vec = VirtualPad::GetInstance().GetAxisInput(VirtualPad::eGameAxisAction::Move);

    // 入力がなければ待機へ遷移.
    if (MyMath::IsVector2NearlyZero(input_vec, 0.f)) {
        m_pOwner->ChangeState(PlayerState::eID::Idle);
        return;
    }

    // それぞれベクトルを取得.
    DirectX::XMFLOAT3 camera_forward_vec = CameraManager::GetInstance().GetCurrentCamera()->GetForwardVec();
    DirectX::XMFLOAT3 camera_right_vec = CameraManager::GetInstance().GetCurrentCamera()->GetRightVec();
    DirectX::XMVECTOR v_camera_forward = DirectX::XMLoadFloat3(&camera_forward_vec);
    DirectX::XMVECTOR v_camera_right = DirectX::XMLoadFloat3(&camera_right_vec);

    // Y座標を削除.
    v_camera_forward = DirectX::XMVectorSetY(v_camera_forward, 0.0f);
    v_camera_right = DirectX::XMVectorSetY(v_camera_right, 0.0f);

    // 正規化.
    v_camera_forward = DirectX::XMVector3Normalize(v_camera_forward);
    v_camera_right = DirectX::XMVector3Normalize(v_camera_right);

    // 入力とベクトルを合成.
    DirectX::XMVECTOR v_movement_z = DirectX::XMVectorScale(v_camera_forward, input_vec.y);
    DirectX::XMVECTOR v_movement_x = DirectX::XMVectorScale(v_camera_right, input_vec.x);

    // 最終的な移動ベクトルを合成.
    DirectX::XMVECTOR v_final_move = DirectX::XMVectorAdd(v_movement_z, v_movement_x);

    // 速度 * delta * 時間尺度.
    float speed_and_delta = m_pOwner->m_RunMoveSpeed * m_pOwner->GetDelta();
    v_final_move = DirectX::XMVectorScale(v_final_move, speed_and_delta);
    DirectX::XMFLOAT3 final_move;
    DirectX::XMStoreFloat3(&final_move, v_final_move);

    // 移動加算.
    m_pOwner->m_MoveVec.x = final_move.x;
    m_pOwner->m_MoveVec.y = final_move.z;
}

} // PlayerState.
