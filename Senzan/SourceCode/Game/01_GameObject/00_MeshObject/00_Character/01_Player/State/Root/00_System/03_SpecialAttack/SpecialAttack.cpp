#include "SpecialAttack.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/State/Root/01_Action/02_Dodge/01_JustDodge/AfterImage.h"
#include "Game/04_Time/Time.h"
#include "System/Singleton/CameraManager/CameraManager.h"
#include "System/Singleton/CombatCoordinator/CombatCoordinator.h"
#include "Resource/Mesh/02_Skin/SkinMesh.h"
#include <random>
#include <cmath>

namespace PlayerState {
SpecialAttack::SpecialAttack(Player* owner)
	: System(owner)
    , m_AttackDuration(3.5f)  // 総演出時間
    , m_pAfterImage(std::make_unique<AfterImage>())
{
    // 残像設定
    m_pAfterImage->SetLifeTime(0.6f);        // 生存時間（長め）
    m_pAfterImage->SetStartAlpha(0.5f);      // 開始アルファ
    m_pAfterImage->SetDarkness(0.4f);        // 黒み
    m_pAfterImage->SetSpawnInterval(0.05f);  // 生成間隔（長め）
}

SpecialAttack::~SpecialAttack()
{
}

// IDの取得.
constexpr PlayerState::eID SpecialAttack::GetStateID() const
{
	return PlayerState::eID::SpecialAttack;
}

void SpecialAttack::Enter()
{
    m_CurrentTime = 0.0f;
    m_CurrentSlash = 0;
    m_SlashTimer = 0.0f;
    m_IsMoving = false;
    m_SlashPhase = SlashPhase::TowardsBoss;

    // ゲージを消費
    m_pOwner->m_CurrentUltValue = 0.0f;

    // 回転攻撃アニメーション開始（0.624から開始）
    m_pOwner->SetIsLoop(false);  // 手動でループ制御
    m_pOwner->SetAnimSpeed(3.0f);
    m_pOwner->ChangeAnim(Player::eAnim::Attack_2);
    m_pOwner->SetAnimTime(0.624);  // 開始位置を設定

    // 時間スケールを少し遅くして演出
    Time::GetInstance().SetWorldTimeScale(0.8f, 0.3f);

    // カメラを固定（発動地点で固定）
    CameraManager::GetInstance().StartSpecialCamera();
    CameraManager::GetInstance().ShakeCamera(0.2f, 0.5f);
    
    m_pOwner->PlayEffectAtWorldPos("Special", m_pOwner->GetPosition(), 8.f);
    m_pOwner->SetSpecial(true);

    // 被ダメージ判定を無効化
    m_pOwner->SetDamageColliderActive(false);
    // 押し戻し判定を無効化
    m_pOwner->SetPressColliderActive(false);

    // CombatCoordinatorに通知（ボスをSpecialDamageStateに遷移させる）
    CombatCoordinator::GetInstance().Enter();

    // 開始位置を保存
    m_StartPos = m_pOwner->GetPosition();
    
    // 初期角度をランダムに決定
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, DirectX::XM_2PI);
    m_CurrentAngle = angleDist(gen);

    // 残像をクリア
    m_pAfterImage->Clear();
    m_AfterImageTimer = 0.0f;
}

void SpecialAttack::Update()
{
    float deltaTime = m_pOwner->GetDelta();
    m_CurrentTime += deltaTime;

    // アニメーション時間を0.624〜1.872でループ（毎フレーム監視）
    auto meshWeak = m_pOwner->GetAttachMesh();
    auto mesh = meshWeak.lock();
    if (mesh)
    {
        auto skinMesh = std::dynamic_pointer_cast<SkinMesh>(mesh);
        if (skinMesh)
        {
            double animTime = skinMesh->GetAnimTime();
            // 範囲外になったらループ開始位置に戻す
            if (animTime >= 1.872)
            {
                skinMesh->SetAnimTime(0.624);
                m_pOwner->GetAnimationController()->SetTrackPosition(0, 0.624);
            }
        }
    }

    // 残像の更新
    m_pAfterImage->Update(deltaTime);

    // 残像の生成（移動中のみ）
    if (m_SlashPhase == SlashPhase::TowardsBoss)
    {
        m_AfterImageTimer += deltaTime;
        if (m_AfterImageTimer >= m_AfterImageInterval)
        {
            SpawnAfterImage();
            m_AfterImageTimer = 0.0f;
        }
    }

    CombatCoordinator::GetInstance().DamageToBoss(10.5f);

    // 往復斬撃の更新
    UpdateSlashRush(deltaTime);

    // 演出終了
    if (m_SlashPhase == SlashPhase::Finished )
    {
        m_pOwner->PlayEffectAtWorldPos("Spark", m_pOwner->m_TargetPos, 8.f);
        
        // ボスをダウンさせる
        CombatCoordinator::GetInstance().ForceBossDown();

        m_pOwner->ChangeState(PlayerState::eID::Idle);
    }
}



void SpecialAttack::LateUpdate()
{
}

void SpecialAttack::Draw()
{
    // 残像の描画
    if (m_pAfterImage && m_pAfterImage->HasImages())
    {
        auto meshWeak = m_pOwner->GetAttachMesh();
        auto mesh = meshWeak.lock();
        if (mesh)
        {
            auto skinMesh = std::dynamic_pointer_cast<SkinMesh>(mesh);
            if (skinMesh)
            {
                m_pAfterImage->Draw(skinMesh, m_pOwner->GetAnimationController());
            }
        }
    }
}

void SpecialAttack::Exit()
{
    m_pOwner->SetSpecial(false);
    m_pOwner->SetAttackColliderActive(false);
    m_pOwner->m_pAttackCollider->SetAttackAmount(10.0f);

    // 被ダメージ判定を有効化（元に戻す）
    m_pOwner->SetDamageColliderActive(true);
    // 押し戻し判定を有効化（元に戻す）
    m_pOwner->SetPressColliderActive(true);

    // アニメーションをリセット
    m_pOwner->SetIsLoop(false);
    m_pOwner->SetAnimSpeed(1.0f);

    // 時間スケールを戻す
    Time::GetInstance().SetWorldTimeScale(1.0f, 0.2f);

    // カメラを通常モードに戻す
    CameraManager::GetInstance().EndSpecialCamera();

    // 残像をクリア
    m_pAfterImage->Clear();


    m_CurrentTime = 0.f;
    m_CurrentSlash = 0;
    m_SlashPhase = SlashPhase::TowardsBoss;
    m_AfterImageTimer = 0.0f;
    m_FinalPoseTimer = 0.0f;
}

void SpecialAttack::SpawnAfterImage()
{
    DirectX::XMFLOAT3 pos = m_pOwner->GetPosition();
    DirectX::XMFLOAT3 rot = m_pOwner->GetTransform()->Rotation;
    DirectX::XMFLOAT3 scale = m_pOwner->GetTransform()->Scale;
    
    auto meshWeak = m_pOwner->GetAttachMesh();
    auto mesh = meshWeak.lock();
    if (mesh)
    {
        auto skinMesh = std::dynamic_pointer_cast<SkinMesh>(mesh);
        if (skinMesh)
        {
            double animTime = skinMesh->GetAnimTime();
            int animNo = static_cast<int>(Player::eAnim::Attack_2);
            m_pAfterImage->AddImage(pos, rot, scale, animTime, animNo);
        }
    }
}

void SpecialAttack::UpdateSlashRush(float deltaTime)
{
    DirectX::XMFLOAT3 bossPos = m_pOwner->m_TargetPos;
    DirectX::XMFLOAT3 playerPos = m_pOwner->GetPosition();

    switch (m_SlashPhase)
    {
    case SlashPhase::TowardsBoss:
    {
        // ボスに向かって一定速度で移動
        float dx = bossPos.x - playerPos.x;
        float dz = bossPos.z - playerPos.z;
        float dist = std::sqrt(dx * dx + dz * dz);

        if (dist > m_SlashDistance)
        {
            // 一定速度で移動
            float moveAmount = m_SlashSpeed * deltaTime;
            float normalizedDx = dx / dist;
            float normalizedDz = dz / dist;

            DirectX::XMFLOAT3 newPos = {
                playerPos.x + normalizedDx * moveAmount,
                playerPos.y,
                playerPos.z + normalizedDz * moveAmount
            };
            m_pOwner->SetPosition(newPos);

            // ボス方向を向く
            float rotY = std::atan2(dx, dz);
            m_pOwner->GetTransform()->SetRotationY(rotY);
        }
        else
        {
            // ボスに到達 → ダメージ＆エフェクト＆サウンド
            CombatCoordinator::GetInstance().DamageToBoss(m_OraOraDamage);
            CombatCoordinator::GetInstance().HitSpecialAttackToBoss();
            m_pOwner->m_Combo += 2;
            
            // ヒットエフェクト（ボス位置に表示）
            m_pOwner->PlayEffectAtWorldPos("Hit", bossPos, 3.0f);
            
            // ヒットサウンド
            SoundManager::GetInstance().Play("Damage");
            SoundManager::GetInstance().SetVolume("Damage", 8500);
            
            // カメラシェイク
            CameraManager::GetInstance().ShakeCamera(0.1f, 0.08f);
            
            m_CurrentSlash++;

            if (m_CurrentSlash >= m_TotalSlashes)
            {
                // 最終斬撃へ
                m_SlashPhase = SlashPhase::FinalSlash;
                m_SlashTimer = 0.0f;
                m_pOwner->SetAnimSpeed(2.0f);
            }
            else
            {
                // 次の攻撃角度をランダムに決定
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<float> angleDist(0.0f, DirectX::XM_2PI);
                m_CurrentAngle = angleDist(gen);

                // Y座標もランダムに決定
                std::uniform_real_distribution<float> yDist(bossPos.y, bossPos.y + 5.0f);
                float randomY = yDist(gen);

                // ボスの周りに瞬間移動
                float targetX = bossPos.x + std::cos(m_CurrentAngle) * m_ReturnDistance;
                float targetZ = bossPos.z + std::sin(m_CurrentAngle) * m_ReturnDistance;
                
                DirectX::XMFLOAT3 newPos = { targetX, randomY, targetZ };
                m_pOwner->SetPosition(newPos);

                // ボス方向を向く
                float toBossDx = bossPos.x - newPos.x;
                float toBossDz = bossPos.z - newPos.z;
                float rotY = std::atan2(toBossDx, toBossDz);
                m_pOwner->GetTransform()->SetRotationY(rotY);
            }
        }
        break;
    }

    case SlashPhase::FinalSlash:
    {
        // 最終攻撃：大ダメージ
        m_SlashTimer += deltaTime;
        
        if (m_SlashTimer >= 0.3f)
        {
            CombatCoordinator::GetInstance().DamageToBoss(m_AttackDamage);
            m_pOwner->m_Combo += 5;
            
            m_pOwner->PlayEffectAtWorldPos("Special2", bossPos, 10.f);
            SoundManager::GetInstance().Play("Hit2");
            SoundManager::GetInstance().SetVolume("Hit2", 8500);
            CameraManager::GetInstance().ShakeCamera(0.4f, 0.3f);

            // ボスの背後に移動
            DirectX::XMFLOAT3 bossForward = CombatCoordinator::GetInstance().GetBossForward();
            DirectX::XMFLOAT3 behindPos = {
                bossPos.x - bossForward.x * m_FinalPoseDistance,
                bossPos.y,
                bossPos.z - bossForward.z * m_FinalPoseDistance
            };
            m_pOwner->SetPosition(behindPos);

            // ボスと反対方向を向く（ボスに背を向ける）
            float rotY = std::atan2(-bossForward.x, -bossForward.z);
            m_pOwner->GetTransform()->SetRotationY(rotY);
            
            // 最終ポーズの座標と回転を保存（固定用）
            m_FinalPosePosition = behindPos;
            m_FinalPoseRotationY = rotY;

            // SpecialAttack_2アニメーションを再生
            m_pOwner->SetIsLoop(false);
            m_pOwner->SetAnimSpeed(3.0f);
            m_pOwner->ChangeAnim(Player::eAnim::SpecialAttack_2);

            // 最終ポーズカメラに切り替え（プレイヤーの左下から見上げる）
            CameraManager::GetInstance().StartFinalPoseCamera();

            m_FinalPoseTimer = 0.0f;
            m_SlashPhase = SlashPhase::FinalPose;
        }
        break;
    }

	case SlashPhase::FinalPose:
	{
		// 最終ポーズ：座標と向きを固定（カメラに依存しない）
		m_pOwner->SetPosition(m_FinalPosePosition);
		m_pOwner->GetTransform()->SetRotationY(m_FinalPoseRotationY);
        
		m_FinalPoseTimer += deltaTime;

        static bool IsFast = false;
        if(!IsFast)
        {
            IsFast = true;
            m_pOwner->PlayEffectAtWorldPos("Special2", m_pOwner->m_TargetPos, 8.f);

        }

		// アニメーションが終了したら終了（または一定時間経過）
		if (m_FinalPoseTimer >= 2.0f)
		{
            IsFast = false;
			m_SlashPhase = SlashPhase::Finished;
		}
		break;
	}

    case SlashPhase::Finished:
        // 何もしない（Updateで終了判定）
        break;
    }
}

}// PlayerState.
