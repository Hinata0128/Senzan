#include "FirstPersonCamera.h"

#include "Game/05_InputDevice/Input.h"
#include "Game/04_Time/Time.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Utility/Transform/Transform.h"

#include <DirectXMath.h>
#include <algorithm>

using namespace DirectX;

static constexpr float DEFAULT_MOVE_SPEED = 5.0f;
static constexpr float DEFAULT_SPRINT_MULT = 1.8f;
static constexpr float DEFAULT_EYE_HEIGHT = 1.7f;
static constexpr float DEFAULT_LOOK_DISTANCE = 1.0f;
static constexpr float PITCH_LIMIT = XM_PIDIV2 * 0.95f; // 上下限

// accel
static constexpr float DEFAULT_ACCEL_RATE = 1.0f; // 秒で最大倍率に到達
static constexpr float DEFAULT_MAX_ACCEL = 3.0f; // 最大倍率

FirstPersonCamera::FirstPersonCamera()
    : CameraBase()
    , m_MoveSpeed(DEFAULT_MOVE_SPEED)
    , m_SprintMultiplier(DEFAULT_SPRINT_MULT)
    , m_EyeHeight(DEFAULT_EYE_HEIGHT)
    , m_LookDistance(DEFAULT_LOOK_DISTANCE)
    , m_AccelTimerHorizontal(0.0f)
    , m_AccelTimerVertical(0.0f)
    , m_AccelRate(DEFAULT_ACCEL_RATE)
    , m_MaxAccelMul(DEFAULT_MAX_ACCEL)
{
}

FirstPersonCamera::~FirstPersonCamera()
{
}

void FirstPersonCamera::SetEyeHeight(float height) noexcept
{
    m_EyeHeight = height;
}

void FirstPersonCamera::SetMoveSpeed(float speed) noexcept
{
    m_MoveSpeed = speed;
}

void FirstPersonCamera::SetSprintMultiplier(float mul) noexcept
{
    m_SprintMultiplier = mul;
}

void FirstPersonCamera::Update()
{
    // デルタタイム
    const float dt = Time::GetInstance().GetDeltaTime();

    // --- 視点操作 (マウス or コントローラー右スティック) ---
    XMFLOAT2 moveLook = { 0.0f, 0.0f };

    // コントローラー右スティック優先
    if (Input::IsRStickActive())
    {
        XMFLOAT2 r = Input::GetRStickDirection();
        // 右スティックの Y は上下反転させる（慣習）
        moveLook.x = r.x * m_ControllerSensitivity.x;
        moveLook.y = -r.y * m_ControllerSensitivity.y;
    }
    else
    {
        // マウス：ウィンドウ中心固定かつ ImGui がキャプチャしていない場合のみ受け付ける
        if (Input::IsCenterMouseCursor() && !ImGui::GetIO().WantCaptureMouse)
        {
            XMFLOAT2 delta = Input::GetClientCursorDelta();
            moveLook.x = delta.x * m_MouseSensitivity.x;
            moveLook.y = delta.y * m_MouseSensitivity.y;
        }
    }

    m_Yaw += moveLook.x;
    m_Pitch += moveLook.y;
    m_Pitch = std::clamp(m_Pitch, -PITCH_LIMIT, PITCH_LIMIT);

    // --- 向きベクトル計算（クォータニオン） ---
    XMVECTOR q = XMQuaternionRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f);
    XMStoreFloat4(&m_spTransform.Quaternion, q);

    // 前方／右方（カメラの回転を反映、前方は視線方向）
    XMVECTOR forwardV = XMVector3Rotate(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), q);
    XMVECTOR rightV = XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), q);

    XMFLOAT3 forward;
    XMFLOAT3 right;
    XMStoreFloat3(&forward, forwardV);
    XMStoreFloat3(&right, rightV);

    m_ForwardVec = forward;
    m_RightVec = right;

    // --- 移動入力 (WASD / 左スティック) ---
    XMVECTOR moveDir = XMVectorZero();

    // キーボード
    float kbForward = (Input::IsKeyPress('W') ? 1.0f : 0.0f) - (Input::IsKeyPress('S') ? 1.0f : 0.0f);
    float kbRight   = (Input::IsKeyPress('D') ? 1.0f : 0.0f) - (Input::IsKeyPress('A') ? 1.0f : 0.0f);

    // コントローラー左スティック
    XMFLOAT2 lstick = Input::GetLStickDirection();
    float csForward = lstick.y; // スティックの Y を前方向に使う
    float csRight = lstick.x;

    // 合成（キーボード + コントローラ）
    float forwardInput = kbForward + csForward;
    float rightInput   = kbRight   + csRight;

    // 前方向はカメラの水平成分のみ（ピッチの影響を受けない移動）
    XMVECTOR forwardHorizontal = XMVectorSet(forward.x, 0.0f, forward.z, 0.0f);
    forwardHorizontal = XMVector3Normalize(forwardHorizontal);

    XMVECTOR rightHorizontal = XMVectorSet(right.x, 0.0f, right.z, 0.0f);
    rightHorizontal = XMVector3Normalize(rightHorizontal);

    // 入力ベクトル
    moveDir = XMVectorAdd(
        XMVectorScale(forwardHorizontal, forwardInput),
        XMVectorScale(rightHorizontal, rightInput)
    );

    // 垂直移動 Q/E と Space
    float verticalInput = 0.0f;
    if (Input::IsKeyPress('E')) { verticalInput += 1.0f; }
    if (Input::IsKeyPress('Q')) { verticalInput -= 1.0f; }
    if (Input::IsKeyPress(VK_SPACE)) { verticalInput += 1.0f; }

    // 加速タイマー更新（同じ入力が続くと増える、途切れるとリセット）
    // 水平
    if (std::abs(forwardInput) > 0.0001f || std::abs(rightInput) > 0.0001f)
    {
        m_AccelTimerHorizontal += dt;
    }
    else
    {
        m_AccelTimerHorizontal = 0.0f;
    }

    // 垂直
    if (std::abs(verticalInput) > 0.0001f)
    {
        m_AccelTimerVertical += dt;
    }
    else
    {
        m_AccelTimerVertical = 0.0f;
    }

    auto CalcAccelMul = [&](float timer)->float {
        if (m_AccelRate <= 0.0f) return 1.0f;
        float t = std::clamp(timer / m_AccelRate, 0.0f, 1.0f);
        // イージング的に平方を使う
        float mul = 1.0f + (m_MaxAccelMul - 1.0f) * (t * t);
        return mul;
    };

    float horizMul = CalcAccelMul(m_AccelTimerHorizontal);
    float vertMul = CalcAccelMul(m_AccelTimerVertical);

    // 正規化して速度を適用
    float len = XMVectorGetX(XMVector3Length(moveDir));
    XMVECTOR velocity = XMVectorZero();
    if (len > 0.0001f)
    {
        XMVECTOR dirNorm = XMVectorScale(moveDir, 1.0f / len);
        // スプリント（左シフト）簡易対応
        float speed = m_MoveSpeed * (Input::IsKeyPress(VK_SHIFT) ? m_SprintMultiplier : 1.0f);
        speed *= horizMul; // 加速倍率を掛ける
        velocity = XMVectorScale(dirNorm, speed * dt);
    }

    // 垂直速度
    XMVECTOR vertVelocity = XMVectorZero();
    if (std::abs(verticalInput) > 0.0001f)
    {
        float vSpeed = m_MoveSpeed * 0.8f; // 垂直は少し遅め
        vSpeed *= vertMul;
        vertVelocity = XMVectorSet(0.0f, vSpeed * verticalInput * dt, 0.0f, 0.0f);
    }

    // 現在位置を取得・更新
    XMFLOAT3 pos = m_spTransform.Position;
    XMVECTOR posV = XMLoadFloat3(&pos);
    posV = XMVectorAdd(posV, velocity);
    posV = XMVectorAdd(posV, vertVelocity);

    // Y を目の高さに固定しない（垂直移動で変化するため）
    XMFLOAT3 newPos;
    XMStoreFloat3(&newPos, posV);

    // 注視点はカメラ位置 + 視線方向（ピッチを反映）
    XMFLOAT3 look;
    XMStoreFloat3(&look, XMVectorAdd(XMLoadFloat3(&newPos), XMVectorScale(forwardV, m_LookDistance)));

    // 更新を反映
    m_spTransform.Position = newPos;
    m_LookPos = look;

    // ビュー・プロジェクション行列更新
    ViewAndProjectionUpdate();
}
