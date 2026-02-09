#include "BossShoutState.h"

#include "Game//04_Time//Time.h"
#include "Game//01_GameObject//00_MeshObject//00_Character//02_Boss//Boss.h"
#include "Game//01_GameObject//00_MeshObject//00_Character//01_Player//Player.h"
#include "..//..//BossMoveState//BossMoveState.h"

#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"
#include "System/Singleton/ImGui/CImGuiManager.h"

BossShoutState::BossShoutState(Boss* owner)
	: BossAttackStateBase (owner)
    , m_List(enShout::none)
{
}

BossShoutState::~BossShoutState()
{
}

void BossShoutState::Enter()
{

    // 初期値リセット。コライダーは実際のシャウト開始時に有効化する
    m_ShoutElapsed = 0.0f;
    m_ShoutStartRadius = 0.0f;

    //ボスの向きを設定.
    const DirectX::XMFLOAT3 BossPosF = m_pOwner->GetPosition();
    DirectX::XMVECTOR BossPosXM = DirectX::XMLoadFloat3(&BossPosF);

    const DirectX::XMFLOAT3 PlayerPosF = m_pOwner->m_PlayerPos;
    DirectX::XMVECTOR PlayerPosXM = DirectX::XMLoadFloat3(&PlayerPosF);

    DirectX::XMVECTOR Direction = DirectX::XMVectorSubtract(PlayerPosXM, BossPosXM);
    //X,Z平面の方向.
    Direction = DirectX::XMVectorSetY(Direction, 0.0f);

    //Y軸回転角度を計算し、ボスをプレイヤーに向かせる.
    float dx = DirectX::XMVectorGetX(Direction);
    float dz = DirectX::XMVectorGetZ(Direction);
    float angle_radian = std::atan2f(-dx, -dz);
    m_pOwner->SetRotationY(angle_radian);

    //初期位置を保存.
    DirectX::XMStoreFloat3(&m_StartPos, BossPosXM);

    //アニメーション速度.
    m_pOwner->SetAnimSpeed(1.5);
    m_pOwner->ChangeAnim(Boss::enBossAnim::LaserCharge);
    m_EffectPlayed = false;
}

void BossShoutState::Update()
{
    switch (m_List)
    {
    case BossShoutState::enShout::none:
        m_List = enShout::Shout;
        break;

    case BossShoutState::enShout::Shout:
        if (m_pOwner->IsAnimEnd(Boss::enBossAnim::LaserCharge))
        {
            m_pOwner->SetAnimSpeed(1.0);
            m_pOwner->ChangeAnim(Boss::enBossAnim::Laser);
            if (!m_EffectPlayed && m_pOwner)
            {
                m_pOwner->PlayEffect("Shout", m_EffectOffset, m_EffectScale);
                m_EffectPlayed = true;

                SoundManager::GetInstance().Play("Shout", false);
                SoundManager::GetInstance().SetVolume("Shout", 9400);
            }
            // シャウト発動: コライダーを有効化し、半径を徐々に広げる準備
            if (auto* shoutCol = m_pOwner->GetShoutCollider()) {
                shoutCol->SetActive(true);
                shoutCol->SetAttackAmount(m_ShoutDamage);
                m_ShoutElapsed = 0.0f;
                m_ShoutStartRadius = 0.0f;
                if (m_ShoutExpandTime <= 0.0f) {
                    shoutCol->SetRadius(m_ShoutRadius);
                } else {
                    shoutCol->SetRadius(m_ShoutStartRadius);
                }
            }
            m_List = enShout::ShoutTime;
        }
        break;

    case BossShoutState::enShout::ShoutTime:
        // 更新: コライダー半径を徐々に拡大
        if (auto* shoutCol = m_pOwner->GetShoutCollider()) {
            if (shoutCol->GetActive()) {
                float dt = m_pOwner->GetDelta();
                m_ShoutElapsed += dt;
                if (m_ShoutExpandTime > 0.0f) {
                    float t = m_ShoutElapsed / m_ShoutExpandTime;
                    if (t > 1.0f) t = 1.0f;
                    float radius = m_ShoutStartRadius + (m_ShoutRadius - m_ShoutStartRadius) * t;
                    shoutCol->SetRadius(radius);
                    shoutCol->SetHeight(radius);
                } else {
                    shoutCol->SetRadius(m_ShoutRadius);
                }
            }
        }
        if (m_pOwner->IsAnimEnd(Boss::enBossAnim::Laser))
        {
            m_pOwner->SetAnimSpeed(1.5);
            m_pOwner->ChangeAnim(Boss::enBossAnim::LaserEnd);
            m_List = enShout::ShoutToIdol;

            if (auto* shoutCol = m_pOwner->GetShoutCollider()) 
            {
                shoutCol->SetActive(false);
            }
        }
        break;

    case BossShoutState::enShout::ShoutToIdol:
        if (m_pOwner->IsAnimEnd(Boss::enBossAnim::LaserEnd))
        {
            m_pOwner->GetStateMachine()->ChangeState(std::make_shared<BossIdolState>(m_pOwner));
        }
        break;

    default:
        break;
    }
}

void BossShoutState::LateUpdate()
{
}

void BossShoutState::Draw()
{
}

void BossShoutState::Exit()
{
  
}

std::pair<Boss::enBossAnim, float> BossShoutState::GetParryAnimPair()
{
    return std::pair(Boss::enBossAnim::Slash, 2.358f);
}

void BossShoutState::DrawImGui()
{
#if _DEBUG
    ImGui::Begin(IMGUI_JP("BossShout State"));
    CImGuiManager::Slider<float>(IMGUI_JP("ダメージ量"), m_ShoutDamage, 0.0f, 50.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("範囲半径"), m_ShoutRadius, 5.0f, 60.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("ノックバック力"), m_KnockBackPower, 0.0f, 30.0f, true);
    BossAttackStateBase::DrawImGui();
    ImGui::End();
#endif  
}

void BossShoutState::LoadSettings()
{
    BossAttackStateBase::LoadSettings();
    auto srcDir = std::filesystem::path(__FILE__).parent_path();
    auto filePath = srcDir / GetSettingsFileName();
    if (!std::filesystem::exists(filePath)) return;
    json j = FileManager::JsonLoad(filePath);
    if (j.contains("ShoutDamage")) m_ShoutDamage = j["ShoutDamage"].get<float>();
    if (j.contains("ShoutRadius")) m_ShoutRadius = j["ShoutRadius"].get<float>();
    if (j.contains("KnockBackPower")) m_KnockBackPower = j["KnockBackPower"].get<float>();
    // オフセットは ColliderWindow で管理
}

void BossShoutState::SaveSettings() const
{
    json j = SerializeSettings();
    j["ShoutDamage"] = m_ShoutDamage;
    j["ShoutRadius"] = m_ShoutRadius;
    j["KnockBackPower"] = m_KnockBackPower;
    auto srcDir = std::filesystem::path(__FILE__).parent_path();
    auto filePath = srcDir / GetSettingsFileName();
    FileManager::JsonSave(filePath, j);
}
