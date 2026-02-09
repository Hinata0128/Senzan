#include "Dead.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"
#include "System/Singleton/ImGui/CImGuiManager.h"

namespace PlayerState {
Dead::Dead(Player* owner)
	: System(owner)
{
}
Dead::~Dead()
{
}

// IDの取得.
constexpr PlayerState::eID Dead::GetStateID() const
{
	return PlayerState::eID::Dead;
}

void Dead::Enter()
{
	// 死亡アニメーションの設定
	m_pOwner->SetIsLoop(false);
	m_pOwner->SetAnimTime(0.0);
	m_pOwner->SetAnimSpeed(1.0f); // 必要に応じて調整
	m_pOwner->ChangeAnim(Player::eAnim::Dead);

	// 死亡フラグのセット
	m_pOwner->m_IsDead = true;
}

void Dead::Update()
{
	// アニメーション終了で何か処理（例：リスポーンやゲームオーバー画面へ）
	if (m_pOwner->IsAnimEnd(Player::eAnim::Dead))
	{
		// 必要ならステート遷移や処理を追加
	}
}

void Dead::LateUpdate()
{
	// 死亡中は移動や入力を無効化
}

void Dead::Draw()
{
	// デバッグ表示
	ImGui::Begin("Dead State Debug");
	ImGui::Text("Player is Dead");
	ImGui::End();
}

void Dead::Exit()
{
	// 死亡フラグ解除
	m_pOwner->m_IsDead = false;
}
} // PlayerState.
