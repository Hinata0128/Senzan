#include "UIGameOver.h"
#include "02_UIObject/UILoader/UILoader.h"
#include "02_UIObject/Select/Select.h"
#include "Utility/Color/Color.h"

#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Game//04_Time//Time.h"
#include "Game/05_InputDevice/Input.h"

UIGameOver::UIGameOver()
	: m_pUIs		()
	, m_pSelect		( std::make_shared<Select>() )
	, m_Select		( Items::Continue )
	, m_IsSelected	( true )
	, m_InitAlpha	( 0.6f )
	, m_SelectAlpha	( m_InitAlpha )
	, m_AnimReturn	( false )
	, m_AnimeSpeed	( 0.3f )
    , m_GlobalAlpha ( 0.0f )
    , m_IsFadeIn    ( true )
    , m_FadeSpeed   ( 1.5f )
{
	UILoader::LoadFromJson("Data\\Image\\Sprite\\UIData\\GameOver.json", m_pUIs);
	SelectCreate();
}

//----------------------------------------------------------------.

UIGameOver::~UIGameOver()
{
}

//----------------------------------------------------------------.

void UIGameOver::SelectCreate()
{
	for (auto& ui : m_pUIs)
	{
		if (m_IsSelected && ui->GetUIName() == "SelectFrame_1") {
			m_pSelect->IsSelect(ui->GetPosition());
			m_IsSelected = false;
		}
	}
}

//----------------------------------------------------------------.

void UIGameOver::Update()
{
    if (m_IsFadeIn)
    {
        m_GlobalAlpha += Time::GetInstance().GetDeltaTime() * m_FadeSpeed;
        if (m_GlobalAlpha >= 1.0f) {
            m_GlobalAlpha = 1.0f;
            m_IsFadeIn = false;
        }
        for (auto& ui : m_pUIs)
        {
            if (ui->GetUIName() == "White_0") {
                ui->SetColor(ColorUtil::RGBA(ColorPreset::Black3, m_GlobalAlpha));
            }
            else {
                ui->SetColor(ColorUtil::RGBA(ColorPreset::White3, m_GlobalAlpha));
            }
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

void UIGameOver::LateUpdate()
{
}

//----------------------------------------------------------------.

void UIGameOver::Draw()
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

void UIGameOver::SelectUpdate()
{	
	if (Input::IsKeyDown('W')
	|| Input::IsKeyDown(VK_UP)
	|| Input::IsButtonDown(XInput::Key::Up)) 
	{
		if (m_Select == Items::Continue) { return; }
		InitAnim(Items::Continue);
		m_IsSelected = true;
        SoundManager::Play("Selector");
        SoundManager::SetVolume("Selector", 7500);
	}
	else if (Input::IsKeyDown('S')
	||	Input::IsKeyDown(VK_DOWN)
	||	Input::IsButtonDown(XInput::Key::Down) ) 
	{
		if (m_Select == Items::End) { return; }
		InitAnim(Items::End);
		m_IsSelected = true;
        SoundManager::Play("Selector");
        SoundManager::SetVolume("Selector", 7500);
	}
	
	AnimUpdate();
}

//----------------------------------------------------------------.

void UIGameOver::SelectLateUpdate(std::shared_ptr<UIObject> ui)
{
	if (ui->GetUIName() == "SelectFrame_1" || ui->GetUIName() == "S_Continue_0")
	{
		if (m_Select == 0) { 
			ui->SetColor(ColorUtil::RGBA(ColorPreset::Selected, m_SelectAlpha));

			if (m_IsSelected && ui->GetUIName() == "SelectFrame_1") {
				m_pSelect->IsSelect(ui->GetPosition());
				m_IsSelected = false;
			}
		}
		else {
			ui->SetColor(ColorPreset::UnActive); 
		}
	}
	else if (ui->GetUIName() == "SelectFrame_0" || ui->GetUIName() == "S_End_0")
	{
		if (m_Select == 1) {
			ui->SetColor(ColorUtil::RGBA(ColorPreset::Selected, m_SelectAlpha));

			if (m_IsSelected && ui->GetUIName() == "SelectFrame_0") {
				m_pSelect->IsSelect(ui->GetPosition());
				m_IsSelected = false;
			}
		}
		else {
			ui->SetColor(ColorPreset::UnActive);
		}
	}
}

//----------------------------------------------------------------.

void UIGameOver::InitAnim(Items item)
{
	m_Select = item;
	m_SelectAlpha = m_InitAlpha;
	m_AnimReturn = false;
}

//----------------------------------------------------------------.

void UIGameOver::AnimUpdate()
{
	if (m_SelectAlpha < m_InitAlpha) { m_AnimReturn = false; }
	else if (1.f < m_SelectAlpha) { m_AnimReturn = true; }

	if (m_AnimReturn) {
		m_SelectAlpha = m_SelectAlpha - Time::GetInstance().GetDeltaTime() * m_AnimeSpeed;
	}
	else {
		m_SelectAlpha = m_SelectAlpha + Time::GetInstance().GetDeltaTime() * m_AnimeSpeed;
	}
}

//----------------------------------------------------------------.

UIGameOver::Items UIGameOver::GetSelected()
{
	return m_Select;
}
