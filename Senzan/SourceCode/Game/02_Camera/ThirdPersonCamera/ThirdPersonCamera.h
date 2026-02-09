#pragma once
#include "Game/02_Camera/CameraBase.h"

class Player;


/**********************************************************************************
* @author    : 淵脇 未来.
* @date      : 2025/10/5.
* @brief     : 三人称カメラ.
**********************************************************************************/

class ThirdPersonCamera
	: public CameraBase
{
public:
	ThirdPersonCamera();
	virtual ~ThirdPersonCamera()override;

	virtual void Update()override;

private:
	DirectX::XMFLOAT2 m_MoveVec; // カメラの移動方向.
};
