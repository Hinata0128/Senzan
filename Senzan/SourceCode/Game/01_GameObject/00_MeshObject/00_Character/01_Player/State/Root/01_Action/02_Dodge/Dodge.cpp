#include "Dodge.h"

#include "00_MeshObject/00_Character/01_Player/Player.h"

#include "Game/03_Collision/00_Core/ColliderBase.h"
#include "Game/03_Collision/00_Core/Ex_CompositeCollider/CompositeCollider.h"
#include "Game/05_InputDevice/VirtualPad.h"
#include "System/Singleton/CollisionDetector/CollisionDetector.h"
#include "System/Singleton/CameraManager/CameraManager.h"
#include "System/Singleton/ImGui/CImGuiManager.h"

namespace PlayerState {
Dodge::Dodge(Player* owner)
	: Action	(owner)
	, m_InputVec{}
	, m_Distance(250.f)
	, m_MaxTime(10.8f)
	, m_currentTime(0.f)
{

    // 被ダメコライダーのポインタを保持.
    const auto& internal_colliders = m_pOwner->m_upColliders->GetInternalColliders();
    for (const std::unique_ptr<ColliderBase>& collider_ptr : internal_colliders)
    {
        if (collider_ptr && collider_ptr->GetMyMask() == eCollisionGroup::Player_Damage) // GetID()は仮の関数
        {
            m_pDamageDetectionCollider = collider_ptr.get();
            break;
        }
    }
}

Dodge::~Dodge()
{
}

void Dodge::Enter()
{
    Action::Enter();

    if (m_pDamageDetectionCollider != nullptr)
        CollisionDetector::GetInstance().UnregisterCollider(m_pDamageDetectionCollider);
	
	m_currentTime = 0.f;

	DirectX::XMFLOAT2 input_vec = VirtualPad::GetInstance().GetAxisInput(VirtualPad::eGameAxisAction::Move);
	DirectX::XMFLOAT3 final_move_3d = {};

	Log::GetInstance().Info("", input_vec);

	// 入力がなければプレイヤーの向いている方向へ.
	if (MyMath::IsVector2NearlyZero(input_vec, 0.f))
	{
		final_move_3d = m_pOwner->GetTransform()->GetForward();
		final_move_3d.y = 0;
		m_InputVec = MyMath::NormalizeVector3To2D(final_move_3d);
	}
	// 入力があれば保存.
	else
	{
		// MEMO : カメラから見て後ろにしてもいいかも.

		// カメラベクトルを取得.
		DirectX::XMFLOAT3 camera_forward_vec = CameraManager::GetInstance().GetCurrentCamera()->GetForwardVec();
		DirectX::XMFLOAT3 camera_right_vec = CameraManager::GetInstance().GetCurrentCamera()->GetRightVec();
		DirectX::XMVECTOR v_camera_forward = DirectX::XMLoadFloat3(&camera_forward_vec);
		DirectX::XMVECTOR v_camera_right = DirectX::XMLoadFloat3(&camera_right_vec);

		// Y座標を削除し、水平なベクトルに変換.
		v_camera_forward = DirectX::XMVectorSetY(v_camera_forward, 0.0f);
		v_camera_right = DirectX::XMVectorSetY(v_camera_right, 0.0f);

		// 正規化.
		v_camera_forward = DirectX::XMVector3Normalize(v_camera_forward);
		v_camera_right = DirectX::XMVector3Normalize(v_camera_right);

		// 入力とカメラベクトル.
		DirectX::XMVECTOR v_movement_z = DirectX::XMVectorScale(v_camera_forward, input_vec.y);
		DirectX::XMVECTOR v_movement_x = DirectX::XMVectorScale(v_camera_right, input_vec.x);

		// 最終的な移動方向ベクトル.
		DirectX::XMVECTOR v_final_move = DirectX::XMVectorAdd(v_movement_z, v_movement_x);

		// 正規化し.
		DirectX::XMVECTOR v_normalized_move = DirectX::XMVector3Normalize(v_final_move);

		DirectX::XMStoreFloat3(&final_move_3d, v_normalized_move);

		// 最終的な回避方向をm_InputVecに設定 (XZ平面の正規化ベクトル)
		m_InputVec = { final_move_3d.x, final_move_3d.z };
	}

	// 移動方向を向く.
	m_pOwner->GetTransform()->RotateToDirection(final_move_3d);

	SoundManager::GetInstance().Play("Dodge");
	SoundManager::GetInstance().SetVolume("Dodge", 8500);

    // 回避時に後方へ Dash エフェクトを再生
    {
        const float BACK_OFFSET = 1.5f; // プレイヤーの後方へ出す距離
        DirectX::XMFLOAT3 forward = m_pOwner->GetTransform()->GetForward();
        // 水平成分のみを使って後方位置を計算
        DirectX::XMFLOAT3 ownerPos = m_pOwner->GetPosition();
        DirectX::XMFLOAT3 backPos{
            ownerPos.x - forward.x * BACK_OFFSET,
            ownerPos.y,
            ownerPos.z - forward.z * BACK_OFFSET
        };

        // プレイヤーの向きに合わせてエフェクトも回転する（Y軸のみ使用）
        DirectX::XMFLOAT3 forwardVec = m_pOwner->GetTransform()->GetForward();
        float yaw = atan2f(forwardVec.x, forwardVec.z); // yaw を計算
        DirectX::XMFLOAT3 eulerRotation{ 0.0f, yaw, 0.0f };

        m_pOwner->PlayEffectAtWorldPos("Dash", backPos, eulerRotation, 1.0f);
    }
}

void Dodge::Update()
{
    Action::Update();
}

void Dodge::LateUpdate()
{
    Action::LateUpdate();

}

void Dodge::Draw()
{
    Action::Draw();
}

void Dodge::Exit()
{
    Action::Exit();

    if (m_pDamageDetectionCollider != nullptr)
        CollisionDetector::GetInstance().RegisterCollider(*m_pDamageDetectionCollider);
}

} // PlayerState.
