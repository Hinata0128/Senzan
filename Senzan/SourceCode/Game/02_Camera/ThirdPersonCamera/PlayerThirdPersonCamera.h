#pragma once

#include "ThirdPersonCamera.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"

/**********************************************************************************
* @author    : 淵脇 未来.
* @date      : 2025/11/4.
* @brief     : プレイヤーの用三人称カメラ.
**********************************************************************************/
class PlayerThirdPersonCamera final
	: public ThirdPersonCamera
{
public:
	PlayerThirdPersonCamera(const Player& target)
		: ThirdPersonCamera()
		, m_LookOffset{ 0.0f, 2.5f, 0.0f } 
		, m_rTargetPlayer{ target }
	{
		m_Distance = 15.f;
	}

	void Update()override
	{
		// オフセットとPlayer座標を足した値をLookに設定.
		DirectX::XMVECTOR v_player_position = {};
		DirectX::XMVECTOR v_look_offset = {};

		v_player_position = DirectX::XMLoadFloat3(&m_rTargetPlayer.get().GetPosition());
		v_look_offset = DirectX::XMLoadFloat3(&m_LookOffset);

		DirectX::XMVECTOR v_look_position = DirectX::XMVectorAdd(v_player_position, v_look_offset);

		DirectX::XMFLOAT3 look_position = {};
		DirectX::XMStoreFloat3(&look_position ,v_look_position);

		SetLook(look_position);

		ThirdPersonCamera::Update();
	}

private:
	DirectX::XMFLOAT3 m_LookOffset; 

	std::reference_wrapper<const Player> m_rTargetPlayer;
};
