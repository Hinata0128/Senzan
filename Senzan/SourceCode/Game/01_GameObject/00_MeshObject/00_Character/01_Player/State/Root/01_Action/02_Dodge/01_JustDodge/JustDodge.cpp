#include "JustDodge.h"
#include "AfterImage.h"

#include "00_MeshObject/00_Character/01_Player/Player.h"

#include "Game/05_InputDevice/VirtualPad.h"
#include "System/Singleton/CameraManager/CameraManager.h"
#include "System/Singleton/PostEffectManager/PostEffectManager.h"
#include "System/Singleton/CombatCoordinator/CombatCoordinator.h"
#include "Resource/Effect/EffectResource.h"
#include "Resource/Mesh/02_Skin/SkinMesh.h"

#include "../Dodge.h"

#include <cmath>


static constexpr double JUSTDODGE_ANIM_SPEED = 2.00;

namespace PlayerState {
JustDodge::JustDodge(Player* owner)
	: Dodge(owner)
	, m_pAfterImage(std::make_unique<AfterImage>())
	, m_AfterImageTimer(0.0f)
	, m_AfterImageInterval(0.1f)
	, m_AfterImageLifeTime(0.4f)
    , m_IsEffectStart(false)
{
}
JustDodge::~JustDodge()
{
}

// IDの取得.
constexpr PlayerState::eID JustDodge::GetStateID() const
{
	return PlayerState::eID::JustDodge;
}

void JustDodge::Enter()
{
    Dodge::Enter();

	m_Distance = 250.f;
	m_MaxTime = 2.5f;

    m_pOwner->SetIsLoop(false);
    m_pOwner->SetAnimSpeed(JUSTDODGE_ANIM_SPEED);
    m_pOwner->ChangeAnim(Player::eAnim::Dodge);

    PostEffectManager::GetInstance().StartCircleGrayEffect(0.2f, m_MaxTime - 1.0f, 0.5f);

    m_IsEffectStart = false;

    // ゲージ増加
    m_pOwner->m_CurrentUltValue += 900.0f * m_pOwner->ComboMultiplier();

    // タイムスケールを戻す && Boss攻撃判定を消す.
    CombatCoordinator::GetInstance().StartJustDodge(0.1f);

    // 残像エフェクトの初期化.
    m_AfterImageTimer = 0.0f;
    m_pAfterImage->Clear();
    m_pAfterImage->SetLifeTime(m_AfterImageLifeTime);
    m_pAfterImage->SetSpawnInterval(m_AfterImageInterval);
    m_pAfterImage->SetStartAlpha(0.6f);   // 開始時のアルファ値.
    m_pAfterImage->SetDarkness(0.3f);     // 黒みの強さ.

}

void JustDodge::Update()
{
    //Dodge::Update();

    // 攻撃開始時にボスの方を向く（即時）
    if (m_pOwner)
    {
        DirectX::XMFLOAT3 pos = m_pOwner->GetPosition();
        DirectX::XMFLOAT3 target = m_pOwner->m_TargetPos;
        float dx = target.x - pos.x;
        float dz = target.z - pos.z;
        // 同じ手法でYawを計算してセット
        float angle_radian = std::atan2f(dx, dz);
        m_pOwner->SetRotationY(angle_radian);
    }

	// デルタタイムの計算.
	double animSpeed = MyMath::IsNearlyEqual(m_pOwner->m_AnimSpeed, 0.0) ? 0.0 : m_pOwner->m_AnimSpeed;
	float deltaTime = static_cast<float>(animSpeed) * m_pOwner->GetDelta();

	// 時間と距離の更新.
	m_currentTime += deltaTime;
    // 入力を取得.
    DirectX::XMFLOAT2 input_vec = VirtualPad::GetInstance().GetAxisInput(VirtualPad::eGameAxisAction::Move);

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

    // 残像エフェクトの更新.
    float realDelta = m_pOwner->GetDelta();
    m_AfterImageTimer += realDelta;

    // 一定間隔で残像を生成.
    if (m_AfterImageTimer >= m_AfterImageInterval)
    {
        m_AfterImageTimer = 0.0f;

        // 現在のプレイヤー情報を取得して残像を追加.
        m_pAfterImage->AddImage(
            m_pOwner->GetPosition(),
            m_pOwner->GetRotation(),
            m_pOwner->GetScale(),
            m_pOwner->m_AnimTimer,
            m_pOwner->m_AnimNo
        );
    }

    // 回避攻撃再生 - Player側で管理するのでステートではStartJustDodgeEffectを呼び出すだけ
    if (!m_IsEffectStart && m_currentTime >= 2.0f)
    {
        DirectX::XMFLOAT3 playerPos = m_pOwner->GetPosition();
        DirectX::XMFLOAT3 bossPos = m_pOwner->m_TargetPos;
        // エフェクトの終点をボスより奥にする（例: +10.0f）
        m_pOwner->StartJustDodgeEffect(playerPos, bossPos, 10.0f, 1.0f, 2.0f);
        m_IsEffectStart = true;
    }

    // 残像の更新（フェードアウト処理）.
    m_pAfterImage->Update(realDelta);

	// 回避完了.
	if (m_currentTime >= m_MaxTime)
	{
		m_pOwner->ChangeState(PlayerState::eID::Idle);
	}
}

void JustDodge::LateUpdate()
{
    Dodge::LateUpdate();

    m_pOwner->AddPosition(m_pOwner->m_MoveVec.x * 0.5f, 0.f, m_pOwner->m_MoveVec.y * 0.5f);
}

void JustDodge::Draw()
{
    // 残像を描画（プレイヤー本体より先に描画）.
    if (m_pAfterImage && m_pAfterImage->HasImages())
    {
        if (auto pMesh = m_pOwner->GetAttachMesh().lock())
        {
            if (auto pSkinMesh = std::dynamic_pointer_cast<SkinMesh>(pMesh))
            {
                m_pAfterImage->Draw(pSkinMesh, m_pOwner->m_pAnimCtrl);
            }
        }
    }

    // エフェクトは Player 側で描画されるのでここでは何もしない
    Dodge::Draw();
}

void JustDodge::Exit()
{
	Dodge::Exit();
    PostEffectManager::GetInstance().SetGray(false);

    // タイムスケールを戻す.
    CombatCoordinator::GetInstance().EndJustDodge();

    // エフェクトは Player 側で管理されるので、ステート終了時には何もしない
    // （エフェクトは duration 経過後に自然終了する）
}

} // PlayerState.

