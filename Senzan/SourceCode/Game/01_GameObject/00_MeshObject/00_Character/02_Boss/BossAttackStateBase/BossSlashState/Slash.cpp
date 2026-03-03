#include "Slash.h"

#include "Resource/Mesh/02_Skin/SkinMesh.h"
#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"
#include "00_MeshObject/00_Character/02_Boss/Boss.h"
#include "00_MeshObject/00_Character/01_Player/Player.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Utility/FileManager/FileManager.h"

Slash::Slash(Boss* owner)
    : BossAttackStateBase(owner)
    , m_State(enList::none)
{
}

Slash::~Slash()
{
}

void Slash::Enter()
{
    // 1. JSONをロードして m_ChargeTime などを確定させる
    LoadSettings();

    // 2. 基底のEnterを呼ぶ (ここで JSON の ColliderWindows 等が登録される)
    BossAttackStateBase::Enter();

    // 3. パラメータ初期化
    m_CurrentTime = 0.0f;
    m_IsSwingSoundPlayed = false;
    m_IsHitSoundPlayed = false;

    // 4. アニメーション設定
    m_pOwner->SetIsLoop(false);
    m_pOwner->SetAnimTime(0.0);

    // JSONのアニメーション速度を適用 (m_AnimSpeeds["Attack"] などを使う)
    // JSONの Attack: 7.644 はかなり速いので、反映させると判定も一瞬になります
    m_pOwner->SetAnimSpeed(1.0);

    m_pOwner->ChangeAnim(Boss::enBossAnim::Slash);

    // 5. 状態遷移
    m_State = enList::ChargeSlash;
}

void Slash::Update()
{
    const float deltaTime = m_pOwner->GetDelta();
    BossAttackStateBase::Update();
    UpdateBaseLogic(deltaTime);

    bool isTransition = false;

    switch (m_State)
    {
    case enList::ChargeSlash:
        if (m_CurrentTime < m_HomingEndTime) FacePlayerYawContinuous();
        if (m_CurrentTime >= m_ChargeTime) {
            m_State = enList::SlashAttack;
            if (!m_IsSwingSoundPlayed) {
                SoundManager::GetInstance().Play("BossSwing", false);
                m_IsSwingSoundPlayed = true;
            }
        }
        break;

    case enList::SlashAttack:
        // --- 修正ポイント：当たり判定の実行 ---
        if (auto* col = m_pOwner->GetSlashCollider())
        {
            // 1. 攻撃フェーズ中は強制的に判定をONにする
            col->SetActive(true);

            // 2. 攻撃力を設定（m_AttackAmount が 0 だとダメージが入らない）
            col->SetAttackAmount(m_AttackAmount);

            // 3. ヒット時の音処理
            if (!m_IsHitSoundPlayed && !col->GetCollisionEvents().empty())
            {
                SoundManager::GetInstance().Play("Player_Damage_SE", false);
                m_IsHitSoundPlayed = true;
            }
        }

        // 状態遷移
        if (m_TransitionOnAnimEnd_Attack) {
            if (m_pOwner->IsAnimEnd(Boss::enBossAnim::Slash)) {
                m_State = enList::SlashIdol;
                m_pOwner->ChangeAnim(Boss::enBossAnim::SlashToIdol);
            }
        }
        else if (m_CurrentTime >= m_ChargeTime + m_AttackTime) {
            m_State = enList::SlashIdol;
            m_pOwner->ChangeAnim(Boss::enBossAnim::SlashToIdol);
        }
        break;

    case enList::SlashIdol:
        // 攻撃が終わったら判定を消す
        if (auto* col = m_pOwner->GetSlashCollider()) col->SetActive(false);

        if (m_TransitionOnAnimEnd_Exit)
            isTransition = m_pOwner->IsAnimEnd(Boss::enBossAnim::SlashToIdol);
        else
            isTransition = (m_CurrentTime >= m_ChargeTime + m_AttackTime + m_EndTime);

        if (isTransition && !m_IsDebugStop) {
            m_pOwner->GetStateMachine()->ChangeState(std::make_shared<BossIdolState>(m_pOwner));
        }
        break;
    }
}

void Slash::LateUpdate()
{
    BossAttackStateBase::LateUpdate();
}

void Slash::Draw()
{
    BossAttackStateBase::Draw();
}

void Slash::Exit()
{
    BossAttackStateBase::Exit();
    BossAttackStateBase::Exit();
    if (auto* col = m_pOwner->GetSlashCollider()) {
        col->SetActive(false);
    }
}

std::pair<Boss::enBossAnim, float> Slash::GetParryAnimPair()
{
    return std::pair(Boss::enBossAnim::Slash, 2.360f);
}

void Slash::DrawImGui()
{
#if _DEBUG
    ImGui::Begin(IMGUI_JP("ボス斬撃設定"));
    ImGui::Text("Phase: %d", (int)m_State);
    BossAttackStateBase::DrawImGui();
    if (ImGui::Button("Save Settings")) SaveSettings();
    ImGui::End();
#endif
}

void Slash::LoadSettings()
{
    BossAttackStateBase::LoadSettings();
}

void Slash::SaveSettings() const
{
    BossAttackStateBase::SaveSettings();
}