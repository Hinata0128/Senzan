#include "AfterImage.h"
#include "Resource/Mesh/02_Skin/SkinMesh.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"

AfterImage::AfterImage()
	: m_Images()
	, m_LifeTime(0.4f)
	, m_StartAlpha(0.6f)
	, m_Darkness(0.3f)
	, m_SpawnInterval(0.1f)
{
}

AfterImage::~AfterImage()
{
	Clear();
}

//------------------------------------------------------------------------------------.

void AfterImage::AddImage(
	const DirectX::XMFLOAT3& position,
	const DirectX::XMFLOAT3& rotation,
	const DirectX::XMFLOAT3& scale,
	double animTime,
	int animNo)
{
	ImageData data;
	data.Position = position;
	data.Rotation = rotation;
	data.Scale = scale;
	data.AnimTime = animTime;
	data.AnimNo = animNo;
	data.LifeTime = m_LifeTime;
	data.MaxLifeTime = m_LifeTime;
	data.Alpha = m_StartAlpha;

	m_Images.push_back(data);
}

//------------------------------------------------------------------------------------.

void AfterImage::Update(float deltaTime)
{
	// 残像の更新（生存時間減少とアルファ更新）.
	for (auto it = m_Images.begin(); it != m_Images.end(); )
	{
		it->LifeTime -= deltaTime;

		if (it->LifeTime <= 0.0f)
		{
			// 生存時間が尽きた残像を削除.
			it = m_Images.erase(it);
		}
		else
		{
			// アルファ値をフェードアウト（線形補間）.
			float lifeRatio = it->LifeTime / it->MaxLifeTime;
			it->Alpha = m_StartAlpha * lifeRatio;
			++it;
		}
	}
}

//------------------------------------------------------------------------------------.

void AfterImage::Draw(std::shared_ptr<SkinMesh> pSkinMesh, LPD3DXANIMATIONCONTROLLER pAnimCtrl)
{
	if (!pSkinMesh || !pAnimCtrl) { return; }
	if (m_Images.empty()) { return; }

	// アルファブレンド有効化.
	DirectX11::GetInstance().SetAlphaBlend(true);

	// 元のアルファ値を保存.
	float originalAlpha = pSkinMesh->GetGlobalAlpha();

	for (const auto& img : m_Images)
	{
		// 残像用のアニメーション位置を設定.
		pAnimCtrl->SetTrackPosition(0, img.AnimTime);

		// 位置・回転・スケールを設定.
		pSkinMesh->SetPosition(img.Position);
		pSkinMesh->SetRotation(img.Rotation);
		pSkinMesh->SetScale(img.Scale);

		// アルファ値を設定（黒みを加えるため、アルファを下げる）.
		// 黒みは実際にはアルファ値を低くすることで表現.
		// より正確な黒み表現にはシェーダー対応が必要だが、
		// 現状はアルファ値の調整で半透明の暗い残像を表現.
		float darkAlpha = img.Alpha * (1.0f - m_Darkness * 0.5f);
		pSkinMesh->SetGlobalAlpha(darkAlpha);

		// 描画.
		pSkinMesh->Render(pAnimCtrl);
	}

	// 元のアルファ値を復元.
	pSkinMesh->SetGlobalAlpha(originalAlpha);

	// アルファブレンド無効化.
	DirectX11::GetInstance().SetAlphaBlend(false);
}

//------------------------------------------------------------------------------------.

void AfterImage::Clear()
{
	m_Images.clear();
}
