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
    , m_State(State::Idle)
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
    , m_ParryStartPos({0.0f,0.0f,0.0f})
    , m_ParryElapsed(0.0f)
    , m_ParryDuration(1.0f)
{
    AttachMesh(MeshManager::GetInstance().GetSkinMesh("snowball_nomal"));

    IsVisible = false;
    IsAction = false;

    // 攻撃判定の追加（Player_DamageとPlayer_Parryに当たる）.
    auto attackCol = std::make_unique<CapsuleCollider>(m_spTransform);
    m_pAttackCollider = attackCol.get();
    m_pAttackCollider->SetMyMask(eCollisionGroup::Enemy_Attack);
    // パリィで跳ね返す挙動に対応するために Player_Parry_Suc をターゲットに追加
    m_pAttackCollider->SetTarGetTargetMask(eCollisionGroup::Player_Damage | eCollisionGroup::Player_Parry_Suc);
    m_pAttackCollider->SetPositionOffset({ 0.0f, 10.f, 0.0f });
    m_pAttackCollider->SetAttackAmount(2.0f);
    m_pAttackCollider->SetHeight(25.0f);
    m_pAttackCollider->SetRadius(8.f);
    m_pAttackCollider->SetActive(true);
    m_pAttackCollider->SetColor(Color::eColor::Red);
    m_upColliders->AddCollider(std::move(attackCol));

    CollisionDetector::GetInstance().RegisterCollider(*m_upColliders);
}

bool SnowBall::ShouldDestroy() const
{
    return m_ShouldDestroy;
}

SnowBall::~SnowBall()
{
	if (m_upColliders)
	{
		CollisionDetector::GetInstance().UnregisterCollider(*m_upColliders);
	}
}

// Update メソッドの差し替え（アニメ終了でフェード開始）
void SnowBall::Update()
{
    MeshObject::Update();
	using namespace DirectX;

	float deltaTime = GetDelta();

    m_upColliders->SetDebugInfo();

	// パリィされた場合のアニメーション処理.
    // State machine handling
    if (m_State == State::HitFalling)
    {
        // 落下させる
        DirectX::XMFLOAT3 pos = m_spTransform->Position;
        pos.y -= m_FallSpeed * deltaTime;
        if (pos.y <= m_GroundY)
        {
            // 地面に到達
            pos.y = m_GroundY;
            m_spTransform->SetPosition(pos);
            m_State = State::LandedFading;
            m_IsFalling = false;
            m_IsLanded = true;
            m_FadeTimer = 0.0f;
            return;
        }
        m_spTransform->SetPosition(pos);
        return;
    }

    if (m_State == State::LandedFading)
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
    if (m_State == State::Parried)
    {
        using namespace DirectX;
        // 経過時間を進めて、パリィ開始位置から Boss_Pos まで線形補間する
        m_ParryElapsed += deltaTime;
        float s = m_ParryDuration > 0.0f ? (m_ParryElapsed / m_ParryDuration) : 1.0f;
        if (s > 1.0f) s = 1.0f;

        XMVECTOR startV = XMLoadFloat3(&m_ParryStartPos);
        XMVECTOR bossV = XMLoadFloat3(&Boss_Pos);
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
	XMVECTOR P0 = XMLoadFloat3(&Boss_Pos);
	XMVECTOR P1 = XMLoadFloat3(&Current_Pos);
	XMVECTOR P2 = XMLoadFloat3(&Player_Pos);

	// 二次ベジェ曲線の公式： B(t) = (1-t)^2*P0 + 2(1-t)t*P1 + t^2*P2
	XMVECTOR A = XMVectorLerp(P0, P1, t);
	XMVECTOR B = XMVectorLerp(P1, P2, t);
	XMVECTOR NewPosVec = XMVectorLerp(A, B, t);

	// 3. 座標の適用
    DirectX::XMFLOAT3 NewPosF = {};
	XMStoreFloat3(&NewPosF, NewPosVec);

    if (NewPosF.y <= m_GroundY)
    {
        m_State = State::HitFalling;
        // 地面に到達
        NewPosF.y = m_GroundY;
    }

	m_spTransform->SetPosition(NewPosF);

	// 4. 着弾判定
    if (t >= 1.0f)
    {
        m_State = State::LandedFading;
        m_HasBrokenVisual = false;
    }
    HandleCollision();
}

void SnowBall::Draw()
{
	if (IsVisible)
	{
		MeshObject::Draw();
	}
}

void SnowBall::Fire(const DirectX::XMFLOAT3 PlayerPos, const DirectX::XMFLOAT3 BossPos)
{
	Player_Pos = PlayerPos;
	Boss_Pos = BossPos;
    Launch();
}

void SnowBall::Launch()
{
	using namespace DirectX;
	const float ArcHeight = 10.0f; // 放物線の高さ

	XMVECTOR P0 = XMLoadFloat3(&Boss_Pos);
	XMVECTOR P2 = XMLoadFloat3(&Player_Pos);

	// 中間地点を計算して高さを出す
	XMVECTOR MidPoint = XMVectorLerp(P0, P2, 0.5f);
	XMVECTOR HeightOffset = XMVectorSet(0.0f, ArcHeight, 0.0f, 0.0f);

	XMVECTOR P1_Vec = XMVectorAdd(MidPoint, HeightOffset);
	XMStoreFloat3(&Current_Pos, P1_Vec);

	m_spTransform->SetPosition(Boss_Pos);
	m_spTransform->SetScale({ 0.3f, 0.3f, 0.3f });
	ThrowingTime = 0.0f;
	IsAction = true;
	IsVisible = true;
	m_IsParried = false;
	m_ParriedAnimTime = 0.0f;
    SetAlpha(1.0f);
	
	// 当たり判定を有効化.
	if (m_pAttackCollider) m_pAttackCollider->SetActive(true);
}

// 雪玉がヒットしたときの見た目変更とアニメーション開始
void SnowBall::HandleHitVisual()
{
    // ヒット時にスキンメッシュを差し替えてアニメーションを再生する
    auto skin = MeshManager::GetInstance().GetSkinMesh("snowball");
    if (skin)
    {
        SoundManager::GetInstance().Play("BreakSnow",false);
        SoundManager::GetInstance().SetVolume("BreakSnow",9000);

        AttachMesh(skin);
        // 再生するアニメを先頭に切り替え
        ChangeAnim(0);
        // アニメを最初から再生し、ループしない
        SetIsLoop(false);
        SetAnimTime(0.0);
        SetAnimSpeed(1.0);
        m_IsHitAnimPlaying = true;
        // 当たり後は判定を無効化して落下などを防ぐ
        if (m_pAttackCollider) m_pAttackCollider->SetActive(false);
        // 落下開始（当たった場合はまず割れ表示して落下）
        if (m_State != State::LandedFading)
        {
            m_State = State::HitFalling;
        }
        m_IsFalling = true;
        m_IsHitAnimPlaying = true;
    }
}

void SnowBall::HandleCollision()
{
	if (!m_upColliders || !m_pAttackCollider) return;

	for (const CollisionInfo& info : m_pAttackCollider->GetCollisionEvents())
	{
		if (!info.IsHit) continue;
		
		const ColliderBase* otherCollider = info.ColliderB;
		if (!otherCollider) continue;

		eCollisionGroup other_group = otherCollider->GetMyMask();

		// パリィされた場合.
		if ((other_group & eCollisionGroup::Player_Parry_Suc) != eCollisionGroup::None)
		{
			// パリィ成功: 跳ね返るように振る舞う
			m_IsParried = true;
			m_State = State::Parried;
			// パリィ開始位置を保存して補間タイマーをリセット
			m_ParryStartPos = m_spTransform->Position;
			m_ParryElapsed = 0.0f;
			m_ParryDuration = 1.0f; // 1秒で到達する
			// 当たり判定は跳ね返し中不要なので無効化
			if (m_pAttackCollider) m_pAttackCollider->SetActive(false);
            // CombatCoordinator に雪玉がパリィされたことを通知
            CombatCoordinator::GetInstance().NotifyParriedBySnowball();
			return;
		}

		// プレイヤーにダメージを与えた場合.
		if ((other_group & eCollisionGroup::Player_Damage) != eCollisionGroup::None)
		{
			// プレイヤーに当たったら割れて落下するフロー
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

