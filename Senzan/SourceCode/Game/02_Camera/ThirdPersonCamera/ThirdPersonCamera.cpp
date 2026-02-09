#include "ThirdPersonCamera.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"

#include "Game/05_InputDevice/Input.h"
#include "System/Singleton/ImGui/CImGuiManager.h"

static constexpr float PITCH_LIMIT_RAD = D3DXToRadian(60.0f);

ThirdPersonCamera::ThirdPersonCamera()
	: CameraBase	()
	, m_MoveVec		( 0.0f, 0.0f )
{
}

//-----------------------------------------------------------------------------------.

ThirdPersonCamera::~ThirdPersonCamera()
{
}

// -----------------------------------------------------------------------------------.

void ThirdPersonCamera::Update()
{
    DirectX::XMVECTOR moveVecXM = {};    // XMVECTORで計算するための中間変数。

    // コントローラー入力.
    if (Input::IsRStickActive())
    {
        // 右スティックの入力方向を取得.
        const DirectX::XMFLOAT2 RStickDirection = Input::GetRStickDirection();
        DirectX::XMVECTOR stickDirectionXM = DirectX::XMLoadFloat2(&RStickDirection);
        // 反転.
        DirectX::XMVECTOR negY = DirectX::XMVectorSet(1.0f, -1.0f, 1.0f, 1.0f);
        moveVecXM = DirectX::XMVectorMultiply(stickDirectionXM, negY);

        // 感度を適用する.
        DirectX::XMVECTOR sensitivityXM = XMLoadFloat2(&m_ControllerSensitivity
        );
        moveVecXM = DirectX::XMVectorMultiply(moveVecXM, sensitivityXM);
    }
    else // マウス入力.
    {

        // マウスがウィンドウ中心に固定されている場合のみマウス移動を受け付ける
        // さらに ImGui がマウスをキャプチャしている場合は UI 優先のため無視する
        if (Input::IsCenterMouseCursor() && !ImGui::GetIO().WantCaptureMouse)
        {
            // マウスの相対移動量を計算.
            DirectX::XMFLOAT2 mouse_delta = Input::GetClientCursorDelta();
            moveVecXM = DirectX::XMLoadFloat2(&mouse_delta);
             
            // Y反転.
            DirectX::XMVECTOR negY = DirectX::XMVectorSet(1.0f, -1.0f, 1.0f, 1.0f);
            moveVecXM = DirectX::XMVectorMultiply(moveVecXM, negY);

            // 感度を適用する.
            DirectX::XMVECTOR sensitivityXM = XMLoadFloat2(&m_MouseSensitivity);
            moveVecXM = DirectX::XMVectorMultiply(moveVecXM, sensitivityXM);
        }
        else
        {
            // マウス入力を受け付けないので移動ベクトルはゼロのままにする
        }
    }

    DirectX::XMStoreFloat2(&m_MoveVec, moveVecXM);


    m_Yaw += m_MoveVec.x;  
    m_Pitch += m_MoveVec.y; 

    // 上下の制限.
    m_Pitch = std::clamp(m_Pitch, -DirectX::XM_PIDIV2 * 0.05f, DirectX::XM_PIDIV2 * 0.6f);

    // YawとPitchから前方ベクトルとカメラ位置を計算する.
    CalculateMoveVector();

    // ビュー・プロジェクション行列を計算.
    ViewAndProjectionUpdate();
}
