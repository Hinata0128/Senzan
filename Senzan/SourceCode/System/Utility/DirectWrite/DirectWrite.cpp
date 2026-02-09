#include "DirectWrite.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "CustomFontCollectionLoader/CustomFontCollectionLoader.h"
#include "Utility/String/String.h"

DirectWrite::DirectWrite()
	: m_pD2DFactory				( nullptr )
	, m_pRenderTarget			( nullptr )
	, m_pBrush					( nullptr )
	, m_pShadowBrush			( nullptr )
	, m_pDWriteFactory			( nullptr )
	, m_pTextFormat				( nullptr )
	, m_pTextLayout				( nullptr )
	, m_pBackBuffer				( nullptr )
	, m_pFontCollectionLoader	( nullptr )
	, m_pFontCollection			( nullptr )
	, m_FontData				( FontData() )
	, m_pFontFileList			()
	, m_FontNameList			()
	, m_IsSetting				( false )	
	, m_IsDrawOutline			( false )
	, m_Angle					( 0.0f )	
	, m_Pivot					( 0.5f, 0.5f )
{
	Init();
}

DirectWrite::~DirectWrite()
{
	m_pDWriteFactory->UnregisterFontCollectionLoader(m_pFontCollectionLoader.Get());	
}

//--------------------------------------------------------------------------------------------------.

bool DirectWrite::DrawString(
	const std::string str, 
	const DirectX::XMFLOAT2& Position, 
	bool isShadow, 
	D2D1_DRAW_TEXT_OPTIONS options)
{
	// 文字列変換.
	std::wstring wstr = MyString::StringToWString(str.c_str());

	// ターゲットサイズの取得.
	D2D1_SIZE_F TargetSize = m_pRenderTarget->GetSize();

	// テキストレイアウトを作成.
	auto result = m_pDWriteFactory->CreateTextLayout(
		wstr.c_str(),
		static_cast<UINT32>(wstr.size()),
		m_pTextFormat.Get(), 
		TargetSize.width, 
		TargetSize.height, 
		m_pTextLayout.GetAddressOf());
	assert(SUCCEEDED(result) && "テキストレイアウトの作成に失敗");


	// 描画位置の確定.
	D2D1_POINT_2F points = { Position.x,Position.y };


	// 描画の開始.
	m_pRenderTarget->BeginDraw();


	// 回転の基準点ピボットを考慮して計算.
	D2D1_POINT_2F RotationPoint = D2D1::Point2F(
		Position.x + (TargetSize.width * m_Pivot.x),
		Position.y + (TargetSize.height * m_Pivot.y)
	);


	// 回転行列を適用.
	D2D1_MATRIX_3X2_F Rotation = D2D1::Matrix3x2F::Rotation(m_Angle, RotationPoint);
	m_pRenderTarget->SetTransform(Rotation);
	

	// 影を描画する場合.
	if (isShadow)
	{
		// 影の描画
		m_pRenderTarget->DrawTextLayout(
			D2D1::Point2F(points.x - m_FontData.ShadowOffset.x, points.y - m_FontData.ShadowOffset.y),
			m_pTextLayout.Get(),
			m_pShadowBrush.Get(),
			options);
	}


	// 描画処理
	m_pRenderTarget->DrawTextLayout(points, m_pTextLayout.Get(), m_pBrush.Get(), options);

	// 描画の終了
	result = m_pRenderTarget->EndDraw();
	assert(SUCCEEDED(result) && "描画に失敗");

	return true;
}

//--------------------------------------------------------------------------------------------------.

bool DirectWrite::DrawString(
	const std::string str, 
	const DirectX::XMFLOAT2& Position,
	const DirectX::XMFLOAT2& size,
	bool isShadow, 
	D2D1_DRAW_TEXT_OPTIONS options)
{
	// 文字列変換.
	std::wstring wstr = MyString::StringToWString(str.c_str());	

	// 描画の開始.
	m_pRenderTarget->BeginDraw();

	// 座標と幅高さから描画領域を作成.
	D2D1_RECT_F rect = {};
	rect.top = Position.y;
	rect.bottom = rect.top + size.y;
	rect.left = Position.x;
	rect.right = rect.left + size.x;

	// 回転の基準点ピボットを考慮して計算.
	D2D1_POINT_2F RotationPoint = D2D1::Point2F(
		Position.x + (size.x * (m_Pivot.x)),
		Position.y + (size.y * (m_Pivot.y))
	);


	// 回転行列を適用.
	D2D1_MATRIX_3X2_F Rotation = D2D1::Matrix3x2F::Rotation(m_Angle, RotationPoint);
	m_pRenderTarget->SetTransform(Rotation);

	if (m_IsDrawOutline) 
	{
		// 描画領域を可視化（赤枠を描画）
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> rectBrush;
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &rectBrush);
		m_pRenderTarget->DrawRectangle(rect, rectBrush.Get());
	}

	// 影を描画する場合.
	if (isShadow)
	{
		m_pRenderTarget->DrawText(
			wstr.c_str(),
			static_cast<UINT>(wstr.size()),
			m_pTextFormat.Get(),
			D2D1::RectF(
				rect.left - m_FontData.ShadowOffset.x,
				rect.top - m_FontData.ShadowOffset.y,
				rect.right - m_FontData.ShadowOffset.x,
				rect.bottom - m_FontData.ShadowOffset.y),
			m_pShadowBrush.Get(),
			options);
	}


	// 描画処理.
	m_pRenderTarget->DrawText(
		wstr.c_str(),
		static_cast<UINT>(wstr.size()),
		m_pTextFormat.Get(),
		rect,
		m_pBrush.Get(),
		options);


	// 描画の終了.
	auto result = m_pRenderTarget->EndDraw();
	assert(SUCCEEDED(result) && "描画に失敗");

	return true;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::BeginSetting()
{
	m_IsSetting = true;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::EndSetting()
{
	if (m_IsSetting)
	{
		SetFontData(m_FontData);
		m_IsSetting = false;
	}
}

//--------------------------------------------------------------------------------------------------.

HRESULT DirectWrite::SetFontData(const FontData& data)
{
	m_FontData = data;

	// テキストフォーマットの作成.
	auto result = m_pDWriteFactory->CreateTextFormat(
		GetFontFileNameWithoutExtension(m_FontData.Font.c_str()).c_str(),
		m_pFontCollection.Get(),
		m_FontData.FontWeight,
		m_FontData.FontStyle,
		m_FontData.FontStretch,
		m_FontData.FontSize,
		m_FontData.LocaleName,
		m_pTextFormat.GetAddressOf());
	assert(SUCCEEDED(result) && "テキストフォーマットの作成に失敗");


	// テキストの配置設定.
	result = m_pTextFormat->SetTextAlignment(m_FontData.TextAlignment);
	assert(SUCCEEDED(result) && "テキストの配置設定に失敗");
	result = m_pTextFormat->SetParagraphAlignment(m_FontData.ParagraphAlignment);
	assert(SUCCEEDED(result) && "テキストの配置設定に失敗");


	// ID2D1SolidColorBrushの作成.
	result = m_pRenderTarget->CreateSolidColorBrush(m_FontData.Color, m_pBrush.GetAddressOf());
	assert(SUCCEEDED(result) && "ID2D1SolidColorBrushの作成に失敗");


	// 影用のブラシの作成.
	result = m_pRenderTarget->CreateSolidColorBrush(m_FontData.ShadowColor, m_pShadowBrush.GetAddressOf());
	assert(SUCCEEDED(result) && "影用のブラシの作成に失敗");

	return S_OK;
}

//--------------------------------------------------------------------------------------------------.

const std::wstring DirectWrite::GetFontName(const int num)
{
	// フォント名のリストが空の場合.
	if (m_FontNameList.empty()) { return std::wstring(); }

	// リストのサイズを超えていた場合.
	if (num >= static_cast<int>(m_FontNameList.size())) { return m_FontNameList[0]; }

	return m_FontNameList[num];
}

//--------------------------------------------------------------------------------------------------.

const std::vector<std::wstring>& DirectWrite::GetFontNames() const
{
	return m_FontNameList;
}

//--------------------------------------------------------------------------------------------------.

const std::wstring& DirectWrite::GetFont() const
{
	return m_FontData.Font;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::SetFont(const std::wstring& font)
{
	m_FontData.Font = font;
}

//--------------------------------------------------------------------------------------------------.

const DWRITE_FONT_WEIGHT DirectWrite::GetWeight() const
{
	return m_FontData.FontWeight;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::SetWeight(const DWRITE_FONT_WEIGHT weight)
{
	m_FontData.FontWeight = weight;
}

//--------------------------------------------------------------------------------------------------.

const DWRITE_FONT_STYLE DirectWrite::GetStyle() const
{
	return m_FontData.FontStyle;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::SetStyle(const DWRITE_FONT_STYLE style)
{
	m_FontData.FontStyle = style;
}

//--------------------------------------------------------------------------------------------------.

const DWRITE_FONT_STRETCH DirectWrite::GetStretch() const
{
	return m_FontData.FontStretch;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::SetStretch(const DWRITE_FONT_STRETCH stretch)
{
	m_FontData.FontStretch = stretch;
}

//--------------------------------------------------------------------------------------------------.

const FLOAT DirectWrite::GetSize() const
{
	return m_FontData.FontSize;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::SetSize(const FLOAT size)
{
	m_FontData.FontSize = size;
}

//--------------------------------------------------------------------------------------------------.

const DWRITE_TEXT_ALIGNMENT DirectWrite::GetTextAlignment() const
{
	return m_FontData.TextAlignment;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::SetTextAlignment(const DWRITE_TEXT_ALIGNMENT textAlignment)
{
	m_FontData.TextAlignment = textAlignment;
}

//--------------------------------------------------------------------------------------------------.

const DWRITE_PARAGRAPH_ALIGNMENT DirectWrite::GetParagraphAlignment() const
{
	return m_FontData.ParagraphAlignment;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::SetParagraphAlignment(const DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment)
{
	m_FontData.ParagraphAlignment = paragraphAlignment;
}

//--------------------------------------------------------------------------------------------------.

const D2D1_COLOR_F DirectWrite::GetColor() const
{
	return m_FontData.Color;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::SetColor(const D2D1_COLOR_F color)
{
	m_FontData.Color = color;
}

//--------------------------------------------------------------------------------------------------.

const D2D1_COLOR_F DirectWrite::GetShadowColor() const
{
	return m_FontData.ShadowColor;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::SetShadowColor(const D2D1_COLOR_F color)
{
	m_FontData.ShadowColor = color;
}

//--------------------------------------------------------------------------------------------------.

const D2D1_POINT_2F DirectWrite::GetShadowOffset() const
{
	return m_FontData.ShadowOffset;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::SetShadowOffset(const D2D1_POINT_2F offset)
{
	m_FontData.ShadowOffset = offset;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::SetAngle(const float& angle)
{
	m_Angle = angle;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::SetPivot(const DirectX::XMFLOAT2& pivot)
{
	m_Pivot = pivot;
}

//--------------------------------------------------------------------------------------------------.

void DirectWrite::SetIsDrawOutline(const bool& isDrawOutline)
{
	m_IsDrawOutline = isDrawOutline;
}

//--------------------------------------------------------------------------------------------------.

bool DirectWrite::Init()
{
	// Direct2Dファクトリ情報の初期化.
	auto result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_pD2DFactory.GetAddressOf());
	assert(SUCCEEDED(result) && "Direct2Dファクトリ情報の初期化に失敗");


	// バックバッファの取得.
	result = DirectX11::GetInstance().GetSwapChain()->GetBuffer(0, IID_PPV_ARGS(&m_pBackBuffer));
	assert(SUCCEEDED(result) && "バックバッファの取得に失敗");

	// dpiの設定.
	FLOAT dpiX, dpiY;
	m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);


	// レンダーターゲットの作成.
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT, 
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), 
		dpiX,
		dpiY);


	// サーフェスに描画するレンダーターゲットを作成.
	result = m_pD2DFactory->CreateDxgiSurfaceRenderTarget(
		m_pBackBuffer.Get(), 
		&props, 
		m_pRenderTarget.GetAddressOf());
	assert(SUCCEEDED(result) && "サーフェスに描画するレンダーターゲットの作成に失敗");


	// アンチエイリアシングモードの設定.
	m_pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);


	// IDWriteFactoryの作成.
	result = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED, 
		__uuidof(IDWriteFactory), 
		reinterpret_cast<IUnknown**>(m_pDWriteFactory.GetAddressOf()));
	assert(SUCCEEDED(result) && "サーフェスに描画するレンダーターゲットの作成に失敗");

	m_pFontCollectionLoader = new CustomFontCollectionLoader();

	// カスタムフォントコレクションローダー.
	result = m_pDWriteFactory->RegisterFontCollectionLoader(m_pFontCollectionLoader.Get());
	assert(SUCCEEDED(result) && "カスタムフォントコレクションローダーの作成に失敗");


	// フォントファイルの読み込み.
	result = LoadFonts();
	assert(SUCCEEDED(result) && "フォントファイルの読み込みに失敗");


	// フォント設定.
	result = SetFontData(m_FontData);
	assert(SUCCEEDED(result) && "フォントの設定に失敗");

	return true;
}

//--------------------------------------------------------------------------------------------------.

HRESULT DirectWrite::LoadFonts()
{
	// カスタムフォントコレクションの作成.
	auto result = m_pDWriteFactory->CreateCustomFontCollection(
		m_pFontCollectionLoader.Get(),
		m_pFontFileList.data(),
		static_cast<UINT32>(m_pFontFileList.size()),
		&m_pFontCollection); 
	assert((SUCCEEDED(result) && "カスタムフォントコレクションの作成に失敗"));	
	

	// フォント名を取得.
	result = GetFontFamilyName(m_pFontCollection.Get());
	assert((SUCCEEDED(result) && "フォント名の取得に失敗"));

	return S_OK;
}

//--------------------------------------------------------------------------------------------------.

HRESULT DirectWrite::GetFontFamilyName(IDWriteFontCollection* pCustomFontCollection, const WCHAR* pLocale)
{
	HRESULT result = S_OK;

	// フォントファミリー名一覧をリセット.
	std::vector<std::wstring>().swap(m_FontNameList);

	// フォントの数を取得.
	UINT32 familyCount = pCustomFontCollection->GetFontFamilyCount();

	for (UINT32 i = 0; i < familyCount; i++)
	{
		// フォントファミリーの取得.
		Microsoft::WRL::ComPtr<IDWriteFontFamily> pFontFamily = nullptr;
		result = pCustomFontCollection->GetFontFamily(i, pFontFamily.GetAddressOf());
		assert(SUCCEEDED(result) && "フォントファミリーの取得に失敗");


		// フォントファミリー名の一覧を取得.
		Microsoft::WRL::ComPtr<IDWriteLocalizedStrings> familyNames = nullptr;
		result = pFontFamily->GetFamilyNames(familyNames.GetAddressOf());
		assert(SUCCEEDED(result) && "フォントファミリー名の一覧の取得に失敗");


		// 指定されたロケールに対応するインデックスを検索.
		UINT32	index	= 0;
		BOOL	exists	= FALSE;
		result = familyNames->FindLocaleName(pLocale, &index, &exists);
		assert(SUCCEEDED(result) && "指定されたロケールに対応するインデックスの検索に失敗");


		// 指定されたロケールが見つからなかった場合は、デフォルトのロケールを使用.
		if (!exists)
		{
			result = familyNames->FindLocaleName(L"en-us", &index, &exists);
			assert(SUCCEEDED(result) && "指定されたロケールが見つかりませんでした");
		}


		// フォントファミリー名の長さを取得.
		UINT32 length	= 0;
		result			= familyNames->GetStringLength(index, &length);
		assert(SUCCEEDED(result) && "フォントファミリー名の長さの取得に失敗");


		// フォントファミリー名の取得.
		std::unique_ptr<WCHAR[]> name = std::make_unique<WCHAR[]>(length + 1);		
		result = familyNames->GetString(index, name.get(), length + 1);
		assert(SUCCEEDED(result) && "フォントファミリー名の取得に失敗");


		// フォントファミリー名を追加.
		m_FontNameList.emplace_back(name.get());
	}
	return S_OK;
}

//--------------------------------------------------------------------------------------------------.

HRESULT DirectWrite::GetAllFontFamilyName(IDWriteFontCollection* pCustomFontCollection)
{
	// フォントファミリー名一覧をリセット.
	std::vector<std::wstring>().swap(m_FontNameList);

	// フォントファミリーの数を取得.
	UINT32 familyCount = pCustomFontCollection->GetFontFamilyCount();

	for (UINT32 i = 0; i < familyCount; i++)
	{
		// フォントファミリーの取得.
		Microsoft::WRL::ComPtr<IDWriteFontFamily> fontFamily = nullptr;
		auto result = pCustomFontCollection->GetFontFamily(i, fontFamily.GetAddressOf());
		assert((SUCCEEDED(result) && "フォントファミリーの取得に失敗"));


		// フォントファミリー名の一覧を取得.
		Microsoft::WRL::ComPtr<IDWriteLocalizedStrings> familyNames = nullptr;
		result = fontFamily->GetFamilyNames(familyNames.GetAddressOf());
		assert((SUCCEEDED(result) && "フォントファミリー名の一覧の取得に失敗"));


		// フォントファミリー名の数を取得.
		UINT32 nameCount = familyNames->GetCount();

		// フォントファミリー名の数だけ繰り返す.
		for (UINT32 j = 0; j < nameCount; ++j)
		{
			// フォントファミリー名の長さを取得.
			UINT32 length = 0;
			result = familyNames->GetStringLength(j, &length);
			assert((SUCCEEDED(result) && "フォントファミリー名の長さの取得に失敗"));


			// フォントファミリー名の取得.
			std::unique_ptr<WCHAR[]> name = std::make_unique<WCHAR[]>(length + 1);
			result = familyNames->GetString(j, name.get(), length + 1);
			assert((SUCCEEDED(result) && "フォントファミリー名の取得に失敗"));


			// フォントファミリー名を追加.
			m_FontNameList.emplace_back(name.get());
		}
	}

	return S_OK;
}

//--------------------------------------------------------------------------------------------------.

std::wstring DirectWrite::GetFontFileNameWithoutExtension(const std::wstring& filePath)
{
	// 末尾から検索してファイル名と拡張子の位置を取得
	size_t start	= filePath.find_last_of(L"/\\") + 1;
	size_t end		= filePath.find_last_of(L'.');

	// ファイル名を取得
	std::wstring fileNameWithoutExtension = filePath.substr(start, end - start);

	// ファイル名を返す
	return fileNameWithoutExtension;
}