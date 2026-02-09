#include "UITitle.h"
#include "02_UIObject/UILoader/UILoader.h"
#include "02_UIObject/Select/Select.h"
#include "Utility/Color/Color.h"

#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Game//04_Time//Time.h"
#include "Game/05_InputDevice/Input.h"


UITitle::UITitle()
	: m_pUIs		()
	, m_pSelect		( std::make_shared<Select>() )
	, m_Select		( Items::Start )
	, m_IsSelected	( true )
	, m_InitAlpha	( 0.6f )
	, m_SelectAlpha	( m_InitAlpha )
	, m_AnimReturn	( false )
	, m_AnimeSpeed	( 0.3f )
{
	UILoader::LoadFromJson("Data\\Image\\Sprite\\UIData\\Title.json", m_pUIs);
	SelectCreate();
}

//----------------------------------------------------------------.

UITitle::~UITitle()
{
}

//----------------------------------------------------------------.

void UITitle::SelectCreate()
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

void UITitle::Update()
{
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

void UITitle::LateUpdate()
{
}

//----------------------------------------------------------------.

void UITitle::Draw()
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

void UITitle::SelectUpdate()
{	
	if (Input::IsKeyDown('W')
	|| Input::IsKeyDown(VK_UP)
	|| Input::IsButtonDown(XInput::Key::Up)) 
	{
		if (m_Select == Items::Start) { return; }
		InitAnim(Items::Start);
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
        SoundManager::SetVolume("Selector",7500);
	}
	
	AnimUpdate();
}

//----------------------------------------------------------------.

void UITitle::SelectLateUpdate(std::shared_ptr<UIObject> ui)
{
	if (ui->GetUIName() == "SelectFrame_1" || ui->GetUIName() == "S_Start_0")
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

void UITitle::InitAnim(Items item)
{
	m_Select = item;
	m_SelectAlpha = m_InitAlpha;
	m_AnimReturn = false;
}

//----------------------------------------------------------------.

void UITitle::AnimUpdate()
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

//----------------------------------------------------------------.

UITitle::Items UITitle::GetSelected()
{
	return m_Select;
}
