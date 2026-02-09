#include "BossAttackStateBase.h"

#include "Game/04_Time/Time.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/Boss.h"
#include "System/Utility/FileManager/FileManager.h"
#include "Game/03_Collision/00_Core/02_Sphere/SphereCollider.h"

#include "System/Singleton/ImGui/CImGuiManager.h"
#include <cstring>
#include <cmath>

BossAttackStateBase::BossAttackStateBase(Boss* owner)
    : StateBase<Boss>(owner)
    , m_pTransform(std::make_shared<Transform>())
{
}

bool BossAttackStateBase::UpdateColliderWindows(float prevTime, float currentTime, std::vector<ColliderWindow>& windows)
{
    bool anyJust = false;
    for (auto& window : windows)
    {
        if (window.IsEnd) { continue; }

        // ジャストタイム判定更新（開始時間 - JustTime ～ 開始時間 の間 true）
        float justWindowStart = window.Start - window.JustTime;
        if (window.JustTime > 0.0f && currentTime >= justWindowStart && currentTime < window.Start)
        {
            window.IsJustWindow = true;
            anyJust = true;
        }
        else
        {
            window.IsJustWindow = false;
        }

        // Activation: for OneShot windows, activate only when crossing the start time
        if (!window.IsAct)
        {
            bool shouldActivate = false;
            if (window.OneShot)
            {
                // Activate only when prevTime < Start <= currentTime
                if (prevTime < window.Start && currentTime >= window.Start && !window.OneShotTriggered)
                {
                    shouldActivate = true;
                    window.OneShotTriggered = true;
                }
            }
            else
            {
                if (currentTime >= window.Start) shouldActivate = true;
            }

            if (shouldActivate)
            {
                m_pOwner->SetColliderActiveByName(window.BoneName, true);
                // apply current state collider settings to the activated collider so UI changes take effect
                ColliderBase* targetCol = nullptr;
                if (window.BoneName == "boss_Hand_R") targetCol = m_pOwner->GetSlashCollider();
                else if (window.BoneName == "boss_pSphere28") targetCol = m_pOwner->GetStompCollider();
                else if (window.BoneName == "boss_Shout") targetCol = m_pOwner->GetShoutCollider();
                else if (window.BoneName == "boss_Spinning") targetCol = m_pOwner->GetSpinningCollider();
                else if (window.BoneName == "boss_Laser") targetCol = m_pOwner->GetLaserCollider();
                if (targetCol) {
                    // Use virtual setters so Sphere/Box/Capsule implementations receive values
                    targetCol->SetRadius(m_ColliderWidth);
                    targetCol->SetHeight(m_ColliderHeight);
                    targetCol->SetAttackAmount(m_AttackAmount);
                    // Also apply initial offset
                    targetCol->SetPositionOffset(window.Offset);
                }
                window.IsAct = true;
            }
        }
        // ジャスト時の1回演出: IsJustWindow が true になった瞬間に実行
        if (window.IsJustWindow && !window.JustPlayed)
        {
            // プレイヤーまたはボス固有の演出をここで実行する。
            // 例: UI の Parry_Flash を画面上に表示させる
            if (m_pOwner)
            {
                // ボスの頭のワールド位置を取得して画面座標に変換して UI 表示
                DirectX::XMFLOAT3 headPos{0.0f, 0.0f, 0.0f};
                // Bone 名が設定されていれば、試しに Mesh からボーン位置を取得
                if (!m_pOwner->GetResourceName().empty()) {
                    // Boss のメッシュからボーン位置取得 API がある場合は使用する
                }
                // とりあえず画面中央上寄せで表示
                DirectX::XMFLOAT2 screenPos{ WND_WF * 0.5f, WND_HF * 0.5f };
                auto charPtr = dynamic_cast<Character*>(m_pOwner);
                if (charPtr)
                {
                    charPtr->PlayEffectUIAtScreenPos("Parry_Flash", screenPos, 100.0f);
                    SoundManager::GetInstance().Play("Attack", false);
                    SoundManager::GetInstance().SetVolume("Attack", 9000);
                }
            }
            window.JustPlayed = true;
        }

        // 当たり判定が有効な間、毎フレームオフセットを更新
        // ColliderBase::GetPosition()が親の回転を自動適用するため、ローカルオフセットをそのまま設定
        if (window.IsAct && !window.IsEnd)
        {
            ColliderBase* targetCol = nullptr;
            if (window.BoneName == "boss_Hand_R") targetCol = m_pOwner->GetSlashCollider();
            else if (window.BoneName == "boss_pSphere28") targetCol = m_pOwner->GetStompCollider();
            else if (window.BoneName == "boss_Shout") targetCol = m_pOwner->GetShoutCollider();
            else if (window.BoneName == "boss_Spinning") targetCol = m_pOwner->GetSpinningCollider();
            else if (window.BoneName == "boss_Laser") targetCol = m_pOwner->GetLaserCollider();
            if (targetCol) {
                // ColliderBase provides SetPositionOffset; call on base pointer so all collider types are supported
                targetCol->SetPositionOffset(window.Offset.x, window.Offset.y, window.Offset.z);
                targetCol->SetDebugInfo();
            }
        }

        // Deactivation: for OneShot, deactivate after the next frame (i.e., when currentTime > Start)
        if (!window.IsEnd)
        {
            if (window.IsAct && currentTime >= (window.Start + window.Duration))
            {
                m_pOwner->SetColliderActiveByName(window.BoneName, false);
                window.IsEnd = true;
            }
        }
    }
    return anyJust;
}

DirectX::XMFLOAT3 BossAttackStateBase::ComputeMovementEndPos(const MovementWindow& mv, const DirectX::XMFLOAT3& startPos, const DirectX::XMFLOAT3& targetPos) const
{
    // デフォルト実装: ターゲット方向に対して Speed * Duration * Distance だけ移動する
    DirectX::XMVECTOR start_v = DirectX::XMLoadFloat3(&startPos);
    DirectX::XMVECTOR target_v = DirectX::XMLoadFloat3(&targetPos);
    DirectX::XMVECTOR dir_v = DirectX::XMVectorSubtract(target_v, start_v);
    float len = DirectX::XMVectorGetX(DirectX::XMVector3Length(dir_v));
    DirectX::XMFLOAT3 endPos = startPos;
    if (len > 1e-6f)
    {
        DirectX::XMVECTOR dir_n = DirectX::XMVector3Normalize(dir_v);
        DirectX::XMVECTOR move_amount_v = DirectX::XMVectorScale(dir_n, mv.Speed * mv.Duration * mv.Distance);
        DirectX::XMVECTOR end_v = DirectX::XMVectorAdd(start_v, move_amount_v);
        DirectX::XMStoreFloat3(&endPos, end_v);
    }
    return endPos;
}

void BossAttackStateBase::Enter()
{
    LoadSettings();
    m_CurrentTime = 0.0f;
    // Reset previous time so UpdateColliderWindows sees a fresh time window on re-entry
    // Use a negative value to ensure prevTime < Start for OneShot activations that begin at t=0
    m_PrevTime = -1.0f;
    
    // ウィンドウフラグをリセット
    for (auto& window : m_ColliderWindows)
    {
        window.Reset();
    }
    for (auto& mv : m_MovementWindows)
    {
        mv.Reset();
    }
    for (auto& eff : m_EffectWindows)
    {
        eff.Reset();
    }
    
    // 全ての攻撃コライダーを最初に非アクティブにする
    if (m_pOwner)
    {
        m_pOwner->SetColliderActiveByName("boss_Hand_R", false);
        m_pOwner->SetColliderActiveByName("boss_pSphere28", false);
        m_pOwner->SetColliderActiveByName("boss_Shout", false);
        m_pOwner->SetColliderActiveByName("boss_Spinning", false);

        // 攻撃開始時にプレイヤー方向を向く（デフォルトで有効）
        if (m_AutoFaceOnEnter)
        {
            FacePlayerInstantYaw();
        }
    }
}

void BossAttackStateBase::FacePlayerInstantYaw()
{
    if (!m_pOwner) return;
    if (!m_pOwner->m_IslockOnPlayer) { return; }
    const DirectX::XMFLOAT3 bossPos = m_pOwner->GetPosition();
    const DirectX::XMFLOAT3 playerPos = m_pOwner->m_PlayerPos;
    float dx = playerPos.x - bossPos.x;
    float dz = playerPos.z - bossPos.z;
    float angle_radian = std::atan2f(-dx, -dz);
    m_pOwner->SetRotationY(angle_radian);
}

void BossAttackStateBase::FacePlayerYawContinuous()
{
    if (!m_pOwner) { return; }
    if (!m_pOwner->m_IslockOnPlayer) { return; }

    DirectX::XMFLOAT3 bossPos = m_pOwner->GetPosition();
    DirectX::XMFLOAT3 playerPos = m_pOwner->GetTargetPos();
    DirectX::XMVECTOR BossPosXM = DirectX::XMLoadFloat3(&bossPos);
    DirectX::XMVECTOR PlayerPosXM = DirectX::XMLoadFloat3(&playerPos);
    DirectX::XMVECTOR Dir = DirectX::XMVectorSubtract(PlayerPosXM, BossPosXM);
    Dir = DirectX::XMVectorSetY(Dir, 0.0f);
    if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(Dir)) > 0.0001f)
    {
        float dx = DirectX::XMVectorGetX(Dir);
        float dz = DirectX::XMVectorGetZ(Dir);
        float angle_radian = std::atan2f(dx, dz) + DirectX::XM_PI;
        m_pOwner->SetRotationY(angle_radian);
    }
}

void BossAttackStateBase::Update()
{
    // デバッグ用の ImGui を表示
    DrawImGui();

    // デバッグ停止フラグが立っている場合は Update の残り処理をスキップして
    // ステート遷移を発生させないようにする。
    if (m_IsDebugStop)
    {
        return;
    }
}

void BossAttackStateBase::UpdateBaseLogic(float dt)
{
    // 秒数更新.
    m_CurrentTime += dt;

    // アニメーション速度制御.
    float currentAnimSpeed = 0.f;
    if (m_CurrentTime > m_EndTime)
    {
        currentAnimSpeed = m_AnimSpeedExit;
    }
    else if (m_CurrentTime > m_ChargeTime)
    {
        currentAnimSpeed = m_AnimSpeedAttack;
    }
    else
    {
        currentAnimSpeed = m_AnimSpeedCharge;
    }

    m_AnimSpeed = currentAnimSpeed;
    m_pOwner->SetAnimSpeed(currentAnimSpeed);

    // 当たり判定更新 (共通ヘルパーを使用)
    bool anyJust = UpdateColliderWindows(m_PrevTime, m_CurrentTime, m_ColliderWindows);
    m_PrevTime = m_CurrentTime;
    if (m_pOwner) {
        m_pOwner->SetAnyAttackJustWindow(anyJust);
    }

    // エフェクト再生更新
    for (auto& eff : m_EffectWindows)
    {
        if (!eff.IsPlayed && m_CurrentTime >= eff.Start)
        {
            // エフェクトを再生
            if (m_pOwner)
            {
                m_pOwner->PlayEffect(eff.EffectName, eff.Offset, eff.Scale);
                (eff.EffectName, eff.Offset, eff.Scale);
            }
            eff.IsPlayed = true;
        }
    }

    // 動き更新 (イージング適用).
    for (auto& mv : m_MovementWindows)
    {
        if (!mv.IsAct && m_CurrentTime >= mv.Start)
        {
            mv.IsAct = true;
        }

        if (mv.IsAct && m_CurrentTime < (mv.Start + mv.Duration))
        {
            // 現在位置とターゲット位置を取得.
            DirectX::XMFLOAT3 pos = m_pOwner->GetPosition();
            DirectX::XMFLOAT3 pos_target = m_pOwner->m_PlayerPos;

            // 初回アクティブ化時の初期化.
            if (!mv.Initialized)
            {
                mv.StartPos = pos;
                // 方向はターゲット位置の方を向く(XZ 平面).
                DirectX::XMVECTOR pos_v = DirectX::XMLoadFloat3(&pos);
                DirectX::XMVECTOR target_v = DirectX::XMLoadFloat3(&pos_target);
                DirectX::XMVECTOR dir_v = DirectX::XMVectorSubtract(target_v, pos_v);
                // 正規化ができない場合は移動しない.
                float len = DirectX::XMVectorGetX(DirectX::XMVector3Length(dir_v));
                DirectX::XMFLOAT3 endPos = mv.StartPos;
                if (len > EPSILON_E6)
                {
                    DirectX::XMVECTOR dir_n = DirectX::XMVector3Normalize(dir_v);
                    DirectX::XMVECTOR move_amount_v = DirectX::XMVectorScale(dir_n, mv.Speed * mv.Duration * mv.Distance);
                    DirectX::XMVECTOR end_v = DirectX::XMVectorAdd(pos_v, move_amount_v);
                    DirectX::XMStoreFloat3(&endPos, end_v);
                    endPos = ComputeMovementEndPos(mv, mv.StartPos, pos_target);
                }
                mv.EndPos = endPos;
                // 初期化フラグと LastEasedPos を設定.
                mv.Initialized = true;
                mv.LastEasedPos = mv.StartPos;

                // 追加: 方向オフセットが有効なら、プレイヤー方向に対して指定角度だけ回転させた方向を EndPos とする
                if (mv.UseDirectionOffset)
                {
                    // プレイヤー方向ベクトル（XZ）
                    DirectX::XMVECTOR pos_v = DirectX::XMLoadFloat3(&mv.StartPos);
                    DirectX::XMVECTOR target_v = DirectX::XMLoadFloat3(&pos_target);
                    DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(target_v, pos_v);
                    // XZ 平面に投影
                    DirectX::XMFLOAT3 fdir; DirectX::XMStoreFloat3(&fdir, dir);
                    DirectX::XMFLOAT3 dir_xz = { fdir.x, 0.0f, fdir.z };
                    DirectX::XMVECTOR v_dir_xz = DirectX::XMLoadFloat3(&dir_xz);
                    float len_dir = DirectX::XMVectorGetX(DirectX::XMVector3Length(v_dir_xz));
                    if (len_dir > EPSILON_E6)
                    {
                        DirectX::XMVECTOR dir_n = DirectX::XMVector3Normalize(v_dir_xz);
                        // 回転角をラジアンで作成
                        float rad = DirectX::XMConvertToRadians(mv.DirectionOffsetDeg);
                        DirectX::XMVECTOR rot = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0,1,0,0), rad);
                        DirectX::XMVECTOR rotated = DirectX::XMVector3Rotate(dir_n, rot);
                        // EndPos = StartPos + rotated * (Speed * Duration * Distance)
                        DirectX::XMVECTOR move_amount = DirectX::XMVectorScale(rotated, mv.Speed * mv.Duration * mv.Distance);
                        DirectX::XMVECTOR end_pos_v = DirectX::XMVectorAdd(pos_v, move_amount);
                        DirectX::XMFLOAT3 fend; DirectX::XMStoreFloat3(&fend, end_pos_v);
                        mv.EndPos = fend;
                    }
                }

                // 回転.
                float dx = mv.EndPos.x - mv.StartPos.x;
                float dz = mv.EndPos.z - mv.StartPos.z;
                float target_rad = std::atan2f(dx, dz);
                const float RAD_TO_DEG = 180.0f / 3.14159f;
                float target_deg = target_rad * RAD_TO_DEG;
                m_pOwner->RotetToTarget(target_deg, 180.0f);
            }

            // 経過時間 (ウィンドウ内).
            float t = m_CurrentTime - mv.Start;

            // イージング.
            DirectX::XMFLOAT3 easedPos;
            MyEasing::UpdateEasing(mv.EasingType, t, mv.Duration, mv.StartPos, mv.EndPos, easedPos);

            // 前フレームからの差分を加算して移動を行う.
            DirectX::XMFLOAT3 delta = {};
            delta.x = easedPos.x - mv.LastEasedPos.x;
            delta.y = easedPos.y - mv.LastEasedPos.y;
            delta.z = easedPos.z - mv.LastEasedPos.z;
            m_pOwner->AddPosition(delta);

            mv.LastEasedPos = easedPos;
        }
        else if (mv.IsAct && m_CurrentTime >= (mv.Start + mv.Duration))
        {
            // ウィンドウ終了時に最終位置へ補正して終了フラグをクリア.
            if (mv.Initialized)
            {
                DirectX::XMFLOAT3 finalDelta;
                finalDelta.x = mv.EndPos.x - mv.LastEasedPos.x;
                finalDelta.y = mv.EndPos.y - mv.LastEasedPos.y;
                finalDelta.z = mv.EndPos.z - mv.LastEasedPos.z;
                m_pOwner->AddPosition(finalDelta);
            }
            mv.IsAct = false;
            mv.Initialized = false;
        }
    }
}

void BossAttackStateBase::Exit()
{
    for (auto& w : m_ColliderWindows) { w.Reset(); }
    for (auto& w : m_MovementWindows) { w.Reset(); }

    m_MovementWindows.clear();
    m_ColliderWindows.clear();
};

void BossAttackStateBase::DrawImGui()
{
    // ImGui を用いた攻撃パラメータ編集パネル
    // このパネルからダメージ、当たり判定サイズ、フェーズ毎のアニメ速度
    // などのパラメータを調整できます。実行時のみの編集用です。
#if _DEBUG
	if (!ImGui::Begin(IMGUI_JP("Boss Attack State"))) {
		ImGui::End();
		return;
	}

    ImGui::Checkbox(IMGUI_JP("ストップ"), &m_IsDebugStop);

    // 攻撃力
    ImGui::SliderFloat(IMGUI_JP("攻撃力"), &m_AttackAmount, 0.0f, 9999.0f);

    // 当たり判定サイズ
    ImGui::SliderFloat(IMGUI_JP("判定半径"), &m_ColliderWidth, 0.0f, 100.0f);
    ImGui::SliderFloat(IMGUI_JP("判定高さ"), &m_ColliderHeight, 0.0f, 100.0f);

    // ボーン名表示
    ImGui::Text(IMGUI_JP("追従ボーン: %s"), m_AttackBoneName.empty() ? "None" : m_AttackBoneName.c_str());

    // ステートの時間設定
    ImGui::SliderFloat(IMGUI_JP("ステート全体の長さ"), &m_EndTime, 0.0f, 10.0f);
    ImGui::SliderFloat(IMGUI_JP("チャージ時間"), &m_ChargeTime, 0.0f, 10.0f);
    ImGui::SliderFloat(IMGUI_JP("攻撃時間"), &m_AttackTime, 0.0f, 10.0f);

	ImGui::Separator();
	ImGui::Text(IMGUI_JP("アニメーション速度"));
    ImGui::SliderFloat("Anim Speed - Charge", &m_AnimSpeedCharge, 0.0f, 3.0f);
    ImGui::SliderFloat("Anim Speed - Attack", &m_AnimSpeedAttack, 0.1f, 10.0f);
    ImGui::SliderFloat("Anim Speed - Exit", &m_AnimSpeedExit, 0.1f, 3.0f);

    const float attackStart = m_ChargeTime;
    const float attackEnd = (m_AttackTime > 0.0f) ? m_AttackTime : m_EndTime;
    const char* phase = "Unknown";
    if (m_CurrentTime < attackStart && m_ChargeTime > 0.0f) phase = IMGUI_JP("溜め");
    else if (m_CurrentTime >= attackStart && m_CurrentTime < attackEnd) phase = IMGUI_JP("攻撃");
    else phase = IMGUI_JP("余波");

    ImGui::Separator();
    ImGui::Text(IMGUI_JP("現在の経過時間: %.3f 秒"), m_CurrentTime);
    ImGui::Text(IMGUI_JP("現在フェーズ: %s"), phase);

    // 遷移制御: 各フェーズでアニメ終了か時間かで遷移するか選択
    ImGui::Separator();
    ImGui::Text(IMGUI_JP("遷移条件"));
    ImGui::Checkbox(IMGUI_JP("溜め(Charge)フェーズ: アニメ終了で遷移"), &m_TransitionOnAnimEnd_Charge);
    ImGui::Checkbox(IMGUI_JP("攻撃(Attack)フェーズ: アニメ終了で遷移"), &m_TransitionOnAnimEnd_Attack);
    ImGui::Checkbox(IMGUI_JP("余韻(Exit)フェーズ: アニメ終了で遷移"), &m_TransitionOnAnimEnd_Exit);

    // コライダーウィンドウ編集
    ImGui::Separator();
    ImGui::Text(IMGUI_JP("当たり判定ウィンドウ"));
    
    // デバッグ: ボスの回転情報を表示
    if (m_pOwner)
    {
        const auto& quat = m_pOwner->GetTransform()->Quaternion;
        ImGui::Text(IMGUI_JP("Boss Quat: (%.3f, %.3f, %.3f, %.3f)"), quat.x, quat.y, quat.z, quat.w);
        DirectX::XMFLOAT3 forward = m_pOwner->GetTransform()->GetForward();
        ImGui::Text(IMGUI_JP("Boss Forward: (%.3f, %.3f, %.3f)"), forward.x, forward.y, forward.z);
    }
    
    for (size_t i = 0; i < m_ColliderWindows.size(); ++i)
    {
        auto &w = m_ColliderWindows[i];
        ImGui::PushID((int)i);
        // 開始時刻／継続時間
        ImGui::DragFloat(IMGUI_JP("開始時刻 (秒)"), &w.Start, 0.01f, 0.0f, m_EndTime);
        ImGui::DragFloat(IMGUI_JP("継続時間 (秒)"), &w.Duration, 0.01f, 0.0f, m_EndTime);
        // オフセット編集（Bossのローカル座標系: X=横, Y=上, Z=前方）
        float offset[3] = { w.Offset.x, w.Offset.y, w.Offset.z };
        if (ImGui::DragFloat3(IMGUI_JP("オフセット (横/上/前)"), offset, 0.5f, -500.0f, 500.0f)) {
            w.Offset.x = offset[0];
            w.Offset.y = offset[1];
            w.Offset.z = offset[2];
        }
        
        // デバッグ: 回転後のオフセットを表示
        if (m_pOwner && w.IsAct && !w.IsEnd)
        {
            float yaw = m_pOwner->GetTransform()->Rotation.y;

            // XMMatrixRotationY と一致: (-sin(y), 0, cos(y))
            DirectX::XMVECTOR v_forward = DirectX::XMVectorSet(-sinf(yaw), 0.0f, cosf(yaw), 0.0f);
            DirectX::XMVECTOR v_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            DirectX::XMVECTOR v_right = DirectX::XMVector3Cross(v_up, v_forward);

            DirectX::XMVECTOR v_worldOffset = DirectX::XMVectorAdd(
                DirectX::XMVectorAdd(
                    DirectX::XMVectorScale(v_right, w.Offset.x),
                    DirectX::XMVectorScale(v_up, w.Offset.y)
                ),
                DirectX::XMVectorScale(v_forward, w.Offset.z)
            );

            DirectX::XMFLOAT3 rotated;
            DirectX::XMStoreFloat3(&rotated, v_worldOffset);
            DirectX::XMFLOAT3 fwd;
            DirectX::XMStoreFloat3(&fwd, v_forward);

            ImGui::Text(IMGUI_JP("Yaw(rad): %.3f (deg: %.1f)"), yaw, yaw * 180.0f / 3.14159f);
            ImGui::Text(IMGUI_JP("計算された前方: (%.2f, %.2f, %.2f)"), fwd.x, fwd.y, fwd.z);
            ImGui::Text(IMGUI_JP("回転後オフセット: (%.2f, %.2f, %.2f)"), rotated.x, rotated.y, rotated.z);
        }
        
        // ボーン名編集
        char buf[128];
#ifdef _MSC_VER
        strncpy_s(buf, sizeof(buf), w.BoneName.c_str(), _TRUNCATE);
#else
        std::strncpy(buf, w.BoneName.c_str(), sizeof(buf)); buf[sizeof(buf)-1] = '\0';
#endif
        if (ImGui::InputText(IMGUI_JP("ボーン名"), buf, sizeof(buf))) {
            w.BoneName = std::string(buf);
        }
        // ジャストタイム編集
        ImGui::DragFloat(IMGUI_JP("ジャストタイム (秒)"), &w.JustTime, 0.01f, 0.0f, 5.0f);
        ImGui::SameLine();
        ImGui::Checkbox(IMGUI_JP("ワンショット"), &w.OneShot);
        // ジャスト判定状態表示
        ImGui::Text(IMGUI_JP("ジャスト判定: %s"), w.IsJustWindow ? "ON" : "OFF");
        ImGui::SameLine();
        if (ImGui::Button(IMGUI_JP("削除"))) { m_ColliderWindows.erase(m_ColliderWindows.begin() + i); ImGui::PopID(); break; }
        ImGui::PopID();
        ImGui::Separator();
    }
    if (ImGui::Button(IMGUI_JP("ウィンドウを追加"))) { ColliderWindow nw; nw.Start = 0.0f; nw.Duration = 0.1f; nw.BoneName = ""; m_ColliderWindows.push_back(nw); }

    // 移動ウィンドウ編集
    ImGui::Separator();
    ImGui::Text(IMGUI_JP("移動ウィンドウ"));
    for (size_t i = 0; i < m_MovementWindows.size(); ++i)
    {
        auto &mv = m_MovementWindows[i];
        ImGui::PushID((int)i + 1000);
        ImGui::DragFloat(IMGUI_JP("開始時刻 (秒)"), &mv.Start, 0.01f, 0.0f, m_EndTime);
        ImGui::SameLine();
        ImGui::DragFloat(IMGUI_JP("継続時間 (秒)"), &mv.Duration, 0.01f, 0.0f, m_EndTime);
        ImGui::SameLine();
        ImGui::DragFloat(IMGUI_JP("速度"), &mv.Speed, 0.1f, 0.0f, 1000.0f);
        //ImGui::SameLine(); 
        // Easing 選択
        int easing_idx = static_cast<int>(mv.EasingType);
        const char* easing_names[] = {
            "Liner","InSine","OutSine","InOutSine","InQuad","OutQuad","InOutQuad",
            "InCubic","OutCubic","InOutCubic","InQuart","OutQuart","InOutQuart",
            "InQuint","OutQuint","InOutQuint","InExpo","OutExpo","InOutExpo",
            "InCirc","OutCirc","InOutCirc","InBack","OutBack","InOutBack",
            "InElastic","OutElastic","InOutElastic","InBounce","OutBounce","InOutBounce"
        };
        if (ImGui::Combo(IMGUI_JP("イージング"), &easing_idx, easing_names, IM_ARRAYSIZE(easing_names))) {
            mv.EasingType = static_cast<MyEasing::Type>(easing_idx);
        }
        ImGui::SameLine();
        // 移動量係数の編集
        ImGui::PushItemWidth(100);
        ImGui::DragFloat(IMGUI_JP("距離"), &mv.Distance, 0.01f, 0.0f, 1000.0f);
        ImGui::PushItemWidth(100);
        if (ImGui::Checkbox(IMGUI_JP("方向オフセットを使用"), &mv.UseDirectionOffset)) {
            mv.Initialized = false;
            mv.LastEasedPos = mv.StartPos;
        }
        ImGui::PushItemWidth(80);
        ImGui::DragFloat(IMGUI_JP("方向オフセット(deg)"), &mv.DirectionOffsetDeg, 1.0f, -180.0f, 180.0f);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button(IMGUI_JP("削除"))) { m_MovementWindows.erase(m_MovementWindows.begin() + i); ImGui::PopID(); break; }
        ImGui::PopID();
        ImGui::SameLine();
    }
    if (ImGui::Button(IMGUI_JP("移動ウィンドウを追加"))) { MovementWindow mv; mv.Start = 0.0f; mv.Duration = 0.2f; mv.Speed = 10.0f; m_MovementWindows.push_back(mv); }

    // Load / Save ボタン
    if (ImGui::Button(IMGUI_JP("Load"))) {
        try {
            LoadSettings();
        }
        catch (...) {}
    }
    ImGui::SameLine();
    if (ImGui::Button(IMGUI_JP("Save"))) {
        try {
            SaveSettings();
        }
        catch (...) {}
    }
    if (ImGui::Button(IMGUI_JP("ReEtert"))) {
        try {
            this->Enter();
        }
        catch (...) {}
    }

    ImGui::End();
#endif
}

void BossAttackStateBase::LoadSettings()
{
    auto filePath = GetSettingsFileName();
    if (!filePath.is_absolute()) {
        filePath = std::filesystem::current_path() / "Data" / "Json" / "Boss" / filePath;
    }
    if (!std::filesystem::exists(filePath)) return;
    // 読み込み前に既存ウィンドウ設定をクリアして重複を防ぐ
    m_ColliderWindows.clear();
    m_MovementWindows.clear();
    json j = FileManager::JsonLoad(filePath);
    if (j.contains("m_AttackAmount")) m_AttackAmount = j["m_AttackAmount"].get<float>();
    if (j.contains("m_ColliderWidth")) m_ColliderWidth = j["m_ColliderWidth"].get<float>();
    if (j.contains("m_ColliderHeight")) m_ColliderHeight = j["m_ColliderHeight"].get<float>();
    if (j.contains("m_EndTime")) m_EndTime = j["m_EndTime"].get<float>();
    if (j.contains("m_AttackTime")) m_AttackTime = j["m_AttackTime"].get<float>();
    if (j.contains("m_ChargeTime")) m_ChargeTime = j["m_ChargeTime"].get<float>();
    if (j.contains("m_TransitionOnAnimEnd_Charge")) m_TransitionOnAnimEnd_Charge = j["m_TransitionOnAnimEnd_Charge"].get<bool>();
    if (j.contains("m_TransitionOnAnimEnd_Attack")) m_TransitionOnAnimEnd_Attack = j["m_TransitionOnAnimEnd_Attack"].get<bool>();
    if (j.contains("m_TransitionOnAnimEnd_Exit")) m_TransitionOnAnimEnd_Exit = j["m_TransitionOnAnimEnd_Exit"].get<bool>();
    if (j.contains("m_AttackBoneName")) m_AttackBoneName = j["m_AttackBoneName"].get<std::string>();
    if (j.contains("AnimSpeeds") && j["AnimSpeeds"].is_object()) {
        auto& a = j["AnimSpeeds"];
        if (a.contains("Charge")) m_AnimSpeedCharge = a["Charge"].get<float>();
        if (a.contains("Attack")) m_AnimSpeedAttack = a["Attack"].get<float>();
        if (a.contains("Exit")) m_AnimSpeedExit = a["Exit"].get<float>();
    }
    if (j.contains("ColliderWindows") && j["ColliderWindows"].is_array()) {
        m_ColliderWindows.clear();
        for (const auto& entry : j["ColliderWindows"]) {
            if (entry.contains("start") && entry.contains("duration")) {
                ColliderWindow w;
                w.Start = entry["start"].get<float>();
                w.Duration = entry["duration"].get<float>();
                if (entry.contains("BoneName")) w.BoneName = entry["BoneName"].get<std::string>();
                // オフセット読み込み
                if (entry.contains("offset") && entry["offset"].is_array() && entry["offset"].size() >= 3) {
                    w.Offset.x = entry["offset"][0].get<float>();
                    w.Offset.y = entry["offset"][1].get<float>();
                    w.Offset.z = entry["offset"][2].get<float>();
                }
                // ジャストタイム読み込み
                if (entry.contains("justTime")) {
                    w.JustTime = entry["justTime"].get<float>();
                }
                if (entry.contains("oneShot")) {
                    w.OneShot = entry["oneShot"].get<bool>();
                }
                m_ColliderWindows.push_back(w);
            }
        }
    }
    if (j.contains("MovementWindows") && j["MovementWindows"].is_array()) {
        m_MovementWindows.clear();
        for (const auto& entry : j["MovementWindows"]) {
            if (entry.contains("start") && entry.contains("duration") && entry.contains("speed")) {
                MovementWindow mv;
                mv.Start = entry["start"].get<float>();
                mv.Duration = entry["duration"].get<float>();
                mv.Speed = entry["speed"].get<float>();
                if (entry.contains("easing")) {
                    int et = entry["easing"].get<int>();
                    mv.EasingType = static_cast<MyEasing::Type>(et);
                }
                if (entry.contains("distance")) {
                    mv.Distance = entry["distance"].get<float>();
                }
                // 互換性: 旧フォーマットの "reverse" を読めるようにする
                if (entry.contains("use_direction_offset")) {
                    mv.UseDirectionOffset = entry["use_direction_offset"].get<bool>();
                }
                if (entry.contains("direction_offset_deg")) {
                    mv.DirectionOffsetDeg = entry["direction_offset_deg"].get<float>();
                }
                if (entry.contains("reverse")) {
                    bool rev = entry["reverse"].get<bool>();
                    if (rev) {
                        mv.UseDirectionOffset = true;
                        // 旧 reverse の意味は 180deg 回転に等しい
                        mv.DirectionOffsetDeg = 180.0f;
                    }
                }
                m_MovementWindows.push_back(mv);
            }
        }
    }
}

void BossAttackStateBase::SaveSettings() const
{
    auto filePath = GetSettingsFileName();
    if (!filePath.is_absolute()) {
        auto dir = std::filesystem::current_path() / "Data" / "Json" / "Boss";
        std::filesystem::create_directories(dir);
        filePath = dir / filePath;
    }

    json j;
    j["m_AttackAmount"] = m_AttackAmount;
    j["m_ColliderWidth"] = m_ColliderWidth;
    j["m_ColliderHeight"] = m_ColliderHeight;
    j["m_EndTime"] = m_EndTime;
    j["m_AttackTime"] = m_AttackTime;
    j["m_ChargeTime"] = m_ChargeTime;
    j["m_TransitionOnAnimEnd_Charge"] = m_TransitionOnAnimEnd_Charge;
    j["m_TransitionOnAnimEnd_Attack"] = m_TransitionOnAnimEnd_Attack;
    j["m_TransitionOnAnimEnd_Exit"] = m_TransitionOnAnimEnd_Exit;
    j["m_AttackBoneName"] = m_AttackBoneName;
    j["AnimSpeeds"] = json::object();
    j["AnimSpeeds"]["Charge"] = m_AnimSpeedCharge;
    j["AnimSpeeds"]["Attack"] = m_AnimSpeedAttack;
    j["AnimSpeeds"]["Exit"] = m_AnimSpeedExit;

    j["ColliderWindows"] = json::array();
    for (const auto& w : m_ColliderWindows) {
        json e;
        e["start"] = w.Start;
        e["duration"] = w.Duration;
        e["BoneName"] = w.BoneName;
        e["offset"] = { w.Offset.x, w.Offset.y, w.Offset.z };
        e["justTime"] = w.JustTime;
        e["oneShot"] = w.OneShot;
        j["ColliderWindows"].push_back(e);
    }

    j["MovementWindows"] = json::array();
    for (const auto& mv : m_MovementWindows) {
        json e;
        e["start"] = mv.Start;
        e["duration"] = mv.Duration;
        e["speed"] = mv.Speed;
        e["use_direction_offset"] = mv.UseDirectionOffset;
        e["direction_offset_deg"] = mv.DirectionOffsetDeg;
        e["easing"] = static_cast<int>(mv.EasingType);
        e["distance"] = mv.Distance;
        j["MovementWindows"].push_back(e);
    }

    FileManager::JsonSave(filePath, j);
}

nlohmann::json BossAttackStateBase::SerializeSettings() const
{
    nlohmann::json j;
    j["m_AttackAmount"] = m_AttackAmount;
    j["m_ColliderWidth"] = m_ColliderWidth;
    j["m_ColliderHeight"] = m_ColliderHeight;
    j["m_EndTime"] = m_EndTime;
    j["m_AttackTime"] = m_AttackTime;
    j["m_ChargeTime"] = m_ChargeTime;
    j["m_TransitionOnAnimEnd_Charge"] = m_TransitionOnAnimEnd_Charge;
    j["m_TransitionOnAnimEnd_Attack"] = m_TransitionOnAnimEnd_Attack;
    j["m_TransitionOnAnimEnd_Exit"] = m_TransitionOnAnimEnd_Exit;
    j["m_AttackBoneName"] = m_AttackBoneName;
    j["AnimSpeeds"] = nlohmann::json::object();
    j["AnimSpeeds"]["Charge"] = m_AnimSpeedCharge;
    j["AnimSpeeds"]["Attack"] = m_AnimSpeedAttack;
    j["AnimSpeeds"]["Exit"] = m_AnimSpeedExit;

    j["ColliderWindows"] = nlohmann::json::array();
    for (const auto& w : m_ColliderWindows) {
        nlohmann::json e;
        e["start"] = w.Start;
        e["duration"] = w.Duration;
        e["BoneName"] = w.BoneName;
        // 保存時にオフセットとジャストタイムも書き出す
        e["offset"] = { w.Offset.x, w.Offset.y, w.Offset.z };
        e["justTime"] = w.JustTime;
        e["oneShot"] = w.OneShot;
        j["ColliderWindows"].push_back(e);
    }

    j["MovementWindows"] = nlohmann::json::array();
    for (const auto& mv : m_MovementWindows) {
        nlohmann::json e;
        e["start"] = mv.Start;
        e["duration"] = mv.Duration;
        e["speed"] = mv.Speed;
        e["distance"] = mv.Distance;
        e["use_direction_offset"] = mv.UseDirectionOffset;
        e["direction_offset_deg"] = mv.DirectionOffsetDeg;
        e["easing"] = static_cast<int>(mv.EasingType);
        j["MovementWindows"].push_back(e);
    }

    return j;
}

std::pair<Boss::enBossAnim, float> BossAttackStateBase::GetParryAnimPair()
{
    return { Boss::enBossAnim::none, 0.0f };
}

// 次にアクティブなる予定の当たり判定のIsEndをtrueにする.
void BossAttackStateBase::SetNextAttackCansel()
{
    for (auto& window : m_ColliderWindows)
    {
        // まだアクティブになっていない、かつ開始時刻が現在時刻以降のウィンドウをキャンセル
        if (!window.IsAct && !window.IsEnd && window.Start >= m_CurrentTime)
        {
            window.IsEnd = true;
            return;
        }
    }
}

