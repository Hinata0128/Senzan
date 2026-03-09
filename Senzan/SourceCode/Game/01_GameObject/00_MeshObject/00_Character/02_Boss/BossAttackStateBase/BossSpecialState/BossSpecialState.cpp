#include "BossSpecialState.h"
#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"
#include "00_MeshObject/00_Character/02_Boss/Boss.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Utility/FileManager/FileManager.h"
#include "System/Utility/Math/Easing/Easing.h"

BossSpecialState::BossSpecialState(Boss* owner)
    : BossAttackStateBase(owner)
    , m_List(enSpecial::None)
{
    try { LoadSettings(); }
    catch (...) {}
}

BossSpecialState::~BossSpecialState() {}

void BossSpecialState::Enter() {
    try { LoadSettings(); }
    catch (...) {}
    BossAttackStateBase::Enter();

    m_IsMoving = false;
    m_AnimSlowed = false;
    m_SlowElapsed = 0.0f;
    m_VerticalTimer = 0.0f;
    m_StartY = m_pOwner->GetPosition().y;

    m_pOwner->SetIsLoop(false);
    m_pOwner->SetAnimSpeed(2.0); // 溜めは速めに
    m_pOwner->ChangeAnim(Boss::enBossAnim::Special_0);

    SoundManager::GetInstance().Play("AttackCharge", false);
}

void BossSpecialState::Update() {
    BossAttackStateBase::Update();
    float dt = m_pOwner->GetDelta();
    UpdateBaseLogic(dt);

    switch (m_List) {
    case enSpecial::None:
        // 溜め中のスロー演出 (StompState 8割ベース)
        if (!m_AnimSlowed && m_CurrentTime >= m_WaitSeconds) {
            m_AnimSlowed = true;
            m_pOwner->SetAnimSpeed(m_SlowAnimSpeed);
        }
        if (m_AnimSlowed) {
            m_SlowElapsed += dt;
            if (m_SlowElapsed >= m_SlowDuration) {
                m_AnimSlowed = false;
                m_pOwner->SetAnimSpeed(1.0f);
            }
        }

        FacePlayerInstantYaw();

        // 溜めアニメーション終了で突進開始
        if (m_pOwner->IsAnimEnd(Boss::enBossAnim::Special_0) && !m_IsMoving) {
            m_IsMoving = true;
            m_pOwner->SetAnimSpeed(1.0f);
            m_pOwner->ChangeAnim(Boss::enBossAnim::Special_1); // ここでパリィ可能アニメへ
            m_List = enSpecial::Attack;
            m_MoveTimer = 0.0f;

            // ターゲット方向の初期化
            DirectX::XMVECTOR vBoss = DirectX::XMLoadFloat3(&m_pOwner->GetPosition());
            DirectX::XMVECTOR vPlayer = DirectX::XMLoadFloat3(&m_pOwner->m_PlayerPos);
            DirectX::XMVECTOR vDiff = DirectX::XMVectorSubtract(vPlayer, vBoss);
            m_Distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(vDiff));
            DirectX::XMStoreFloat3(&m_CurrentDirection, DirectX::XMVector3Normalize(DirectX::XMVectorSetY(vDiff, 0.0f)));

            SoundManager::GetInstance().Play("Jump", false);
        }
        break;

    case enSpecial::Attack:
        if (m_IsMoving) {
            m_MoveTimer += dt;
            BossAttack();
        }

        // 攻撃アニメーションの終了、または移動完了で次のフェーズへ
        // パリィされなかった場合の処理
        if (m_pOwner->IsAnimEnd(Boss::enBossAnim::Special_1) || !m_IsMoving) {
            m_IsMoving = false;
            m_pOwner->SetAnimSpeed(1.5f);
            m_pOwner->ChangeAnim(Boss::enBossAnim::SpecialToIdol);
            m_List = enSpecial::CoolTime;

            SoundManager::GetInstance().Play("Landing", false);
        }
        break;

    case enSpecial::CoolTime:
        if (m_pOwner->IsAnimEnd(Boss::enBossAnim::SpecialToIdol)) {
            m_List = enSpecial::Trans;
        }
        break;

    case enSpecial::Trans:
        m_pOwner->GetStateMachine()->ChangeState(std::make_shared<BossIdolState>(m_pOwner));
        break;
    }
}

void BossSpecialState::BossAttack() {
    float dt = m_pOwner->GetDelta();
    float progress = std::min(m_MoveTimer / m_MoveDuration, 1.0f);

    // --- 2割のロジック: 緩やかな追尾 ---
    DirectX::XMVECTOR vCurrentDir = DirectX::XMLoadFloat3(&m_CurrentDirection);
    DirectX::XMVECTOR vPlayer = DirectX::XMLoadFloat3(&m_pOwner->m_PlayerPos);
    DirectX::XMVECTOR vBoss = DirectX::XMLoadFloat3(&m_pOwner->GetPosition());
    DirectX::XMVECTOR vToPlayer = DirectX::XMVector3Normalize(DirectX::XMVectorSetY(DirectX::XMVectorSubtract(vPlayer, vBoss), 0.0f));

    // 進行方向をプレイヤーの方へ少しずつ向ける
    float lerpFactor = dt * 2.0f; // 追尾の強さ
    vCurrentDir = DirectX::XMVector3Normalize(DirectX::XMVectorLerp(vCurrentDir, vToPlayer, lerpFactor));
    DirectX::XMStoreFloat3(&m_CurrentDirection, vCurrentDir);

    // --- 8割のロジック: イージング移動 ---
    auto easeOut = [](float x) { return 1.0f - (1.0f - x) * (1.0f - x); };
    float eased = easeOut(progress);
    float prevEased = easeOut(std::max(0.0f, progress - (dt / m_MoveDuration)));
    float frameDistance = m_Distance * (eased - prevEased);

    DirectX::XMFLOAT3 movement;
    DirectX::XMStoreFloat3(&movement, DirectX::XMVectorScale(vCurrentDir, frameDistance));
    m_pOwner->AddPosition(movement);

    // 向きを移動方向に合わせる
    float angle = std::atan2f(-m_CurrentDirection.x, -m_CurrentDirection.z);
    m_pOwner->SetRotationY(angle);

    // 垂直イージング (Stompベース)
    if (m_UseVerticalEasing) {
        m_VerticalTimer += dt;
        float totalVert = m_AscentDuration + m_DescentDuration;
        float t = std::min(m_VerticalTimer, totalVert);
        float newY = m_StartY;

        if (t <= m_AscentDuration) {
            float out; MyEasing::UpdateEasing(MyEasing::Type::OutSine, t, m_AscentDuration, 0.0f, m_AscentHeight, out);
            newY = m_StartY + out;
        }
        else {
            float out; MyEasing::UpdateEasing(MyEasing::Type::InQuad, t - m_AscentDuration, m_DescentDuration, m_AscentHeight, 0.0f, out);
            newY = m_StartY + out;
        }
        m_pOwner->SetPositionY(newY);
    }

    if (progress >= 1.0f) m_IsMoving = false;
}

std::pair<Boss::enBossAnim, float> BossSpecialState::GetParryAnimPair() {
    // Special_1(突進中)のアニメーション開始直後からパリィ可能にする
    return std::pair(Boss::enBossAnim::Special_1, 0.1f);
}

void BossSpecialState::Exit() {
    try { SaveSettings(); }
    catch (...) {}
    m_pOwner->SetPositionY(m_StartY);
}

void BossSpecialState::DrawImGui()
{
#if _DEBUG
    ImGui::Begin("BossSpecialState Settings");

    CImGuiManager::Slider<float>("溜め時間(秒)", m_WaitSeconds, 0.0f, 5.0f);
    CImGuiManager::Slider<float>("スロー時アニメ速度", m_SlowAnimSpeed, 0.0f, 1.0f);
    CImGuiManager::Slider<float>("スロー継続時間", m_SlowDuration, 0.0f, 5.0f);

    ImGui::Separator();
    CImGuiManager::Slider<float>("突進時間(秒)", m_MoveDuration, 0.1f, 5.0f);
    CImGuiManager::Slider<float>("追尾性能(強さ)", m_MaxTrackingAngle, 0.0f, 10.0f); // lerpFactorとして使用

    ImGui::Separator();
    ImGui::Checkbox("垂直イージングを使用", &m_UseVerticalEasing);
    CImGuiManager::Slider<float>("上昇高さ", m_AscentHeight, 0.0f, 50.0f);
    CImGuiManager::Slider<float>("上昇時間", m_AscentDuration, 0.01f, 2.0f);
    CImGuiManager::Slider<float>("下降時間", m_DescentDuration, 0.01f, 2.0f);

    if (ImGui::Button("Save Settings")) SaveSettings();
    if (ImGui::Button("Load Settings")) LoadSettings();

    // 基底クラス（当たり判定ウィンドウなど）の表示
    BossAttackStateBase::DrawImGui();

    ImGui::End();
#endif
}

void BossSpecialState::LoadSettings()
{
    // 基底クラスの設定（ColliderWindowsなど）を読み込む
    BossAttackStateBase::LoadSettings();

    auto path = std::filesystem::current_path() / "Data/Json/Boss" / GetSettingsFileName();
    if (!std::filesystem::exists(path)) return;

    json j = FileManager::JsonLoad(path);
    if (j.contains("WaitSeconds")) m_WaitSeconds = j["WaitSeconds"];
    if (j.contains("SlowAnimSpeed")) m_SlowAnimSpeed = j["SlowAnimSpeed"];
    if (j.contains("SlowDuration")) m_SlowDuration = j["SlowDuration"];
    if (j.contains("MoveDuration")) m_MoveDuration = j["MoveDuration"];
    if (j.contains("MaxTrackingAngle")) m_MaxTrackingAngle = j["MaxTrackingAngle"];
    if (j.contains("UseVerticalEasing")) m_UseVerticalEasing = j["UseVerticalEasing"];
    if (j.contains("AscentHeight")) m_AscentHeight = j["AscentHeight"];
    if (j.contains("AscentDuration")) m_AscentDuration = j["AscentDuration"];
    if (j.contains("DescentDuration")) m_DescentDuration = j["DescentDuration"];
}

void BossSpecialState::SaveSettings() const
{
    // 基底クラスの設定をシリアライズ
    json j = SerializeSettings();

    j["WaitSeconds"] = m_WaitSeconds;
    j["SlowAnimSpeed"] = m_SlowAnimSpeed;
    j["SlowDuration"] = m_SlowDuration;
    j["MoveDuration"] = m_MoveDuration;
    j["MaxTrackingAngle"] = m_MaxTrackingAngle;
    j["UseVerticalEasing"] = m_UseVerticalEasing;
    j["AscentHeight"] = m_AscentHeight;
    j["AscentDuration"] = m_AscentDuration;
    j["DescentDuration"] = m_DescentDuration;

    auto dir = std::filesystem::current_path() / "Data/Json/Boss";
    std::filesystem::create_directories(dir);
    FileManager::JsonSave(dir / GetSettingsFileName(), j);
}