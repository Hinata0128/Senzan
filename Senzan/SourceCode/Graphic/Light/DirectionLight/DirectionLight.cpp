#include "DirectionLight.h"

DirectionLight::DirectionLight()
    : m_Position    ( Axis::ZERO )
    , m_Direction   ( Axis::ZERO )
    , m_View        ()
    , m_Projection  ()
{
}

//--------------------------------------------------------------------------.

DirectionLight::~DirectionLight()
{
}

//--------------------------------------------------------------------------.

void DirectionLight::UpdateViewAndProjection()
{
    View();
	projection();
}

//--------------------------------------------------------------------------.

void DirectionLight::View()
{
    // 1. 必要なXMFLOAT3データをXMVECTORにロード
    DirectX::XMVECTOR vDirectionXM = DirectX::XMLoadFloat3(&m_Direction);
    DirectX::XMVECTOR vPositionXM = DirectX::XMLoadFloat3(&m_Position);

    // 上方ベクトルは固定値でXMVECTORを生成
    DirectX::XMVECTOR vUpVecXM = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    // 2. ディレクショナルライトの方向ベクトルを正規化 (D3DXVec3Normalize -> XMVector3Normalize)
    DirectX::XMVECTOR vDirectionNormalizedXM = DirectX::XMVector3Normalize(vDirectionXM);

    // 3. 視点（LookAt）の位置を計算: Position + Direction (XMVECTORで加算)
    DirectX::XMVECTOR vLookAtXM = DirectX::XMVectorAdd(vPositionXM, vDirectionNormalizedXM);

    // 4. ビュー行列を計算 (D3DXMatrixLookAtLH -> XMMatrixLookAtLH)
    // m_View は XMMATRIX 型を想定
    m_View = DirectX::XMMatrixLookAtLH(
        vPositionXM,          // Eye position (視点)
        vLookAtXM,            // Focus position (注視点)
        vUpVecXM              // Up direction (上方ベクトル)
    );
}

// --------------------------------------------------------------------------.

void DirectionLight::projection()
{
    float near_z = 0.1f;
    float far_z = 10000.0f;

    // ライトの射影行列の幅と高さを定義
    // WND_W * 5 と WND_H * 5 は、ライトのシャドウカスケードをカバーする直交ビューボリュームのサイズ
    float width = WND_W * 5;
    float height = WND_H * 5;

    // 射影行列を計算 (D3DXMatrixOrthoLH -> XMMatrixOrthographicLH)
    // m_Projection は XMMATRIX 型を想定
    m_Projection = DirectX::XMMatrixOrthographicLH(
        width,
        height,
        near_z,
        far_z
    );
}

//--------------------------------------------------------------------------.

const DirectX::XMFLOAT3& DirectionLight::GetPosition() const
{
    return m_Position;
}

//--------------------------------------------------------------------------.

void DirectionLight::SetPosition(const DirectX::XMFLOAT3& Position)
{
    m_Position = Position;
}

//--------------------------------------------------------------------------.

const DirectX::XMFLOAT3& DirectionLight::GetDirection() const
{
    return m_Direction;
}

//--------------------------------------------------------------------------.

void DirectionLight::SetDirection(const DirectX::XMFLOAT3& direction)
{
    m_Direction = direction;
}

//--------------------------------------------------------------------------.
    
const DirectX::XMMATRIX DirectionLight::GetViewMatrix() const
{
    return m_View;
}

//--------------------------------------------------------------------------.

const DirectX::XMMATRIX DirectionLight::GetProjectionMatrix() const
{
    return m_Projection;
}
