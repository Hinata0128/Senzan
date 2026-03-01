#include "Laser.h"

#include "Game//04_Time//Time.h"
#include "Game//01_GameObject//00_MeshObject//00_Character//02_Boss//Boss.h"
#include "Game//01_GameObject//00_MeshObject//00_Character//01_Player//Player.h"
#include "00_MeshObject/00_Character/02_Boss/BossIdolState/BossIdolState.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Utility/FileManager/FileManager.h"
#include <cmath>

Laser::Laser(Boss* owner)
    : BossAttackStateBase(owner)
    , m_State(enLaser::None)

    , m_ChargeDuration(3.0f)
    , m_ChargeElapsed(0.0f)

    , m_FireDuration(1.0f)
    , m_FireElapsed(0.0f)

    , m_LaserDamage(10.0f)
    , m_LaserRadius(5.0f)
    , m_LaserRange(200.0f)

    , m_EffectPlayed(false)

    , m_pBossIdol()
{
    //開始時にレーザー攻撃のパラメータを取得する.
    try
    {
        LoadSettings();
    }
    catch (...)
    {
        //何も書かない.
    }
}

Laser::~Laser()
{
}

//最初に入る.
void Laser::Enter()
{
    try
    {
        LoadSettings();
    }
    catch (...)
    {
        //何も書かない.
    }
    BossAttackStateBase::Enter();

    //ステートをチャージにする.
    m_State = enLaser::Charge;
    m_ChargeElapsed = 0.0f;
    m_FireElapsed = 0.0f;
    m_EffectPlayed = false;

    //プレイヤーの方に向く.
    FacePlayerInstantYaw();
    //アニメーションの再生速度.
    m_pOwner->SetAnimSpeed(1.5);
    //アニメーションをレーザーの溜めに変更する.
    m_pOwner->ChangeAnim(Boss::enBossAnim::LaserCharge);
}

void Laser::Update()
{
    BossAttackStateBase::Update();
    //デルタタイムの取得.
    float deltaTime = Time::GetInstance().GetDeltaTime();
    UpdateBaseLogic(deltaTime);

    if (m_State == enLaser::Fire || m_State == enLaser::Cool)
    {
        //レーザーの当たり判定を取得.
        if (auto* Col = m_pOwner->GetLaserCollider())
        {
            //攻撃判定あり.
            Col->SetActive(true);
            //レーザーの大きさを時間経過に合わせて変化させる.
            float T = (m_FireElapsed / (m_FireDuration > 0.0f ? m_FireDuration : 1.0f));
            if (T > 1.0f)
            {
                //最大値を1.0fにする.
                T = 1.0f;
            }
            //最終的な半径の計算(徐々に大きくする演出).
            float Randius = m_LaserRadius * T;
            //レーザーの当たり判定の半径取得.
            Col->SetRadius(Randius);
            //位置の更新(毎フレーム).
            Col->SetPositionOffset(0.0f, 0.0f, (Randius * -30.0f) + (35.0f));
        }
    }

    //switch文で状態の遷移をする.
    switch (m_State)
    {
    case Laser::enLaser::None:
        //何も書かない.
        break;
    case Laser::enLaser::Charge:
        m_ChargeElapsed += deltaTime;
        //毎フレームプレイヤーの方に向き続ける.
        FacePlayerYawContinuous();
        if (m_ChargeElapsed >= m_ChargeDuration || m_pOwner->IsAnimEnd(Boss::enBossAnim::LaserCharge))
        {
            //発射に遷移.
            m_State = enLaser::Fire;
            m_pOwner->SetAnimSpeed(1.0);
            m_pOwner->ChangeAnim(Boss::enBossAnim::Laser);
        }
        break;
    case Laser::enLaser::Fire:
        m_FireElapsed += deltaTime;
        if (m_EffectPlayed == false)
        {
            if (m_pOwner)
            {
                DirectX::XMFLOAT3 BossPos_F = m_pOwner->GetPosition();
                DirectX::XMFLOAT3 LocalOffset(0.0f, 12.0f, -5.0f);
                float Yaw = m_pOwner->GetTransform()->Rotation.y;
                float c = cosf(Yaw);
                float s = sinf(Yaw);
                DirectX::XMFLOAT3 WorldOffset;
                WorldOffset.x = c * LocalOffset.x + s * LocalOffset.z;
                WorldOffset.y = LocalOffset.y;
                WorldOffset.z = -s * LocalOffset.x + c * LocalOffset.z;
                DirectX::XMFLOAT3 EffectPos{ BossPos_F.x + WorldOffset.x, BossPos_F.y + WorldOffset.y, BossPos_F.z + WorldOffset.z };
                DirectX::XMFLOAT3 EulerRot{ 0.0f, Yaw, 0.0f };
                //エフェクトの表示.
                m_pOwner->PlayEffectAtWorldPos("Boss_Laser", EffectPos, EulerRot, 2.0f, false);
                //音の再生.
                SoundManager::GetInstance().Play("Beam");
                SoundManager::GetInstance().SetVolume("Beam", 9000);
            }
            m_EffectPlayed = true;
        }
        if (m_FireElapsed >= m_FireDuration || m_pOwner->IsAnimEnd(Boss::enBossAnim::Laser))
        {
            //アニメーション速度.
            m_pOwner->SetAnimSpeed(1.5);
            //アニメーションの変更.
            m_pOwner->ChangeAnim(Boss::enBossAnim::LaserEnd);
            //状態の遷移.
            m_State = enLaser::Cool;
        }
        break;
    case Laser::enLaser::Cool:
        //レーザーの当たり判定取得.
        if (auto* col = m_pOwner->GetLaserCollider())
        {
            //攻撃判定を強制敵に消す.
            col->SetActive(false);
        }
        if (m_pOwner->IsAnimEnd(Boss::enBossAnim::LaserEnd))
        {
            //状態の遷移.
            m_State = enLaser::Trans;
        }

        break;
    case Laser::enLaser::Trans:
        //アイドルへの遷移.
        m_pOwner->GetStateMachine()->ChangeState(std::make_shared<BossIdolState>(m_pOwner));
        break;
    default:
        break;
    }
}

void Laser::LateUpdate()
{
}

void Laser::Draw()
{
}

void Laser::Exit()
{
    try
    {
        SaveSettings();
    }
    catch (...)
    {
    }
    if (auto* Col = m_pOwner->GetShoutCollider())
    {
        Col->SetActive(false);
    }
}

std::pair<Boss::enBossAnim, float> Laser::GetParryAnimPair()
{
    return std::pair(Boss::enBossAnim::Laser, 0.0f);
}

void Laser::DrawImGui()
{
#if _DEBUG
    //レーザーのImGui表示.
    ImGui::Begin(IMGUI_JP("レーザーのステータス"));
    ImGui::Text(IMGUI_JP("Laser State : %d"), static_cast<int>(m_State));
    CImGuiManager::Slider<float>(IMGUI_JP("チャージ時間"), m_ChargeDuration, 0.0f, 5.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("発射時間"), m_FireDuration, 0.0f, 5.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("ダメージ"), m_LaserDamage, 0.0f, 100.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("範囲"), m_LaserRadius, 0.0f, 200.0f, true);
    CImGuiManager::Slider<float>(IMGUI_JP("射程"), m_LaserRange, 0.0f, 1000.0f, true);
    //レーザ攻撃をしている時に強制的に攻撃のタイミング等をImGuiで設定できる.
    if (ImGui::Button(IMGUI_JP("Force Charge")))
    {
        m_State = enLaser::Charge; m_ChargeElapsed = 0.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button(IMGUI_JP("Force Fire")))
    {
        m_State = enLaser::Fire; m_FireElapsed = 0.0f; if (auto* c = m_pOwner->GetShoutCollider())
        {
            c->SetActive(true);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(IMGUI_JP("Force Cool")))
    {
        m_State = enLaser::Cool;
    }
    ImGui::SameLine();
    if (ImGui::Button(IMGUI_JP("Force Trans")))
    {
        m_State = enLaser::Trans;
    }
    BossAttackStateBase::DrawImGui();
    //当たり判定表示用.
    if (auto* col = m_pOwner->GetShoutCollider())
    {
        ImGui::Separator();
        ImGui::Text(IMGUI_JP("Shout Collider: Active=%s Radius=%.2f"), col->GetActive() ? "ON" : "OFF", col->GetRadius());
        DirectX::XMFLOAT3 off = col->GetPositionOffset();
        ImGui::Text(IMGUI_JP("Offset: (%.2f, %.2f, %.2f)"), off.x, off.y, off.z);
    }
    if (ImGui::Button(IMGUI_JP("Load")))
    {
        try
        {
            LoadSettings();
        }
        catch (...)
        {
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(IMGUI_JP("Save")))
    {
        try
        {
            SaveSettings();
        }
        catch (...) {}
    }
    ImGui::End();

#endif
}

//Laserの攻撃のパラメータ用のjsonファイルの読み込み.
void Laser::LoadSettings()
{
    auto FilePath = GetSettingsFileName();
    if (std::filesystem::exists(FilePath) == false)
    {
        return;
    }

    //jsonに入る攻撃等のパラメータのこうもく.
    json j = FileManager::JsonLoad(FilePath);
    if (j.contains("ChargeDuration"))
    {
        m_ChargeDuration = j["ChargeDuration"].get<float>();
    }
    if (j.contains("FireDuration"))
    {
        m_FireDuration = j["FireDuration"].get<float>();
    }
    if (j.contains("LaserDuration"))
    {
        m_LaserDamage = j["LaserDuration"].get<float>();
    }
    if (j.contains("LaserRadius"))
    {
        m_LaserRadius = j["LaserRadius"].get<float>();
    }
    if (j.contains("LaserRange"))
    {
        m_LaserRange = j["LaserRange"].get<float>();
    }
}

//Laserの攻撃のパラメータ用のjsonファイルの保存.
void Laser::SaveSettings() const
{
    auto FilePath = GetSettingsFileName();
    if (FilePath.is_absolute() == false)
    {
        auto Dir = std::filesystem::current_path() / "Data" / "Json" / "Boss";
        std::filesystem::create_directories(Dir);
        FilePath = Dir / FilePath;
    }
    json j = SerializeSettings();
    j["ChargeDuration"] = m_ChargeDuration;
    j["FireDuration"] = m_FireDuration;
    j["LaserDamage"] = m_LaserDamage;
    j["LaserRadius"] = m_LaserRadius;
    j["LaserRange"] = m_LaserRange;
    FileManager::JsonSave(FilePath, j);
}

