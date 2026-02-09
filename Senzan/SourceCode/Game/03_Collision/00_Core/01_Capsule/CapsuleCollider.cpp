#include "System/Singleton/Debug/CollisionVisualizer/CollisionVisualizer.h"

#include "CapsuleCollider.h"
#include "Game/03_Collision/00_Core/00_Box/BoxCollider.h"
#include "Game/03_Collision/00_Core/02_Sphere/SphereCollider.h"

namespace {
// 分割数.
static constexpr int DIVIDE_X = 16;
static constexpr int DIVIDE_Y = 16;
}

CapsuleCollider::CapsuleCollider()
	: ColliderBase	()
	, m_Radius		( 0.5f )
	, m_Height		( 2.0f )
{
}

CapsuleCollider::CapsuleCollider(std::weak_ptr<const Transform> parentTransform)
	: ColliderBase	(parentTransform)
	, m_Radius		( 0.5f )
	, m_Height		( 2.0f )
{
}

CapsuleCollider::~CapsuleCollider()
{
}

void CapsuleCollider::Update()
{

}

CollisionInfo CapsuleCollider::CheckCollision(const ColliderBase& other) const
{
    // フィルター判断.
    if (!ShouldCollide(other)) {
        return {}; 
    }

    // 形状ごとの衝突判定へディスパッチ.
    CollisionInfo info = other.DispatchCollision(*this);

    if (info.IsHit)
    {
        // 法線の反転.
        DirectX::XMVECTOR v_normal = DirectX::XMLoadFloat3(&info.Normal);
        v_normal = DirectX::XMVectorNegate(v_normal);
        DirectX::XMStoreFloat3(&info.Normal, v_normal);

        // ポインタの入れ替え.
        const ColliderBase* temp_collider = info.ColliderA;
        info.ColliderA = info.ColliderB;
        info.ColliderB = temp_collider;
    }

    return info;
}

CollisionInfo CapsuleCollider::DispatchCollision(const SphereCollider& other) const
{
    CollisionInfo info = {};

    // 自分のカプセルの線分 P1, P2 の終点を計算.
    const DirectX::XMVECTOR p1 = GetCulcCapsuleSegmentStart(this);
    const DirectX::XMVECTOR p2 = GetCulcCapsuleSegmentEnd(this);
    // 相手 (球体) の中心座標 Q.
	const DirectX::XMFLOAT3 other_position = other.GetPosition();
    const DirectX::XMVECTOR q = DirectX::XMLoadFloat3(&other_position);

    DirectX::XMVECTOR closest_p; // 線分上の最短点P (カプセル側の接触点).

    // --- 最短点Pの計算（ClosestPtPointSegmentSqロジックから流用）---
    DirectX::XMVECTOR ab = DirectX::XMVectorSubtract(p2, p1);
    DirectX::XMVECTOR ap = DirectX::XMVectorSubtract(q, p1);

    // ap を ab に投影し、スカラー値 e (Dot(ap, ab)) を求める.
    float e = DirectX::XMVectorGetX(DirectX::XMVector3Dot(ap, ab));
    float f = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(ab)); // 線分 AB の長さの二乗.

    if (e <= 0.0f) {
        // Aの外側 (P1が最短点)
        closest_p = p1;
    }
    else if (e >= f) {
        // Bの外側 (P2が最短点)
        closest_p = p2;
    }
    else {
        // 線分上 (t = e/f)
        float t = e / f;
        closest_p = DirectX::XMVectorAdd(p1, DirectX::XMVectorScale(ab, t));
    }

    // 最短ベクトル V = Q - P (球体の中心から線分上の最短点へ向かうベクトル)
    DirectX::XMVECTOR v_shortest = DirectX::XMVectorSubtract(q, closest_p);

    // 距離の二乗
    float distSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(v_shortest));

    // 許容距離（半径の合計）
    const float requiredDistance = GetRadius() + other.GetRadius();
    const float requiredDistanceSq = requiredDistance * requiredDistance;

    if (distSq <= requiredDistanceSq)
    {
        info.IsHit = true;

        // 距離 (Vの長さ).
        float distance = sqrtf(distSq);

        // 衝突法線 N = Normalize(V) .
        DirectX::XMVECTOR v_normal = DirectX::XMVector3Normalize(v_shortest);

        // めり込み深さ D = (R_capsule + R_sphere) - |V|.
        info.PenetrationDepth = requiredDistance - distance;

        // 接触点: カプセル表面上の点 (P + N * R_capsule).
        DirectX::XMVECTOR v_contact_point = DirectX::XMVectorAdd(closest_p, DirectX::XMVectorScale(v_normal, GetRadius()));
        DirectX::XMStoreFloat3(&info.Normal, v_normal);
        DirectX::XMStoreFloat3(&info.ContactPoint, v_contact_point);

        // 衝突に関わったコライダーへのポインタ設定.
        info.ColliderA = this;
        info.ColliderB = &other;
    }

    return info;
}

CollisionInfo CapsuleCollider::DispatchCollision(const CapsuleCollider& other) const
{
    CollisionInfo info = {};

    // 1. 自分の線分 P1, P2 の終点を計算 (線分 r = p2 - p1)
    const DirectX::XMVECTOR p1 = GetCulcCapsuleSegmentStart(this);
    const DirectX::XMVECTOR p2 = GetCulcCapsuleSegmentEnd(this);

    // 2. 相手の線分 Q1, Q2 の終点を計算 (線分 s = q2 - q1)
    const DirectX::XMVECTOR q1 = GetCulcCapsuleSegmentStart(&other);
    const DirectX::XMVECTOR q2 = GetCulcCapsuleSegmentEnd(&other);

    // --- 最短点 P と Q の計算ロジック（GetCulcClosestPtSegmentSegmentSqから流用） ---

    const DirectX::XMVECTOR r = DirectX::XMVectorSubtract(p2, p1); // r = B - A
    const DirectX::XMVECTOR s = DirectX::XMVectorSubtract(q2, q1); // s = D - C
    const DirectX::XMVECTOR e = DirectX::XMVectorSubtract(p1, q1); // e = A - C

    const float a_val = DirectX::XMVectorGetX(DirectX::XMVector3Dot(r, r));
    const float e_val = DirectX::XMVectorGetX(DirectX::XMVector3Dot(r, s));
    const float f_val = DirectX::XMVectorGetX(DirectX::XMVector3Dot(s, s));
    const float g_val = DirectX::XMVectorGetX(DirectX::XMVector3Dot(r, e));
    const float h_val = DirectX::XMVectorGetX(DirectX::XMVector3Dot(s, e));

    const float EPSILON = 1e-6f;
    float s_param, t_param;
    const float k = a_val * f_val - e_val * e_val;

    if (k < EPSILON) {
        // ... (平行な場合の処理) ...
        s_param = 0.0f;
        if (f_val < EPSILON) { t_param = 0.0f; }
        else {
            t_param = h_val / f_val;
            if (t_param < 0.0f) t_param = 0.0f;
            if (t_param > 1.0f) t_param = 1.0f;
        }
    }
    else {
        float inv_k = 1.0f / k;
        s_param = (e_val * h_val - f_val * g_val) * inv_k;
        if (s_param < 0.0f) { s_param = 0.0f; }
        else if (s_param > 1.0f) { s_param = 1.0f; }
        if (f_val < EPSILON) { t_param = 0.0f; }
        else { t_param = (s_param * e_val + h_val) / f_val; }
        if (t_param < 0.0f) { t_param = 0.0f; }
        else if (t_param > 1.0f) { t_param = 1.0f; }
        if (t_param == 0.0f || t_param == 1.0f) {
            if (t_param == 0.0f) { s_param = -g_val / a_val; }
            else { s_param = (e_val - g_val) / a_val; }
            if (s_param < 0.0f) s_param = 0.0f;
            if (s_param > 1.0f) s_param = 1.0f;
        }
    }

    // 最短点 P(s) と Q(t) を計算
    const DirectX::XMVECTOR closest_p = DirectX::XMVectorAdd(p1, DirectX::XMVectorScale(r, s_param)); // 自分の線分上の最短点
    const DirectX::XMVECTOR closest_q = DirectX::XMVectorAdd(q1, DirectX::XMVectorScale(s, t_param)); // 相手の線分上の最短点

    // --- 衝突判定と情報計算 ---

    // 最短ベクトル V = Q - P (相手の最短点から自分の最短点へ向かうベクトル)
    DirectX::XMVECTOR v_shortest = DirectX::XMVectorSubtract(closest_p, closest_q);

    float distSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(v_shortest));

    // 許容距離（半径の合計）
    const float requiredDistance = GetRadius() + other.GetRadius();
    const float requiredDistanceSq = requiredDistance * requiredDistance;

    if (distSq <= requiredDistanceSq)
    {
        info.IsHit = true;

        float distance = sqrtf(distSq);

        // 法線 N = Normalize(V) (自分カプセルから相手カプセルを押し出す方向 P -> Q).
        DirectX::XMVECTOR v_normal = DirectX::XMVector3Normalize(v_shortest);

        // めり込み深さ D = (R1 + R2) - |V|.
        info.PenetrationDepth = requiredDistance - distance;

        // 接触点: ２つの最短点を結ぶ線分の中点.
        DirectX::XMVECTOR v_contact_point = DirectX::XMVectorScale(DirectX::XMVectorAdd(closest_p, closest_q), 0.5f);

        // XMFLOAT3 にストア
        DirectX::XMStoreFloat3(&info.Normal, v_normal);
        DirectX::XMStoreFloat3(&info.ContactPoint, v_contact_point);

        // 衝突に関わったコライダーへのポインタ設定.
        info.ColliderA = this;
        info.ColliderB = &other;
    }

    return info;
}

CollisionInfo CapsuleCollider::DispatchCollision(const BoxCollider& other) const
{
    CollisionInfo a = {};
    // 1. 自分のカプセルの線分 P1, P2 の終点を計算
      // const DirectX::XMVECTOR p1 = GetCulcCapsuleSegmentStart(this);
      // const DirectX::XMVECTOR p2 = GetCulcCapsuleSegmentEnd(this);

      // 2. OBB（other）と線分（P1P2）の最短距離の二乗を計算する
      // OBB-Segment の最短距離アルゴリズム（Boxのローカル空間への変換とクランプ）が必要です。

      // const float distSq = 9999.0f; // OBB-Segment::ClosestPtSegmentOBBSq(...)

      // 3. 判定: 最短距離がカプセルの半径の二乗以下なら衝突
      // const float requiredRadius = GetRadius();

      // return distSq <= (requiredRadius * requiredRadius);

      // 💡 Boxとの判定ロジックが未実装のため、一旦 false を返す
    return a;
}

// カプセルの中心線分の終点 P1 を「計算し」取得.
DirectX::XMVECTOR CapsuleCollider::GetCulcCapsuleSegmentStart(const CapsuleCollider* capsule)
{
    const float radius = capsule->GetRadius();
    const float height = capsule->GetHeight();
    // 線分長さは (height - 2 * radius)。負の値にならないようにクランプする。
    float segment_length = height - 2.0f * radius;
    if (segment_length < 0.0f) segment_length = 0.0f;
    const float half_segment_length = segment_length * 0.5f;


    if (auto spTransform = capsule->m_wpTransform.lock())
    {
        DirectX::XMMATRIX mat_parent_world = spTransform->GetWorldMatrix();

        DirectX::XMVECTOR scale, rotation, translation;
        DirectX::XMMatrixDecompose(&scale, &rotation, &translation, mat_parent_world);

        DirectX::XMMATRIX mat_no_scale = DirectX::XMMatrixRotationQuaternion(rotation);
        mat_no_scale = DirectX::XMMatrixMultiply(mat_no_scale, DirectX::XMMatrixTranslationFromVector(translation));

        // PositionOffset を親の回転で回転させてから中心に加算する
        DirectX::XMVECTOR v_offset = DirectX::XMLoadFloat3(&capsule->m_PositionOffset);
        // mat_no_scale は回転と位置を含む行列だが回転のみの行列を作成してオフセットを回転させる
        DirectX::XMMATRIX mat_rotation = DirectX::XMMatrixRotationQuaternion(rotation);
        DirectX::XMVECTOR v_rotated_offset = DirectX::XMVector3TransformNormal(v_offset, mat_rotation);
        DirectX::XMVECTOR v_center = DirectX::XMVectorAdd(translation, v_rotated_offset);

        // ローカル座標 (0, -half_segment_length, 0) を回転だけ適用してワールドに変換し、中心に加算する
        DirectX::XMVECTOR v_local_start = DirectX::XMVectorSet(0.0f, -half_segment_length, 0.0f, 0.0f);
        DirectX::XMVECTOR v_rotated_local_start = DirectX::XMVector3TransformNormal(v_local_start, mat_rotation);
        return DirectX::XMVectorAdd(v_center, v_rotated_local_start);
    }

    // 親Transformがない場合、オフセットなしのローカル座標を返す.
    const DirectX::XMFLOAT3 pos = capsule->GetPositionOffset();
    return DirectX::XMVectorSet(pos.x, pos.y - half_segment_length, pos.z, 1.0f);
}

// カプセルの中心線分の終点 P2 を「計算し」取得.
DirectX::XMVECTOR CapsuleCollider::GetCulcCapsuleSegmentEnd(const CapsuleCollider* capsule)
{
    const float radius = capsule->GetRadius();
    const float height = capsule->GetHeight();
    // 線分長さは (height - 2 * radius)。負の値にならないようにクランプする。
    float segment_length = height - 2.0f * radius;
    if (segment_length < 0.0f) segment_length = 0.0f;
    const float half_segment_length = segment_length * 0.5f;

    if (auto spTransform = capsule->m_wpTransform.lock())
    {
        DirectX::XMMATRIX mat_parent_world = spTransform->GetWorldMatrix();

        // 親のワールド行列からスケールを取り除き、回転と位置のみを取得.
        DirectX::XMVECTOR scale, rotation, translation;
        DirectX::XMMatrixDecompose(&scale, &rotation, &translation, mat_parent_world);

        DirectX::XMMATRIX mat_no_scale = DirectX::XMMatrixRotationQuaternion(rotation);
        mat_no_scale = DirectX::XMMatrixMultiply(mat_no_scale, DirectX::XMMatrixTranslationFromVector(translation));

        // PositionOffset を親の回転で回転させてから中心に加算する
        DirectX::XMVECTOR v_offset = DirectX::XMLoadFloat3(&capsule->m_PositionOffset);
        DirectX::XMMATRIX mat_rotation = DirectX::XMMatrixRotationQuaternion(rotation);
        DirectX::XMVECTOR v_rotated_offset = DirectX::XMVector3TransformNormal(v_offset, mat_rotation);
        DirectX::XMVECTOR v_center = DirectX::XMVectorAdd(translation, v_rotated_offset);

        // ローカル座標 (0, +half_segment_length, 0) を回転だけ適用してワールドに変換し、中心に加算する
        DirectX::XMVECTOR v_local_end = DirectX::XMVectorSet(0.0f, half_segment_length, 0.0f, 0.0f);
        DirectX::XMVECTOR v_rotated_local_end = DirectX::XMVector3TransformNormal(v_local_end, mat_rotation);
        return DirectX::XMVectorAdd(v_center, v_rotated_local_end);
    }
    // 親Transformがない場合、オフセットなしのローカル座標を返す.
    const DirectX::XMFLOAT3 pos = capsule->GetPositionOffset();
    return DirectX::XMVectorSet(pos.x, pos.y + half_segment_length, pos.z, 1.0f);
}

// 点 P から線分 AB への最短距離の二乗を「計算し」取得.
float CapsuleCollider::GetCulcClosestPtPointSegmentSq(DirectX::XMVECTOR p, DirectX::XMVECTOR a, DirectX::XMVECTOR b)
{
    DirectX::XMVECTOR ab = DirectX::XMVectorSubtract(b, a); // 線分 AB のベクトル
    DirectX::XMVECTOR ap = DirectX::XMVectorSubtract(p, a); // 点 A から点 P へのベクトル

    // ap を ab に投影し、スカラー値 e (Dot(ap, ab)) を求める.
    float e = DirectX::XMVectorGetX(DirectX::XMVector3Dot(ap, ab));

    // 1. e <= 0.0f の場合: 点 A の外側 (A が最短).
    if (e <= 0.0f) {
        return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(ap));
    }

    float f = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(ab)); // 線分 AB の長さの二乗.

    // 2. e >= f の場合: 点 B の外側 (B が最短).
    if (e >= f) {
        DirectX::XMVECTOR bp = DirectX::XMVectorSubtract(p, b);
        return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(bp));
    }

    // 3. 0 < e < f の場合: 線分上に最短点がある.
    // 最短距離の二乗 = |ap|^2 - (e^2 / f).
    return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(ap)) - e * e / f;
}

//  線分 AB と線分 CD の最短距離の二乗を「計算し」取得.
float CapsuleCollider::GetCulcClosestPtSegmentSegmentSq(
    DirectX::XMVECTOR a,
    DirectX::XMVECTOR b,
    DirectX::XMVECTOR c,
    DirectX::XMVECTOR d)
{
    // r = B - A (線分 AB の方向ベクトル).
    const DirectX::XMVECTOR r = DirectX::XMVectorSubtract(b, a);
    // s = D - C (線分 CD の方向ベクトル).
    const DirectX::XMVECTOR s = DirectX::XMVectorSubtract(d, c);
    // e = A - C (線分 AC のベクトル).
    const DirectX::XMVECTOR e = DirectX::XMVectorSubtract(a, c);

    // 各種のドット積を計算.
    const float a_val = DirectX::XMVectorGetX(DirectX::XMVector3Dot(r, r)); // |r|^2 (a = r . r).
    const float e_val = DirectX::XMVectorGetX(DirectX::XMVector3Dot(r, s)); // r . s.
    const float f_val = DirectX::XMVectorGetX(DirectX::XMVector3Dot(s, s)); // |s|^2 (f = s . s).

    const float g_val = DirectX::XMVectorGetX(DirectX::XMVector3Dot(r, e)); // r . e.
    const float h_val = DirectX::XMVectorGetX(DirectX::XMVector3Dot(s, e)); // s . e.

    // 許容誤差（ゼロ除算防止）.
    const float EPSILON = 1e-6f;

    float s_param, t_param;

    // k = a*f - e^2 (2x2行列の行列式).
    const float k = a_val * f_val - e_val * e_val;

    if (k < EPSILON) {
        // 線分 AB と CD がほぼ平行な場合 (k ≈ 0).
        // s=0 (点A)に固定して t を解く.
        s_param = 0.0f;

        // t を [0, 1] にクランプ
        if (f_val < EPSILON) {
            t_param = 0.0f; // 線分 CD が点に近い場合.
        }
        else {
            t_param = h_val / f_val;

            // クランプ処理 (std::min/maxを使わずif文で処理).
            if (t_param < 0.0f) t_param = 0.0f;
            if (t_param > 1.0f) t_param = 1.0f;
        }

    }
    else {
        // 線分が平行でない一般のケース.
        float inv_k = 1.0f / k;
        s_param = (e_val * h_val - f_val * g_val) * inv_k;

        // 1. s の値を [0, 1] の範囲にクランプ.
        if (s_param < 0.0f) {
            s_param = 0.0f;
        }
        else if (s_param > 1.0f) {
            s_param = 1.0f;
        }

        // 2. s のクランプ値を使って t を再計算し、t をクランプ.
        if (f_val < EPSILON) {
            t_param = 0.0f;
        }
        else {
            t_param = (s_param * e_val + h_val) / f_val;
        }

        if (t_param < 0.0f) {
            t_param = 0.0f;
        }
        else if (t_param > 1.0f) {
            t_param = 1.0f;
        }

        // 3. t のクランプ値を使って s を再計算し、s をクランプ (最短点チェック).
        if (t_param == 0.0f || t_param == 1.0f) {
            if (t_param == 0.0f) {
                // t = 0 (点C)からの最短点.
                s_param = -g_val / a_val;
            }
            else { // t = 1.0f (点D)からの最短点.
                s_param = (e_val - g_val) / a_val;
            }

            // s を最終的に [0, 1] にクランプ.
            if (s_param < 0.0f) s_param = 0.0f;
            if (s_param > 1.0f) s_param = 1.0f;
        }
    }

    // 最短点 P(s) と Q(t) を計算.
    const DirectX::XMVECTOR closest_p = DirectX::XMVectorAdd(a, DirectX::XMVectorScale(r, s_param));
    const DirectX::XMVECTOR closest_q = DirectX::XMVectorAdd(c, DirectX::XMVectorScale(s, t_param));

    // 距離の二乗を計算し、返す.
    const DirectX::XMVECTOR pq = DirectX::XMVectorSubtract(closest_p, closest_q);
    return DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(pq));
}

// デバッグ描画用設定.
void CapsuleCollider::SetDebugInfo()
{
    CollisionVisualizer& visualizer = CollisionVisualizer::GetInstance();
    DebugColliderInfo info = {};

    std::shared_ptr<const Transform> spParentTransform = m_wpTransform.lock();

    // 親Transform から回転と位置を取得し、スケールを除いた行列を作成
    DirectX::XMMATRIX mat_parent_world = spParentTransform
        ? spParentTransform->GetWorldMatrix()
        : DirectX::XMMatrixIdentity();

    DirectX::XMVECTOR v_scale, v_rotation, v_translation;
    DirectX::XMMatrixDecompose(&v_scale, &v_rotation, &v_translation, mat_parent_world);

    DirectX::XMMATRIX mat_parent_no_scale = DirectX::XMMatrixAffineTransformation(
        DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f),
        DirectX::XMVectorZero(),
        v_rotation,
        v_translation
    );

    // オフセット行列.
    DirectX::XMMATRIX mat_offset = DirectX::XMMatrixTranslation(
        m_PositionOffset.x, m_PositionOffset.y, m_PositionOffset.z
    );

    // 親のワールド行列にオフセットを乗算してデバッグ用ワールド行列を作成.
    DirectX::XMMATRIX mat_debug_world = DirectX::XMMatrixMultiply(mat_offset, mat_parent_no_scale);

    info.ShapeType = eShapeType::Capsule;
    info.Color = m_Color; 

    DirectX::XMStoreFloat4x4(&info.WorldMatrix, mat_debug_world);

    // 半径を Data0.x に、高さを Data0.y に設定.
    info.Data0 = DirectX::XMFLOAT4(m_Radius, m_Height, 0.0f, 0.0f);
    info.Data1 = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

    visualizer.RegisterCollider(info);
}
