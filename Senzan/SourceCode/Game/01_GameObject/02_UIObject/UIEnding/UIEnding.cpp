#include "UIEnding.h"
#include "02_UIObject/UILoader/UILoader.h"
#include "02_UIObject/Select/Select.h"
#include "Utility/Color/Color.h"

#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Game//04_Time//Time.h"
#include "Game/05_InputDevice/Input.h"


UIEnding::UIEnding()
	: m_pUIs		( ) 
	, m_pSelect		( std::make_shared<Select>() )
	, m_InitAlpha	( 0.6f )
	, m_SelectAlpha	( m_InitAlpha )
	, m_AnimReturn	( false )
	, m_AnimeSpeed	( 0.3f )
    , m_GlobalAlpha ( 0.0f )
    , m_IsFadeIn    ( true )
    , m_FadeSpeed   ( 1.5f )
{
	UILoader::LoadFromJson("Data\\Image\\Sprite\\UIData\\Ending.json", m_pUIs);
	SelectCreate();
}

//----------------------------------------------------------------.

UIEnding::~UIEnding()
{
}

//----------------------------------------------------------------.

void UIEnding::SelectCreate()
{
	for (auto& ui : m_pUIs)
	{
		if (ui->GetUIName() == "SelectFrame_0") {
			m_pSelect->IsSelect(ui->GetPosition());
		}
	}
}

//----------------------------------------------------------------.

void UIEnding::Update()
{
    if (m_IsFadeIn)
    {
        m_GlobalAlpha += Time::GetInstance().GetUnscaledDeltaTime() * m_FadeSpeed;
        if (m_GlobalAlpha >= 1.0f) {
            m_GlobalAlpha = 1.0f;
            m_IsFadeIn = false;
        }
        for (auto& ui : m_pUIs)
        {
            ui->SetColor(ColorUtil::RGBA(ColorPreset::White3, m_GlobalAlpha));
        }
    }

	SelectUpdate();
	for (auto& ui : m_pUIs)
	{
		ui->Update();
		SelectLateUpdate(ui);
	}
	m_pSelect->LateUpdate();
	m_pSelect->Update();
}

//----------------------------------------------------------------.

void UIEnding::LateUpdate()
{
}

//----------------------------------------------------------------.

void UIEnding::Draw()
{
	for (auto& ui : m_pUIs)
	{
		DirectX11::GetInstance().SetDepth(false);
		ui->Draw();
		DirectX11::GetInstance().SetDepth(true);
	}
	m_pSelect->Draw();
}

//----------------------------------------------------------------.

void UIEnding::SelectUpdate()
{
	AnimUpdate();
}

//----------------------------------------------------------------.

void UIEnding::SelectLateUpdate(std::shared_ptr<UIObject> ui)
{
	if (ui->GetUIName() == "SelectFrame_0" || ui->GetUIName() == "S_End_0")
	{
		ui->SetColor(ColorUtil::RGBA(ColorPreset::Selected, m_SelectAlpha));
	}
}

//----------------------------------------------------------------.

void UIEnding::AnimUpdate()
{
	if (m_SelectAlpha < m_InitAlpha) { m_AnimReturn = false; }
	else if (1.f < m_SelectAlpha) { m_AnimReturn = true; }

	if (m_AnimReturn) {
		m_SelectAlpha = m_SelectAlpha - Time::GetInstance().GetUnscaledDeltaTime() * m_AnimeSpeed;
	}
	else {
		m_SelectAlpha = m_SelectAlpha + Time::GetInstance().GetUnscaledDeltaTime() * m_AnimeSpeed;
	}
}
