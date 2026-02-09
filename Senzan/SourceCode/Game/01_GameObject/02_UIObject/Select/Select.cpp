#include "Select.h"
#include "02_UIObject/UILoader/UILoader.h"

#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Game//04_Time//Time.h"
#include "Math/Easing/Easing.h"

namespace {
	constexpr const char* kHiddenTags[] =
	{
		"ZNode",
		"SelectBack",
		"SelectFrame",
	};
}


Select::Select()
	: m_pUIs		()
	, m_Whites		()
	, m_DoAnimation	( false )
	, m_StartRate	( 1.0f )
	, m_Rate		( 0.0f )
	, m_Alpha		( 1.0f )
	, m_Time		( 0.0f )
	, m_Duration	( 0.7f )
	, m_MoveDistance( 38.0f )
{
	UILoader::LoadFromJson("Data\\Image\\Sprite\\UIData\\SelectAnimation.json", m_pUIs);
	Create();
}

//----------------------------------------------------------------.

Select::~Select()
{
}

//----------------------------------------------------------------.

void Select::Create() 
{
	int whiteIndex = 0;

	for (auto& ui : m_pUIs)
	{
		if (IsHiddenUI(ui->GetUIName())) { continue; }

		WhiteInfo info;
		info.basePos = ui->GetPosition();
		info.scale = ui->GetScale();

		std::string number = ui->GetUIName().substr(ui->GetUIName().find_last_of('_'));
		for (auto& zui : m_pUIs)
		{
			if (zui->GetUIName().find("ZNode") != std::string::npos &&
				zui->GetUIName().substr(zui->GetUIName().find_last_of('_')) == number)
			{
				DirectX::XMFLOAT3 v{
					zui->GetPosition().x - info.basePos.x,
					zui->GetPosition().y - info.basePos.y,
					zui->GetPosition().z - info.basePos.z
				};

				float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

				// ゼロ除算ガード
				if (len > 0.0001f) {
					info.moveDir = { v.x / len, v.y / len, v.z / len };
				}
				else {
					info.moveDir = { 0.0f, 0.0f, 0.0f };
				}

				break;
			}
		}
		m_Whites.push_back(info);

		++whiteIndex;
	}
}

//----------------------------------------------------------------.

void Select::Update()
{
	if (!m_DoAnimation) { return; }

	m_Time += Time::GetInstance().GetUnscaledDeltaTime();

	float t = std::clamp(m_Time / m_Duration, 0.0f, 1.0f);

	// 移動用Easing.
	float moveT = 0.0f;
	MyEasing::UpdateEasing(
		MyEasing::Type::OutCirc,
		m_Time,
		m_Duration,
		0.0f,
		1.0f,
		moveT
	);

	// αlpha用Easing.
	float alpha = 1.0f;
	MyEasing::UpdateEasing(
		MyEasing::Type::InCubic,
		m_Time,
		m_Duration,
		1.0f,
		0.0f,
		alpha
	);

	// scale用Easing.
	float scale = 1.0f;
	MyEasing::UpdateEasing(
		MyEasing::Type::OutCirc,
		m_Time,
		m_Duration,
		1.0f,
		0.0f,
		scale
	);

	int whiteIndex = 0;
	for (auto& ui : m_pUIs)
	{
		if (IsHiddenUI(ui->GetUIName())) { continue; }

		const auto& info = m_Whites[whiteIndex];

		float horizontalBoost = 1.0f;
		if (whiteIndex == 1 || whiteIndex == 3)
		{
			horizontalBoost = 3.8f;
		}

		DirectX::XMFLOAT3 startPos{
			info.basePos.x + m_OffsetPos.x,
			info.basePos.y + m_OffsetPos.y,
			info.basePos.z + m_OffsetPos.z
		};

		DirectX::XMFLOAT3 pos{
			startPos.x + info.moveDir.x * m_MoveDistance * moveT * horizontalBoost,
			startPos.y + info.moveDir.y * m_MoveDistance * moveT,
			startPos.z + info.moveDir.z * m_MoveDistance * moveT
		};

		ui->SetPosition(pos);
		ui->SetAlpha(alpha);
		ui->SetScaleX(info.scale.x * scale);

		ui->Update();

		++whiteIndex;
	}

	if (t >= 1.0f)
	{
		m_DoAnimation = false;
	}
}

//----------------------------------------------------------------.

void Select::LateUpdate()
{
}

//----------------------------------------------------------------.

void Select::Draw()
{
	for (auto& ui : m_pUIs)
	{
		if (IsHiddenUI(ui->GetUIName())){ continue; }

		DirectX11::GetInstance().SetDepth(false);
		ui->Draw();
		DirectX11::GetInstance().SetDepth(true);
	}
}

//----------------------------------------------------------------.

void Select::IsSelect(DirectX::XMFLOAT3 pos)
{
	m_OffsetPos = pos;
	int whiteIndex = 0;
	for (auto& ui : m_pUIs)
	{
		if (IsHiddenUI(ui->GetUIName())) { continue; }

		const DirectX::XMFLOAT3& initialPos = m_Whites[whiteIndex].basePos;

		DirectX::XMFLOAT3 targetPos{
			pos.x + initialPos.x,
			pos.y + initialPos.y,
			pos.z + initialPos.z
		};

		ui->SetPosition(targetPos);
		ui->SetAlpha(1.0f);

		++whiteIndex;
	}

	m_Time = 0.0f;
	m_DoAnimation = true;
	m_Rate = m_StartRate;
}

//----------------------------------------------------------------.

bool Select::IsHiddenUI(const std::string& name)
{
	for (const auto& tag : kHiddenTags)
	{
		if (name.find(tag) != std::string::npos) return true;
	}
	return false;
}
