#include "SnowBall.h"
#include "Game/04_Time/Time.h"
#include "Game/03_Collision/00_Core/01_Capsule/CapsuleCollider.h"
#include "Game/03_Collision/00_Core/ColliderBase.h"
#include "System/Singleton/CollisionDetector/CollisionDetector.h"
#include "System/Singleton/CombatCoordinator/CombatCoordinator.h"
#include "Resource/Mesh/02_Skin/SkinMesh.h"
#include <algorithm>

SnowBall::SnowBall()
    : MeshObject()
    , ThrowingTime(0.0f)
    , Totle_ThrowingTime(1.5f)
    , m_upColliders(std::make_unique<CompositeCollider>())
    , m_pAttackCollider(nullptr)
    , m_ParriedAnimTime(0.0f)
    , m_State(enSnowBall::Idol)
    , m_IsParried(false)
    , m_IsHitAnimPlaying(false)
    , m_IsFalling(false)
    , m_FallSpeed(50.0f)
    , m_IsLanded(false)
    , m_FadeDuration(2.0f)
    , m_FadeTimer(0.0f)
    , m_GroundY(1.f)
    , m_ShouldDestroy(false)
    , m_HasBrokenVisual(false)
    , m_BounceSpeed(60.0f)
    , m_ParryStartPos({ 0.0f,0.0f,0.0f })
    , m_ParryElapsed(0.0f)
    , m_ParryDuration(1.0f)
{
    //雪玉のメッシュをアタッチ.
    AttachMesh(MeshManager::GetInstance().GetSkinMesh("snowball_nomal"));

    IsVisible = false;
    IsAction = false;

    //攻撃判定の追加(Player_DamageとPlayer_Parryに判定をつける).
    auto AttackCol = std::make_unique<CapsuleCollider>(m_spTransform);
    m_pAttackCollider = AttackCol.get();
    //敵の攻撃を設定.
    m_pAttackCollider->SetMyMask(eCollisionGroup::Enemy_Attack);
    //プレイヤーのパリィで跳ね返す挙動に対応させるためにPlayer_Parry_Sucをターゲットに追加.
    m_pAttackCollider->SetTarGetTargetMask(eCollisionGroup::Player_Damage | eCollisionGroup::Player_Parry_Suc);
    m_pAttackCollider->SetPositionOffset({ 0.0f, 10.0f, 0.0f });
    m_pAttackCollider->SetAttackAmount(2.0f);
    m_pAttackCollider->SetHeight(25.0f);
    m_pAttackCollider->SetRadius(8.0f);
    //攻撃判定を一旦なしに設定する.
    m_pAttackCollider->SetActive(true);
    //当たり判定の可視化の時に赤色の当たり判定を表示する.
    m_pAttackCollider->SetColor(Color::eColor::Red);
    m_upColliders->AddCollider(std::move(AttackCol));
    CollisionDetector::GetInstance().RegisterCollider(*m_upColliders);

}

SnowBall::~SnowBall()
{
    if (m_upColliders)
    {
        CollisionDetector::GetInstance().UnregisterCollider(*m_upColliders);
    }
}

void SnowBall::Update()
{
#if 1
    MeshObject::Update();
    //デルタタイムを取得する.
    float deltaTime = GetDelta();

    m_upColliders->SetDebugInfo();

    //パリィされた場合のアニメーションの処理.
    if (m_State == enSnowBall::HitFalling)
    {
        //落下させる.
        //位置を取得する.
        DirectX::XMFLOAT3 Pos = m_spTransform->Position;
        //投擲の速度を設定.
        Pos.y -= m_FallSpeed * deltaTime;
        if (Pos.y <= m_GroundY)
        {
            //地面に到着
            Pos.y = m_GroundY;
            m_spTransform->SetPosition(Pos);
            m_State = enSnowBall::LandedFading;
            m_IsFalling = false;
            m_IsLanded = true;
            m_FadeTimer = 0.0f;
            return;
        }
        m_spTransform->SetPosition(Pos);
        return;
    }

    if (m_State == enSnowBall::LandedFading)
    {
        if (!m_HasBrokenVisual)
        {
            HandleHitVisual();
            m_HasBrokenVisual = true;
        }
        if (!IsAnimEnd(0))
        {
            return;
        }

        m_FadeTimer += deltaTime;
        float Alpha = 1.0f - std::clamp(m_FadeTimer / m_FadeDuration, 0.0f, 1.0f);
        //SkinMeshのグローバルアルファを設定.
        if (auto skin = std::dynamic_pointer_cast<SkinMesh>(m_pMesh.lock()))
        {
            SetAlpha(Alpha);
        }

        if (m_FadeTimer >= m_FadeDuration)
        {
            //フェード完了で自己消滅フラグを立てる.
            //Managerが削除する.
            m_ShouldDestroy = true;
        }
        return;
    }

    //跳ね返り状態 : Bossに到達したら割れて消える.
    if (m_State == enSnowBall::Parried)
    {
        m_ParryElapsed += deltaTime;
        float S = m_ParryDuration > 0.0f ? std::clamp(m_ParryElapsed / m_ParryDuration, 0.0f, 1.0f) : 1.0f;
        if (S > 1.0f)
        {
            S = 1.0f;
        }

        DirectX::XMVECTOR StartV = DirectX::XMLoadFloat3(&m_ParryStartPos);
        DirectX::XMVECTOR BossV = DirectX::XMLoadFloat3(&Boss_Pos_F);
        DirectX::XMVECTOR NewPosV = DirectX::XMVectorLerp(StartV, BossV, S);
        DirectX::XMFLOAT3 NewPos; DirectX::XMStoreFloat3(&NewPos, NewPosV);
        m_spTransform->SetPosition(NewPos);

        if (S >= 1.0f)
        {
            // 到達したら割れ表示にする
            HandleHitVisual();
        }
        return;
    }
    //時間の更新.
    ThrowingTime += deltaTime;
    float T = ThrowingTime / Totle_ThrowingTime;
    if (T >= 1.0f)
    {
        T = 1.0f;
    }
    //2次ベジェ曲線計算.
    DirectX::XMVECTOR P0 = DirectX::XMLoadFloat3(&Boss_Pos_F);
    DirectX::XMVECTOR P1 = DirectX::XMLoadFloat3(&Current_Pos_F);
    DirectX::XMVECTOR P2 = DirectX::XMLoadFloat3(&Player_Pos_F);
    //二次ベジェ曲線の公式: B(T) = (1-T) ^ 2 * P0 + 2 * (1-T) * T * P1 + T^2 * P2
    DirectX::XMVECTOR A = DirectX::XMVectorLerp(P0, P1, T);
    DirectX::XMVECTOR B = DirectX::XMVectorLerp(P1, P2, T);
    DirectX::XMVECTOR NewPosV = DirectX::XMVectorLerp(A, B, T);

    //座標の適用.
    DirectX::XMFLOAT3 NewPos_F = {};
    DirectX::XMStoreFloat3(&NewPos_F, NewPosV);

    if (NewPos_F.y <= m_GroundY)
    {
        m_State = enSnowBall::HitFalling;
        //地面に到着.
        NewPos_F.y = m_GroundY;
    }
    m_spTransform->SetPosition(NewPos_F);
    //着弾判定.
    if (T >= 1.0f)
    {
        m_State = enSnowBall::LandedFading;
        m_HasBrokenVisual = false;
    }
    HandleCollision();
#else
    MeshObject::Update();
    using namespace DirectX;

    float deltaTime = GetDelta();

    m_upColliders->SetDebugInfo();

    // パリィされた場合のアニメーション処理.
    // State machine handling
    if (m_State == enSnowBall::HitFalling)
    {
        // 落下させる
        DirectX::XMFLOAT3 pos = m_spTransform->Position;
        pos.y -= m_FallSpeed * deltaTime;
        if (pos.y <= m_GroundY)
        {
            // 地面に到達
            pos.y = m_GroundY;
            m_spTransform->SetPosition(pos);
            m_State = enSnowBall::LandedFading;
            m_IsFalling = false;
            m_IsLanded = true;
            m_FadeTimer = 0.0f;
            return;
        }
        m_spTransform->SetPosition(pos);
        return;
    }

    if (m_State == enSnowBall::LandedFading)
    {
        if (!m_HasBrokenVisual)
        {
            HandleHitVisual();
            m_HasBrokenVisual = true;
        }

        if (!IsAnimEnd(0))
        {
            return;
        }

        m_FadeTimer += deltaTime;
        float alpha = 1.0f - std::clamp(m_FadeTimer / m_FadeDuration, 0.0f, 1.0f);
        // SkinMesh のグローバルアルファを設定
        if (auto skin = std::dynamic_pointer_cast<SkinMesh>(m_pMesh.lock()))
        {
            SetAlpha(alpha);
        }

        if (m_FadeTimer >= m_FadeDuration)
        {
            // フェード完了で自己消滅フラグを立てる（Manager が削除）
            m_ShouldDestroy = true;
        }
        return;
    }

    // 跳ね返り状態: Boss に到達したら割れて消える
    if (m_State == enSnowBall::Parried)
    {
        using namespace DirectX;
        // 経過時間を進めて、パリィ開始位置から Boss_Pos まで線形補間する
        m_ParryElapsed += deltaTime;
        float s = m_ParryDuration > 0.0f ? (m_ParryElapsed / m_ParryDuration) : 1.0f;
        if (s > 1.0f) s = 1.0f;

        XMVECTOR startV = XMLoadFloat3(&m_ParryStartPos);
        XMVECTOR bossV = XMLoadFloat3(&Boss_Pos_F);
        XMVECTOR newPosV = XMVectorLerp(startV, bossV, s);
        XMFLOAT3 newPos; XMStoreFloat3(&newPos, newPosV);
        m_spTransform->SetPosition(newPos);

        if (s >= 1.0f)
        {
            // 到達したら割れ表示にする
            HandleHitVisual();
        }
        return;
    }

    // 1. 時間の更新（deltaTimeに余計な倍率をかけない）
    ThrowingTime += deltaTime;

    // t (0.0 ～ 1.0) を計算
    float t = ThrowingTime / Totle_ThrowingTime;
    if (t >= 1.0f) t = 1.0f;

    // 2. 二次ベジェ曲線計算
    XMVECTOR P0 = XMLoadFloat3(&Boss_Pos_F);
    XMVECTOR P1 = XMLoadFloat3(&Current_Pos_F);
    XMVECTOR P2 = XMLoadFloat3(&Player_Pos_F);

    // 二次ベジェ曲線の公式： B(t) = (1-t)^2*P0 + 2(1-t)t*P1 + t^2*P2
    XMVECTOR A = XMVectorLerp(P0, P1, t);
    XMVECTOR B = XMVectorLerp(P1, P2, t);
    XMVECTOR NewPosVec = XMVectorLerp(A, B, t);

    // 3. 座標の適用
    DirectX::XMFLOAT3 NewPosF = {};
    XMStoreFloat3(&NewPosF, NewPosVec);

    if (NewPosF.y <= m_GroundY)
    {
        m_State = enSnowBall::HitFalling;
        // 地面に到達
        NewPosF.y = m_GroundY;
    }

    m_spTransform->SetPosition(NewPosF);

    // 4. 着弾判定
    if (t >= 1.0f)
    {
        m_State = enSnowBall::LandedFading;
        m_HasBrokenVisual = false;
    }
    HandleCollision();

#endif
}

void SnowBall::Draw()
{
    if (IsVisible)
    {
        MeshObject::Draw();
    }
}

void SnowBall::Fire(
    const DirectX::XMFLOAT3 PlayerPos,
    const DirectX::XMFLOAT3 BossPos)
{
    Player_Pos_F = PlayerPos;
    Boss_Pos_F = BossPos;
    Launch();
}

bool SnowBall::Destroy() const
{
    return m_ShouldDestroy;
}

void SnowBall::Launch()
{
    //放物線の高さ.
    const float ArcHeight = 30.0f;
    DirectX::XMVECTOR P0 = DirectX::XMLoadFloat3(&Boss_Pos_F);
    DirectX::XMVECTOR P2 = DirectX::XMLoadFloat3(&Player_Pos_F);
    //中間地点を計算して高さを出す.
    DirectX::XMVECTOR MidPoint = DirectX::XMVectorLerp(P0, P2, 0.5f);
    DirectX::XMVECTOR HeightOffset = DirectX::XMVectorSet(0.0f, ArcHeight, 0.0f, 0.0f);

    DirectX::XMVECTOR P1_Vec = DirectX::XMVectorAdd(MidPoint, HeightOffset);
    DirectX::XMStoreFloat3(&Current_Pos_F, P1_Vec);

    m_spTransform->SetPosition(Boss_Pos_F);
    m_spTransform->SetScale({ 0.3f, 0.3f,0.3 });
    ThrowingTime = 0.0f;
    IsAction = true;
    IsVisible = true;
    m_IsParried = false;
    m_ParriedAnimTime = 0.0f;
    SetAlpha(1.0f);
    //当たり判定有効化.
    if (m_pAttackCollider)
    {
        m_pAttackCollider->SetActive(true);
    }
}

void SnowBall::HandleCollision()
{
    if (!m_upColliders || !m_pAttackCollider)
    {
        return;
    }
    for (const CollisionInfo& Info : m_pAttackCollider->GetCollisionEvents())
    {
        if (!Info.IsHit)
        {
            continue;
        }
        const ColliderBase* OtherCollider = Info.ColliderB;
        if (!OtherCollider)
        {
            continue;
        }

        eCollisionGroup Other_Group = OtherCollider->GetMyMask();
        //パリィされた場合.
        if ((Other_Group & eCollisionGroup::Player_Parry_Suc) != eCollisionGroup::None)
        {
            //パリィ成功: 跳ね返るように振る舞う.
            m_IsParried = true;
            m_State = enSnowBall::Parried;
            //パリィ開始位置を保存して補足タイマーをリセット.
            m_ParryStartPos = m_spTransform->Position;
            m_ParryElapsed = 0.0f;
            m_ParryDuration = 1.0f; //1秒で到着.
            //当たり判定は跳ね返し中不要なので無効化.
            if (m_pAttackCollider)
            {
                m_pAttackCollider->SetActive(false);
            }
            CombatCoordinator::GetInstance().NotifyParriedBySnowball();
            return;
        }

        //プレイヤーにダメージを与えた場合.
        if ((Other_Group & eCollisionGroup::Player_Damage) != eCollisionGroup::None)
        {
            //プレイヤーに当たった場合は割れて落下するようにする.
            IsAction = false;
            HandleHitVisual();
            return;
        }
    }
}

void SnowBall::OnParried()
{
    // 旧来の単純な割れ処理は不要になった。
    // 今は HandleCollision でパリィを検出して State::Parried に遷移する。
    // この関数は互換性のために残すが、直接状態を操作しない。
    // 互換性のために OnParried でも同様の初期化を行う
    m_IsParried = true;
    m_ParriedAnimTime = 0.0f;
    IsAction = false;
    m_ParryStartPos = m_spTransform->Position;
    m_ParryElapsed = 0.0f;
    m_ParryDuration = 1.0f;

}

//雪玉がヒットしたときの見た目の変更とアニメーション開始.
void SnowBall::HandleHitVisual()
{
    //ヒット時にスキンメッシュのアニメーションを再生する.
    auto Skin = MeshManager::GetInstance().GetSkinMesh("snowball");
    if (Skin)
    {
        SoundManager::GetInstance().Play("BreakSnow", false);
        SoundManager::GetInstance().SetVolume("BreakSnow", 9000);
        AttachMesh(Skin);
        // 再生するアニメを先頭に切り替え
        ChangeAnim(0);
        // アニメを最初から再生し、ループしない
        SetIsLoop(false);
        SetAnimTime(0.0);
        SetAnimSpeed(1.0);
        m_IsHitAnimPlaying = true;
        //当たり判定を無効化して落下などを防ぐ.
        if (m_pAttackCollider)
        {
            m_pAttackCollider->SetActive(false);
        }
        //落下開始(当たった場合は、まず割れを表示して落下).
        if (m_State != enSnowBall::LandedFading)
        {
            m_State = enSnowBall::HitFalling;
        }
        m_IsFalling = true;
        m_IsHitAnimPlaying = true;
    }
}
