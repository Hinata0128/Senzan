#include "UIGameMain.h"
#include "02_UIObject/UILoader/UILoader.h"
#include "Utility/Color/Color.h"

#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Math/Easing/Easing.h"
#include "ResourceManager/ResourceManager.h"
#include "02_UIObject/ULTSparkle/ULTSparkle.h"
#include "02_UIObject/TimerWarning/TimerWarning.h"

#if _DEBUG
#include "Singleton/ImGui/CImGuiManager.h"
#endif


namespace {
	struct ComboColor
	{
		float rate;
		DirectX::XMFLOAT4 color;
	};

	static const ComboColor kComboColorTable[] =
	{
		{ 0.0f, ColorPreset::White4 },
		{ 0.3f, ColorPreset::TitleCyan },
		{ 0.6f, ColorPreset::Green },
		{ 1.0f, ColorPreset::WarmYellow },
	};
}


UIGameMain::UIGameMain()
	: m_pUIs			()
    , m_pULTSparkle     ()
    , m_pTimerWarning   ()
	, m_ComboColor		()
	, m_GuageDelSpeed	()
	, m_ClockSecInitRot	( 6.28f )
	, m_ClockSecNow		( 0.0f )
	, m_Combo			( 0 )
	, m_ComboBefore		( 0 )
	, m_ComboMax		( 999 )
	, m_PlayerHP		()
	, m_PlayerDamage	()
	, m_PlayerUlt		()
	, m_BossHP			()
	, m_BossDamage		()
{
	UILoader::LoadFromJson("Data\\Image\\Sprite\\UIData\\GameMain.json", m_pUIs);
	Create();
}

UIGameMain::~UIGameMain()
{
}

//-----------------------------------------------------------------------.

void UIGameMain::Create()
{
    auto it = m_pUIs.begin();
    for (auto& ui : m_pUIs)
    {
        auto& ui = *it;
        bool shouldRemove = false;

        if (ui->GetUIName() == "HPGauge_0") {
            m_BossHP.InitRate = ui->GetScaleX();
        }
        else if (ui->GetUIName() == "HPDamage_0") {
            m_BossDamage.InitRate = ui->GetScaleX();
        }
        else if (ui->GetUIName() == "ULTGauge_0") {
            ui->SetScaleX(0.0f);
            m_PlayerUlt.InitRate = ui->GetScaleX();
        }
        else if (ui->GetUIName() == "HPGauge_1") {
            m_PlayerHP.InitRate = ui->GetScaleX();
        }
        else if (ui->GetUIName() == "HPDamage_1") {
            m_PlayerDamage.InitRate = ui->GetScaleX();
        }
        else if (ui->GetUIName() == "Sparkle_0") {
            m_pULTSparkle = std::make_shared<ULTSparkle>(ui);
            shouldRemove = true;
        }
        else if (ui->GetUIName() == "Circle_0") {
            m_pTimerWarning = std::make_shared<TimerWarning>(ui);
            m_pTimerWarning->Create();
            shouldRemove = true;
        }

        if (shouldRemove) {
            it = m_pUIs.erase(it);
        }
        else {
            ++it;
        }
    }
}

//-----------------------------------------------------------------------.

void UIGameMain::Update()
{
	float dt = Time::GetInstance().GetUnscaledDeltaTime();

	m_PlayerHP.Update(dt);
	m_PlayerDamage.Update(dt);
	m_PlayerUlt.Update(dt);
	m_BossHP.Update(dt);
	m_BossDamage.Update(dt);

    m_pULTSparkle->Update();
    m_pTimerWarning->Update(m_TimeProgress);
	m_ComboColor = GetComboColor(m_Combo);

	for (auto& ui : m_pUIs)
	{
		if (ui->GetUIName() == "HPGauge_0") {
			ui->SetScaleX(m_BossHP.InitRate * m_BossHP.Rate);
		}
		else if (ui->GetUIName() == "HPDamage_0") {
			ui->SetScaleX(m_BossDamage.InitRate * m_BossDamage.Rate);
		}
		else if (ui->GetUIName() == "ULTGauge_0") {
			ui->SetScaleX(m_PlayerUlt.Rate);
            bool m_switch = m_PlayerUlt.Rate >= 1.0f ? true : false;
            if (m_PlayerUlt.Before < m_PlayerUlt.Max &&  m_PlayerUlt.Now >= m_PlayerUlt.Max) {
                m_pULTSparkle->DoPeakAnim();
                SoundManager::GetInstance().Play("ULTCharge",false);
                SoundManager::GetInstance().SetVolume("ULTCharge",9500);
            }
            m_pULTSparkle->SetULTGaugeStatus(m_switch, ui->GetPosition(), ui->GetDrawSize());
		}
		else if (ui->GetUIName() == "HPGauge_1") {
			ui->SetScaleX(m_PlayerHP.InitRate * m_PlayerHP.Rate);
		}
		else if (ui->GetUIName() == "HPDamage_1") {
			ui->SetScaleX(m_PlayerDamage.InitRate * m_PlayerDamage.Rate);
		}
		else if (ui->GetUIName() == "ClockSec_0") {
			ui->SetRotationZ(m_ClockSecNow);
		}
		else if (ui->GetUIName() == "Number0_0") {
			ui->AttachSprite(ResourceManager::GetSprite2D("Number" + std::to_string((m_Combo / 100) % 10)));
			ui->SetColor(m_ComboColor);
			if (m_ComboChanged) { ui->SetColor(ColorPreset::LightGray); }
		}
		else if (ui->GetUIName() == "Number0_1") {
			ui->AttachSprite(ResourceManager::GetSprite2D("Number" + std::to_string((m_Combo / 10) % 10)));
			ui->SetColor(m_ComboColor);
			if (m_ComboChanged) { ui->SetColor(ColorPreset::LightGray); }
		}
        else if (ui->GetUIName() == "Number0_2") {
            ui->AttachSprite(ResourceManager::GetSprite2D("Number" + std::to_string(m_Combo % 10)));
            ui->SetColor(m_ComboColor);
            if (m_ComboChanged) { ui->SetColor(ColorPreset::LightGray); }
        }
		else if (ui->GetUIName() == "Combo_0") {
			ui->SetColor(m_ComboColor);
		}
		else if (ui->GetUIName() == "Gradation_3") {
			ui->SetColor(ColorPreset::Black);
			if (m_ComboChanged) { ui->SetColor(ColorPreset::Invisible); }
		}
	}
	m_ComboChanged = false;
}

//-----------------------------------------------------------------------.

void UIGameMain::LateUpdate()
{
	for (auto& ui : m_pUIs)
	{
		ui->Update();
	}
    m_pULTSparkle->LateUpdate();
}

//-----------------------------------------------------------------------.

void UIGameMain::Draw()
{
    m_pTimerWarning->Draw();
    for (auto& ui : m_pUIs)
	{
		DirectX11::GetInstance().SetDepth(false);
		ui->Draw();
		DirectX11::GetInstance().SetDepth(true);
	}
    m_pULTSparkle->Draw();
}

//-----------------------------------------------------------------------.

void UIGameMain::SetCombo(int num)
{
	m_Combo = num;
}

//-----------------------------------------------------------------------.

void UIGameMain::SetTime(float progress)
{
	m_ClockSecNow = m_ClockSecInitRot * progress;
    m_TimeProgress = progress;
}

//-----------------------------------------------------------------------.

void UIGameMain::SetPlayerHP(float max, float now)
{
	m_PlayerHP.Set(max, now);
	// ここで赤を追従開始させる.
	if (m_PlayerHP.IsChanged)
	{
		m_PlayerDamage.StartFollow(
			(max > 0.0f) ? (now / max) : 0.0f
		);
	}
}

//-----------------------------------------------------------------------.

void UIGameMain::SetPlayerUlt(float max, float now)
{
	m_PlayerUlt.Set(max, now);
}

//-----------------------------------------------------------------------.

void UIGameMain::SetBossHP(float max, float now)
{
	m_BossHP.Set(max, now);
	// ここで赤を追従開始させる.
	if (m_BossHP.IsChanged)
	{
		m_BossDamage.StartFollow(
			(max > 0.0f) ? (now / max) : 0.0f
		);
	}
}

//-----------------------------------------------------------------------.

DirectX::XMFLOAT4 UIGameMain::LerpColor(const DirectX::XMFLOAT4& a, const DirectX::XMFLOAT4& b, float t)
{
	return {
		a.x + (b.x - a.x) * t,
		a.y + (b.y - a.y) * t,
		a.z + (b.z - a.z) * t,
		a.w + (b.w - a.w) * t
	};
}

//-----------------------------------------------------------------------.

DirectX::XMFLOAT4 UIGameMain::GetComboColor(int combo)
{
	if (m_ComboBefore != m_Combo) { m_ComboChanged = true; }
	m_ComboBefore = m_Combo;
	m_Combo = std::clamp(m_Combo, 0, m_ComboMax);

	int clamped = std::clamp(combo, 0, m_ComboMax);
	float t = clamped / static_cast<float>(m_ComboMax);

	const int count = _countof(kComboColorTable);

	for (int i = 0; i < count - 1; ++i)
	{
		const auto& a = kComboColorTable[i];
		const auto& b = kComboColorTable[i + 1];

		if (t >= a.rate && t <= b.rate)
		{
			float localT = (t - a.rate) / (b.rate - a.rate);
			return LerpColor(a.color, b.color, localT);
		}
	}

	return kComboColorTable[count - 1].color;
}

//-----------------------------------------------------------------------.

void UIGameMain::Gauge::Set(float max, float now)
{
	Max = max;

    if (!IsInitialized)
    {
        Now = now;
        Before = now;
        Rate = (Max > 0.0f) ? std::clamp(now / Max, 0.0f, 1.0f) : 0.0f;
        IsInitialized = true;
        IsChanged = false;
        return;
    }

    Before = Now;
	Now = now;
	IsChanged = (Now != Before);

	if (!IsChanged)
		return;

	float newRate = (Max > 0.0f) ? (Now / Max) : 0.0f;
    newRate = std::clamp(newRate, 0.0f, 1.0f);

	IsEasing	= true;
	EaseTime	= 0.0f;
	EaseStart	= Rate;
	EaseEnd		= newRate;

	DelayMax = DelayFrame * Time::GetInstance().GetUnscaledDeltaTime();
	EaseMax = EaseFrame * Time::GetInstance().GetUnscaledDeltaTime();
}

//-----------------------------------------------------------------------.

void UIGameMain::Gauge::Update(float dt)
{
	if (IsDelay)
	{
		DelayTime += dt;
		if (DelayTime >= DelayMax)
		{
			IsDelay = false;
			IsEasing = true;
			EaseTime = 0.0f;
		}
		return;
	}

	if (!IsEasing)
		return;

	EaseTime += dt;

	MyEasing::UpdateEasing(
		MyEasing::Type::OutExpo,
		EaseTime,
		EaseMax,
		EaseStart,
		EaseEnd,
		Rate
	);

	if (EaseTime >= EaseMax)
	{
		Rate = EaseEnd;
		IsEasing = false;
	}
}

//-----------------------------------------------------------------------.

void UIGameMain::Gauge::StartFollow(float targetRate)
{
	IsDelay = true;
	DelayTime = 0.0f;

	IsEasing = false;
	EaseTime = 0.0f;

	EaseStart = Rate;
	EaseEnd = targetRate;

	DelayMax = DelayFrame * Time::GetInstance().GetUnscaledDeltaTime();
	EaseMax = EaseFrame * Time::GetInstance().GetUnscaledDeltaTime();
}

//-----------------------------------------------------------------------.

bool UIGameMain::Gauge::ConsumeChanged()
{
	if (!IsChanged) return false;
	IsChanged = false;
	return true;
}
