#include "LockOnCamera.h"

#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/Player.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/02_Boss/Boss.h"
#include "Game/04_Time/Time.h"
#include "System/Singleton/ImGui/CImGuiManager.h"
#include "System/Utility/FileManager/FileManager.h"
#include "System/Singleton/CombatCoordinator/CombatCoordinator.h"

LockOnCamera::LockOnCamera(const Player& player, const Boss& target)
    : m_rPlayer(player)
    , m_rTarget(target)
    , m_FollowSpeed(3.15f)
    , m_HeightOffset(5.0f)
    , m_LookOffset(4.5f)
    , m_LookLerp(0.7f)
    , m_CameraMode(eCameraMode::Normal)
    , m_ParryTime(0.0f)
{
    m_Distance = 15.0f; // カメラからプレイヤーまでの距離.
    try { LoadSettings(); } catch (...) {}

    // 初期FOVをCameraBaseへ適用
    SetFOV(m_CurrentFOV);
}

void LockOnCamera::Update()
{
    float dt = Time::GetInstance().GetDeltaTime();

#if _DEBUG
	//if (ImGui::Begin(IMGUI_JP("LockOnCamera Debug")))
	//{
	//	// 距離は明示的ターゲット使用時は無効化
	//	if (!m_UseExplicitTarget) {
	//		ImGui::DragFloat(IMGUI_JP("距離"), &m_Distance, 0.1f, 1.0f, 50.0f);
	//	} else {
	//		ImGui::Text(IMGUI_JP("距離: 明示的ターゲット使用中で無効"));
	//	}
	//	ImGui::DragFloat(IMGUI_JP("追従速度"), &m_FollowSpeed, 0.05f, 0.1f, 10.0f);
	//	ImGui::DragFloat(IMGUI_JP("高さオフセット"), &m_HeightOffset, 0.1f, 0.0f, 20.0f);
	//	ImGui::DragFloat(IMGUI_JP("注視点高さ"), &m_LookOffset, 0.1f, 0.0f, 20.0f);
	//	ImGui::DragFloat(IMGUI_JP("注視点補間"), &m_LookLerp, 0.01f, 0.0f, 1.0f);

	//	// FOV controls (display as degrees)
	//	float defaultFovDeg = DirectX::XMConvertToDegrees(m_DefaultFOV);
	//	float parryFovDeg = DirectX::XMConvertToDegrees(m_ParryFOV);
	//	if (ImGui::DragFloat(IMGUI_JP("デフォルトFOV(度)"), &defaultFovDeg, 0.1f, 10.0f, 170.0f))
	//	{
	//		m_DefaultFOV = DirectX::XMConvertToRadians(defaultFovDeg);
	//		if (m_CameraMode != eCameraMode::Parry)
	//		{
	//			m_CurrentFOV = m_DefaultFOV;
	//			SetFOV(m_CurrentFOV);
	//		}
	//	}
	//	if (ImGui::DragFloat(IMGUI_JP("パリィFOV(度)"), &parryFovDeg, 0.1f, 10.0f, 170.0f))
	//	{
	//		m_ParryFOV = DirectX::XMConvertToRadians(parryFovDeg);
	//	}

	//	ImGui::Separator();
	//	ImGui::Text(IMGUI_JP("パリィカメラ設定"));
	//	ImGui::DragFloat(IMGUI_JP("パリィ: 到達時間"), &m_ParryPhaseInDuration, 0.01f, 0.01f, 3.0f);
	//	ImGui::DragFloat(IMGUI_JP("パリィ: 滞在時間"), &m_ParryHoldDuration, 0.01f, 0.0f, 3.0f);
	//	ImGui::DragFloat(IMGUI_JP("パリィ: 戻る時間"), &m_ParryOutDuration, 0.01f, 0.01f, 3.0f);
	//	ImGui::DragFloat(IMGUI_JP("パリィ時高さ"), &m_ParryHeightOffset, 0.1f, -5.0f, 10.0f);
	//	ImGui::DragFloat(IMGUI_JP("パリィ時注視点高さ"), &m_ParryLookOffset, 0.1f, 0.0f, 15.0f);
	//	ImGui::DragFloat(IMGUI_JP("パリィ時距離"), &m_ParryDistance, 0.1f, 1.0f, 20.0f);
	//	ImGui::DragFloat(IMGUI_JP("パリィ: 横オフセット"), &m_ParryHorizontalOffset, 0.01f, -10.0f, 10.0f);
	//	ImGui::DragFloat(IMGUI_JP("パリィ: 前方オフセット"), &m_ParryForwardOffset, 0.01f, -10.0f, 10.0f);

	//	ImGui::Separator();
	//	ImGui::Text(IMGUI_JP("明示的ターゲット"));
	//	ImGui::Checkbox(IMGUI_JP("明示的な目標/注視点を使用"), &m_UseExplicitTarget);
	//	float camOff[3] = { m_ExplicitCameraPosOffset.x, m_ExplicitCameraPosOffset.y, m_ExplicitCameraPosOffset.z };
	//	if (ImGui::DragFloat3(IMGUI_JP("カメラ位置オフセット (相対: x,y,z)"), camOff, 0.1f)) {
	//		m_ExplicitCameraPosOffset = { camOff[0], camOff[1], camOff[2] };
	//	}
	//	float lookOff[3] = { m_ExplicitLookPosOffset.x, m_ExplicitLookPosOffset.y, m_ExplicitLookPosOffset.z };
	//	if (ImGui::DragFloat3(IMGUI_JP("注視点オフセット (相対: x,y,z)"), lookOff, 0.1f)) {
	//		m_ExplicitLookPosOffset = { lookOff[0], lookOff[1], lookOff[2] };
	//	}

	//	if (ImGui::Button(IMGUI_JP("パリィカメラテスト"))) {
	//		StartParryCamera();
	//	}

	//	if (ImGui::Button(IMGUI_JP("Load"))) {
	//		try { LoadSettings(); } catch (...) {}
	//	}
	//	ImGui::SameLine();
	//	if (ImGui::Button(IMGUI_JP("Save"))) {
	//		try { SaveSettings(); } catch (...) {}
	//	}
	//	ImGui::End();
	//}
#endif

    // カメラモードに応じて更新処理を分岐.
    switch (m_CameraMode)
    {
    case eCameraMode::Normal:
        UpdateNormalCamera(dt);
        break;
    case eCameraMode::Parry:
        UpdateParryCamera(dt);
        break;
    case eCameraMode::Special:
        UpdateSpecialCamera(dt);
        break;
    }

	ViewAndProjectionUpdate();
}

void LockOnCamera::UpdateNormalCamera(float dt)
{
	const auto& player = m_rPlayer.get();
	const auto& target = m_rTarget.get();

	// 1. 各座標を取得.
	DirectX::XMVECTOR vPlayerPos = DirectX::XMLoadFloat3(&player.GetPosition());
	DirectX::XMVECTOR vBossPos = DirectX::XMLoadFloat3(&target.GetPosition());

	// 2. プレイヤーとボスの距離を算出（高さ調整や安定化に使用）.
	DirectX::XMVECTOR vDiffPB = DirectX::XMVectorSubtract(vPlayerPos, vBossPos);
	float distToBoss = DirectX::XMVectorGetX(DirectX::XMVector3Length(vDiffPB));

	// --- 改善点1: 至近距離でのガクつき防止 ---
	// 距離が近すぎるとき、方向ベクトルの計算を安定させる.
	DirectX::XMVECTOR vToPlayerDir;
	if (distToBoss < 0.1f) {
		DirectX::XMFLOAT3 playerForward = player.GetTransform()->GetForward();
		// XMFLOAT3をXMVECTORにロードしてから計算.
		vToPlayerDir = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&playerForward), -1.0f);
	}
	else {
		vToPlayerDir = DirectX::XMVector3Normalize(DirectX::XMVectorSetY(vDiffPB, 0.0f));
	}

	// --- 改善点2: 注視点を「ボスとプレイヤーの間」に設定 ---
	// これによりボスが画面中央に固定されすぎず、自分も映りやすくなる.
	DirectX::XMVECTOR vMidPoint = DirectX::XMVectorLerp(vPlayerPos, vBossPos, m_LookLerp);
	vMidPoint = DirectX::XMVectorAdd(vMidPoint, DirectX::XMVectorSet(0.0f, m_LookOffset, 0.0f, 0.0f));

	// --- 改善点3: 近接時のカメラ高補正 ---
	// 近づくほど少しカメラを高くし、見下ろす角度を強くすることでキャラ被りを防ぐ.
	float heightScaling = (1.0f / (distToBoss + 1.0f)) * 2.0f; // 近いほど加算値が増える.
	float finalHeight = m_HeightOffset + heightScaling;

	// 3. 理想のカメラ位置を計算.
	DirectX::XMVECTOR vIdealPos = DirectX::XMVectorAdd(vPlayerPos, DirectX::XMVectorScale(vToPlayerDir, m_Distance));
	vIdealPos = DirectX::XMVectorAdd(vIdealPos, DirectX::XMVectorSet(0.0f, finalHeight, 0.0f, 0.0f));

	// 4. 現在の位置から理想の位置へ線形補間 (Lerp).
	DirectX::XMVECTOR vCurrentPos = DirectX::XMLoadFloat3(&m_spTransform.Position);
	float lerpFactor = std::clamp(m_FollowSpeed * dt, 0.0f, 1.0f);
	DirectX::XMVECTOR vNextPos = DirectX::XMVectorLerp(vCurrentPos, vIdealPos, lerpFactor);
	DirectX::XMStoreFloat3(&m_spTransform.Position, vNextPos);

	// 5. 注視点の適用（デッドゾーン計算を入れる場合はここで行う）.
	DirectX::XMStoreFloat3(&m_LookPos, vMidPoint);

	// --- 以下、前方ベクトル・右方ベクトル・行列更新（共通） ---
	DirectX::XMVECTOR vForward = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(vMidPoint, vNextPos));
	DirectX::XMStoreFloat3(&m_ForwardVec, vForward);

	DirectX::XMVECTOR vUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR vRight = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vUp, vForward));
	DirectX::XMStoreFloat3(&m_RightVec, vRight);

	DirectX::XMMATRIX lookAtMat = DirectX::XMMatrixLookToLH(vNextPos, vForward, vUp);
	DirectX::XMVECTOR v_Quaternion = DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixTranspose(lookAtMat));
	DirectX::XMStoreFloat4(&m_spTransform.Quaternion, v_Quaternion);
}

void LockOnCamera::UpdateParryCamera(float dt)
{
    m_ParryTime += dt;
    // 演出終了判定: 合計フェーズ時間を超えたら終了
    float totalDuration = m_ParryPhaseInDuration + m_ParryHoldDuration + m_ParryOutDuration;
    if (m_ParryTime >= totalDuration)
    {
        EndParryCamera();
        UpdateNormalCamera(dt);
        return;
    }

    const auto& player = m_rPlayer.get();
    const auto& target = m_rTarget.get();

    // totalDuration は上で計算済みなのでここでは再計算しない
    // (全体進行率 t はフェーズごとの計算で不要)
    static bool isSlow = false;
    // フェーズ別の進行率を計算
    float easedT = 0.0f;
    if (m_ParryTime <= m_ParryPhaseInDuration)
    {
        // フェーズ入力: 通常→パリィ位置へ（EaseOutQuad）
        float phaseT = m_ParryTime / m_ParryPhaseInDuration; // 0..1
        easedT = 1.0f - (1.0f - phaseT) * (1.0f - phaseT);
    }
    else if (m_ParryTime <= m_ParryPhaseInDuration + m_ParryHoldDuration)
    {
        if (!isSlow)
        {
            // 一時的にワールド時間を止める.
            Time::GetInstance().SetWorldTimeScale(0.01f, 0.016f * 5, true);
            isSlow = true;
        }
       
        // ホールドフェーズ: 完全にパリィ位置に固定
        easedT = 1.0f;
    }
    else
    {
        isSlow = false;

        // フェーズアウト: パリィ位置→通常へ（EaseInQuad）
        float outTime = m_ParryTime - (m_ParryPhaseInDuration + m_ParryHoldDuration);
        float phaseT = outTime / m_ParryOutDuration; // 0..1
        float inv = 1.0f - phaseT;
        easedT = 1.0f - (inv * inv); // ease in reversed to go from 1->0
        easedT = 1.0f - easedT; // invert to go from 1->0 over phase
    }

    // 1. 各座標を取得.
    DirectX::XMVECTOR vPlayerPos = DirectX::XMLoadFloat3(&player.GetPosition());
    DirectX::XMVECTOR vBossPos = DirectX::XMLoadFloat3(&target.GetPosition());

    // 2. プレイヤーとボスの距離を算出.
    DirectX::XMVECTOR vDiffPB = DirectX::XMVectorSubtract(vPlayerPos, vBossPos);
    float distToBoss = DirectX::XMVectorGetX(DirectX::XMVector3Length(vDiffPB));

    DirectX::XMVECTOR vToPlayerDir;
    if (distToBoss < 0.1f) {
        DirectX::XMFLOAT3 playerForward = player.GetTransform()->GetForward();
        vToPlayerDir = DirectX::XMVectorScale(DirectX::XMLoadFloat3(&playerForward), -1.0f);
    }
    else {
        vToPlayerDir = DirectX::XMVector3Normalize(DirectX::XMVectorSetY(vDiffPB, 0.0f));
    }

    // 通常パラメータとパリィパラメータを補間.
    float currentHeight = m_HeightOffset + (m_ParryHeightOffset - m_HeightOffset) * easedT;
    float currentLookOffset = m_LookOffset + (m_ParryLookOffset - m_LookOffset) * easedT;
    float currentDistance = m_Distance + (m_ParryDistance - m_Distance) * easedT;

    // FOVを補間して演出（狭める）
    m_CurrentFOV = m_DefaultFOV + (m_ParryFOV - m_DefaultFOV) * easedT;
    SetFOV(m_CurrentFOV);

    // 注視点を計算（パリィ時はボス寄りにして見上げる）.
    float parryLookLerp = m_LookLerp + (0.9f - m_LookLerp) * easedT;
    DirectX::XMVECTOR vMidPoint = DirectX::XMVectorLerp(vPlayerPos, vBossPos, parryLookLerp);
    vMidPoint = DirectX::XMVectorAdd(vMidPoint, DirectX::XMVectorSet(0.0f, currentLookOffset, 0.0f, 0.0f));

    // 3. 理想のカメラ位置を計算（低い位置から）。
    DirectX::XMVECTOR vIdealPos;
    if (m_UseExplicitTarget)
    {
        // 明示的ターゲットモード: プレイヤー位置にオフセットを加える。
        DirectX::XMVECTOR vCamOffset = DirectX::XMLoadFloat3(&m_ExplicitCameraPosOffset);
        vIdealPos = DirectX::XMVectorAdd(vPlayerPos, vCamOffset);
    }
    else
    {
        vIdealPos = DirectX::XMVectorAdd(vPlayerPos, DirectX::XMVectorScale(vToPlayerDir, currentDistance));
        // パリィ時は左右・前方へのオフセットも適用（easedT で補間）
        // 右方向ベクトルを計算（ローカル変数名が後で重複しないよう専用名を使う）
        DirectX::XMVECTOR vUpForOffset = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        DirectX::XMVECTOR vRightForOffset = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vUpForOffset, vToPlayerDir));
        // オフセットベクトル
        DirectX::XMVECTOR vHorizontalOffset = DirectX::XMVectorScale(vRightForOffset, m_ParryHorizontalOffset * easedT);
        DirectX::XMVECTOR vForwardOffset = DirectX::XMVectorScale(vToPlayerDir, m_ParryForwardOffset * easedT);
        vIdealPos = DirectX::XMVectorAdd(vIdealPos, vHorizontalOffset);
        vIdealPos = DirectX::XMVectorAdd(vIdealPos, vForwardOffset);
    }
    // 高さを追加
    vIdealPos = DirectX::XMVectorAdd(vIdealPos, DirectX::XMVectorSet(0.0f, currentHeight, 0.0f, 0.0f));

    // 4. カメラ位置を更新（パリィ中は高速追従）.
    DirectX::XMVECTOR vCurrentPos = DirectX::XMLoadFloat3(&m_spTransform.Position);
    float lerpFactor = std::clamp(m_FollowSpeed * 3.0f * dt, 0.0f, 1.0f);
    DirectX::XMVECTOR vNextPos = DirectX::XMVectorLerp(vCurrentPos, vIdealPos, lerpFactor);

    // 雪玉由来のパリィだったら、XZ成分は現在値を維持してYだけ補間する
    if (!m_ParryBySnowball && CombatCoordinator::GetInstance().WasLastParriedBySnowball())
    {
        m_ParryBySnowball = true;
        CombatCoordinator::GetInstance().ClearLastParriedFlag();
    }
    if (m_ParryBySnowball)
    {
        float curX = DirectX::XMVectorGetX(vCurrentPos);
        float curZ = DirectX::XMVectorGetZ(vCurrentPos);
        float nextY = DirectX::XMVectorGetY(vNextPos);
        vNextPos = DirectX::XMVectorSet(curX, nextY, curZ, 0.0f);
    }

    DirectX::XMStoreFloat3(&m_spTransform.Position, vNextPos);

    // 5. 注視点の適用.
    if (m_UseExplicitTarget)
    {
        // 明示的注視点モード: ボス/プレイヤー間の代わりにプレイヤー位置 + オフセットを使う
        DirectX::XMVECTOR vLookOffset = DirectX::XMLoadFloat3(&m_ExplicitLookPosOffset);
        DirectX::XMVECTOR vExplicitLook = DirectX::XMVectorAdd(vPlayerPos, vLookOffset);
        DirectX::XMStoreFloat3(&m_LookPos, vExplicitLook);
    }
    else
    {
        DirectX::XMStoreFloat3(&m_LookPos, vMidPoint);
    }

    // 前方ベクトル・右方ベクトル・行列更新.
    DirectX::XMVECTOR vForward = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(vMidPoint, vNextPos));
    DirectX::XMStoreFloat3(&m_ForwardVec, vForward);

    DirectX::XMVECTOR vUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR vRight = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vUp, vForward));
    DirectX::XMStoreFloat3(&m_RightVec, vRight);

    DirectX::XMMATRIX lookAtMat = DirectX::XMMatrixLookToLH(vNextPos, vForward, vUp);
    DirectX::XMVECTOR v_Quaternion = DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixTranspose(lookAtMat));
    DirectX::XMStoreFloat4(&m_spTransform.Quaternion, v_Quaternion);
}

void LockOnCamera::StartParryCamera()
{
    m_CameraMode = eCameraMode::Parry;
    m_ParryTime = 0.0f;
    // パリィ開始時は雪玉フラグをクリアしておく
    m_ParryBySnowball = false;
}

void LockOnCamera::EndParryCamera()
{
    m_CameraMode = eCameraMode::Normal;
    m_ParryTime = 0.0f;

    // 雪玉モードを終了
    m_ParryBySnowball = false;

    // パリィ終了時にはFOVを元に戻す
    m_CurrentFOV = m_DefaultFOV;
    SetFOV(m_CurrentFOV);
}


void LockOnCamera::StartSpecialCamera()
{
    m_CameraMode = eCameraMode::Special;
    // 必殺技開始時にカメラ位置を固定するため、現在位置を保存
    m_SpecialFixedCameraPos = m_spTransform.Position;
    m_SpecialFixedLookPos = m_LookPos;
    
    // ボスの正面方向から弧の開始角度を計算
    const auto& target = m_rTarget.get();
    DirectX::XMFLOAT3 bossPos = target.GetPosition();
    DirectX::XMFLOAT3 bossForward = target.GetTransform()->GetForward();
    m_SpecialArcAngle = std::atan2(bossForward.z, bossForward.x);
    
    // 少しFOVを広げる演出
    m_CurrentFOV = m_DefaultFOV + DirectX::XMConvertToRadians(5.0f);
    SetFOV(m_CurrentFOV);
}

void LockOnCamera::EndSpecialCamera()
{
    m_CameraMode = eCameraMode::Normal;
    m_IsFinalPoseCamera = false;
    m_CurrentFOV = m_DefaultFOV;
    SetFOV(m_CurrentFOV);
}

void LockOnCamera::StartFinalPoseCamera()
{
    m_IsFinalPoseCamera = true;
}

void LockOnCamera::UpdateSpecialCamera(float dt)
{
    const auto& player = m_rPlayer.get();
    const auto& target = m_rTarget.get();

    // ボスとプレイヤーの位置を取得
    DirectX::XMFLOAT3 bossPos = target.GetPosition();
    DirectX::XMFLOAT3 playerPos = player.GetPosition();

    static float g_sideOffset = 5.5f;    // 横方向の距離
    static float g_backOffset = -20.0f;    // 後ろ方向の距離
    static float g_heightOffset = 5.0f;  // カメラ自体の高さ
    static float g_lerpSpeed = 3.0f;     // 追従の滑らかさ
    static float g_lookAtHeight = 5.0f;  // 注視点の高さ（ボスの足元からどれくらい上か）

#if _DEBUG
    ImGui::Begin("Camera Debug");
    ImGui::SliderFloat("Side Offset", &g_sideOffset, -10.0f, 10.0f);
    ImGui::SliderFloat("Back Offset", &g_backOffset, -20.0f, 20.0f);
    ImGui::SliderFloat("Height Offset", &g_heightOffset, -5.0f, 10.0f);
    ImGui::SliderFloat("Lerp Speed", &g_lerpSpeed, 0.1f, 10.0f);
    ImGui::SliderFloat("LookAt Height", &g_lookAtHeight, 0.0f, 10.0f);
    ImGui::End();
#endif

    if (m_IsFinalPoseCamera)
    {
        // 最終ポーズ: プレイヤーの左下から見上げる
        DirectX::XMFLOAT3 playerRight = player.GetTransform()->GetRight();
        
        // 1. 目標位置の計算（変数を反映）
        DirectX::XMFLOAT3 playerForward = player.GetTransform()->GetForward();
        DirectX::XMFLOAT3 targetCameraPos = {
            playerPos.x - playerRight.x * g_sideOffset - playerForward.x * g_backOffset,
            playerPos.y + g_heightOffset, // 高さを反映
            playerPos.z - playerRight.z * g_sideOffset - playerForward.z * g_backOffset
        };

        // 2. Lerp補間（g_lerpSpeed を使用）
        DirectX::XMVECTOR vCurrentPos = DirectX::XMLoadFloat3(&m_spTransform.Position);
        DirectX::XMVECTOR vTargetPos = DirectX::XMLoadFloat3(&targetCameraPos);
        float lerpFactor = std::clamp(g_lerpSpeed * dt, 0.0f, 1.0f);
        DirectX::XMVECTOR vNextPos = DirectX::XMVectorLerp(vCurrentPos, vTargetPos, lerpFactor);
        DirectX::XMStoreFloat3(&m_spTransform.Position, vNextPos);

        // 3. 注視点の計算（g_lookAtHeight を使用）
        DirectX::XMFLOAT3 midPoint = {
            bossPos.x,
            bossPos.y + g_lookAtHeight, // ここを変数に
            bossPos.z
        };
        DirectX::XMVECTOR vLookAt = DirectX::XMLoadFloat3(&midPoint);
        DirectX::XMStoreFloat3(&m_LookPos, vLookAt);

        // 前方ベクトル・右方ベクトル・行列更新
        DirectX::XMVECTOR vCamPos = DirectX::XMLoadFloat3(&m_spTransform.Position);
        DirectX::XMVECTOR vForward = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(vLookAt, vCamPos));
        DirectX::XMStoreFloat3(&m_ForwardVec, vForward);

        DirectX::XMVECTOR vUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        DirectX::XMVECTOR vRight = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vUp, vForward));
        DirectX::XMStoreFloat3(&m_RightVec, vRight);

        DirectX::XMMATRIX lookAtMat = DirectX::XMMatrixLookToLH(vCamPos, vForward, vUp);
        DirectX::XMVECTOR v_Quaternion = DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixTranspose(lookAtMat));
        DirectX::XMStoreFloat4(&m_spTransform.Quaternion, v_Quaternion);
    }
    else
    {
        // 通常の弧移動カメラ
        // 弧の角度を更新（時間経過で回転）
        m_SpecialArcAngle += m_SpecialArcSpeed * dt;
        
        // ボスを中心とした円周上のカメラ位置を計算
        DirectX::XMFLOAT3 targetCameraPos = {
            bossPos.x + std::cos(m_SpecialArcAngle) * m_SpecialArcRadius,
            bossPos.y + m_SpecialArcHeight,
            bossPos.z + std::sin(m_SpecialArcAngle) * m_SpecialArcRadius
        };
        
        // 現在のカメラ位置から目標位置へLerp補間（滑らかに追従）
        DirectX::XMVECTOR vCurrentPos = DirectX::XMLoadFloat3(&m_spTransform.Position);
        DirectX::XMVECTOR vTargetPos = DirectX::XMLoadFloat3(&targetCameraPos);
        float lerpSpeed = 5.0f;
        float lerpFactor = std::clamp(lerpSpeed * dt, 0.0f, 1.0f);
        DirectX::XMVECTOR vNextPos = DirectX::XMVectorLerp(vCurrentPos, vTargetPos, lerpFactor);
        DirectX::XMStoreFloat3(&m_spTransform.Position, vNextPos);

        // 注視点はボスを固定で見る
        DirectX::XMVECTOR vBossPos = DirectX::XMLoadFloat3(&bossPos);
        DirectX::XMVECTOR vLookAt = DirectX::XMVectorAdd(vBossPos, DirectX::XMVectorSet(0.0f, m_LookOffset, 0.0f, 0.0f));
        DirectX::XMStoreFloat3(&m_LookPos, vLookAt);

        // 前方ベクトル・右方ベクトル・行列更新
        DirectX::XMVECTOR vCamPos = DirectX::XMLoadFloat3(&m_spTransform.Position);
        DirectX::XMVECTOR vForward = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(vLookAt, vCamPos));
        DirectX::XMStoreFloat3(&m_ForwardVec, vForward);

        DirectX::XMVECTOR vUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        DirectX::XMVECTOR vRight = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vUp, vForward));
        DirectX::XMStoreFloat3(&m_RightVec, vRight);

        DirectX::XMMATRIX lookAtMat = DirectX::XMMatrixLookToLH(vCamPos, vForward, vUp);
        DirectX::XMVECTOR v_Quaternion = DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixTranspose(lookAtMat));
        DirectX::XMStoreFloat4(&m_spTransform.Quaternion, v_Quaternion);
    }
}


void LockOnCamera::SaveSettings() const
{
    json j;
    j["Distance"] = m_Distance;
    j["FollowSpeed"] = m_FollowSpeed;
    j["HeightOffset"] = m_HeightOffset;
    j["LookOffset"] = m_LookOffset;
    j["LookLerp"] = m_LookLerp;
    j["ParryPhaseInDuration"] = m_ParryPhaseInDuration;
    j["ParryHoldDuration"] = m_ParryHoldDuration;
    j["ParryOutDuration"] = m_ParryOutDuration;
    j["ParryHorizontalOffset"] = m_ParryHorizontalOffset;
    j["ParryForwardOffset"] = m_ParryForwardOffset;
    j["UseExplicitTarget"] = m_UseExplicitTarget;
    j["ExplicitCameraPosOffset"] = { m_ExplicitCameraPosOffset.x, m_ExplicitCameraPosOffset.y, m_ExplicitCameraPosOffset.z };
    j["ExplicitLookPosOffset"] = { m_ExplicitLookPosOffset.x, m_ExplicitLookPosOffset.y, m_ExplicitLookPosOffset.z };
    j["DefaultFOV"] = m_DefaultFOV;
    j["ParryFOV"] = m_ParryFOV;

    auto filePath = GetSettingsFileName();
    if (!filePath.is_absolute()) {
        auto dir = std::filesystem::current_path() / "Data" / "Json" / "Camera";
        std::filesystem::create_directories(dir);
        filePath = dir / filePath;
    }
    FileManager::JsonSave(filePath, j);
}

void LockOnCamera::LoadSettings()
{
    auto filePath = GetSettingsFileName();
    if (!filePath.is_absolute()) {
        filePath = std::filesystem::current_path() / "Data" / "Json" / "Camera" / filePath;
    }
    if (!std::filesystem::exists(filePath)) return;
    json j = FileManager::JsonLoad(filePath);
    if (j.contains("Distance")) m_Distance = j["Distance"].get<float>();
    if (j.contains("FollowSpeed")) m_FollowSpeed = j["FollowSpeed"].get<float>();
    if (j.contains("HeightOffset")) m_HeightOffset = j["HeightOffset"].get<float>();
    if (j.contains("LookOffset")) m_LookOffset = j["LookOffset"].get<float>();
    if (j.contains("LookLerp")) m_LookLerp = j["LookLerp"].get<float>();
    if (j.contains("ParryPhaseInDuration")) m_ParryPhaseInDuration = j["ParryPhaseInDuration"].get<float>();
    if (j.contains("ParryHoldDuration")) m_ParryHoldDuration = j["ParryHoldDuration"].get<float>();
    if (j.contains("ParryOutDuration")) m_ParryOutDuration = j["ParryOutDuration"].get<float>();
    if (j.contains("ParryHorizontalOffset")) m_ParryHorizontalOffset = j["ParryHorizontalOffset"].get<float>();
    if (j.contains("ParryForwardOffset")) m_ParryForwardOffset = j["ParryForwardOffset"].get<float>();
    if (j.contains("UseExplicitTarget")) m_UseExplicitTarget = j["UseExplicitTarget"].get<bool>();
    if (j.contains("ExplicitCameraPosOffset") && j["ExplicitCameraPosOffset"].is_array()) {
        auto arr = j["ExplicitCameraPosOffset"];
        m_ExplicitCameraPosOffset = { arr[0].get<float>(), arr[1].get<float>(), arr[2].get<float>() };
    }
    if (j.contains("ExplicitLookPosOffset") && j["ExplicitLookPosOffset"].is_array()) {
        auto arr = j["ExplicitLookPosOffset"];
        m_ExplicitLookPosOffset = { arr[0].get<float>(), arr[1].get<float>(), arr[2].get<float>() };
    }
    if (j.contains("DefaultFOV")) m_DefaultFOV = j["DefaultFOV"].get<float>();
    if (j.contains("ParryFOV")) m_ParryFOV = j["ParryFOV"].get<float>();
    // Apply loaded/default FOV
    m_CurrentFOV = m_DefaultFOV;
    CameraBase::SetFOV(m_CurrentFOV);
}
