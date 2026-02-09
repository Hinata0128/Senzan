#pragma once
#include <wrl.h>

#pragma warning(push)
#pragma warning(disable:4005)

#include <d2d1.h>
#include <DWrite.h>
#include <D3DX10math.h>

#pragma warning(pop)

#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"Dwrite.lib")

#include "System/Singleton/SingletonTemplate.h"

struct FontData
{
	std::wstring 				Font;				// フォント名.
	IDWriteFontCollection*		FontCollection;		// フォントコレクション.
	DWRITE_FONT_WEIGHT			FontWeight;			// フォントの太さ.
	DWRITE_FONT_STYLE			FontStyle;			// フォントスタイル.
	DWRITE_FONT_STRETCH			FontStretch;		// フォントの幅.
	FLOAT						FontSize;			// フォントサイズ.
	WCHAR const*				LocaleName;			// ロケール名.
	DWRITE_TEXT_ALIGNMENT		TextAlignment;		// テキストの配置（水平）.
	DWRITE_PARAGRAPH_ALIGNMENT	ParagraphAlignment;	// テキストの配置（垂直）.
	D2D1_COLOR_F				Color;				// フォントの色.
	D2D1_COLOR_F				ShadowColor;		// 影の色.
	D2D1_POINT_2F				ShadowOffset;		// 影のオフセット.

	FontData()
		: Font				( L"" )
		, FontCollection	( nullptr )
		, FontWeight		( DWRITE_FONT_WEIGHT::DWRITE_FONT_WEIGHT_NORMAL )
		, FontStyle			( DWRITE_FONT_STYLE::DWRITE_FONT_STYLE_NORMAL )
		, FontStretch		( DWRITE_FONT_STRETCH::DWRITE_FONT_STRETCH_NORMAL )
		, FontSize			( 20.0f )
		, LocaleName		( L"ja-jp" )
		, TextAlignment		( DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING )
		, ParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_NEAR )
		, Color				( D2D1::ColorF(D2D1::ColorF::White) )
		, ShadowColor		( D2D1::ColorF(D2D1::ColorF::Black) )
		, ShadowOffset		( D2D1::Point2F(2.0f, -2.0f) )
	{}
};

class CustomFontCollectionLoader;//カスタムフォントコレクションローダークラス.

/**********************************************************************************************
*	DirectWriteクラス.
*	テキストの描画に使用できる.
*	参考サイト：https://islingtonsystem.hatenablog.jp/entry/DirectWrite_CustomFont_DirectX.
**********************************************************************************************/
class DirectWrite final
	: public Singleton<DirectWrite>
{
private:
	friend class Singleton<DirectWrite>;
	DirectWrite();
public:
	~DirectWrite();

	/************************************************************
	* @brief 文字の描画.
	* @param str		：描画する文字.
	* @param Position	：描画する座標.
	* @param isShadow	：影を描画するか.
	* @param options	：テキストのオプション. 
	************************************************************/
	bool DrawString(
		const std::string str,
		const DirectX::XMFLOAT2& Position,
		bool isShadow = false,
		D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_NONE);


	/************************************************************
	* @brief 文字の描画.
	* @param str		：描画する文字.
	* @param Position	：描画領域の座標.
	* @param size		：描画領域の幅高さ.
	* @param isShadow	：影を描画するか.
	* @param options	：テキストのオプション.
	************************************************************/
	bool DrawString(
		const std::string str,
		const DirectX::XMFLOAT2& Position,
		const DirectX::XMFLOAT2& size,
		bool isShadow = false,
		D2D1_DRAW_TEXT_OPTIONS options = D2D1_DRAW_TEXT_OPTIONS_NONE);

	
	/************************************************************
	* @brief 設定開始・終了.
	* @note BeginSetting：開始.
	* @note EndSetting：終了.
	* @code
	*	使用方法：
	*	BeginSetting();
	*	
	*	SetSize()などのFontData設定関数をBeginとEndの間で実行.
	* 
	*	EndSetting();
	* @endcode
	************************************************************/
	void BeginSetting();
	void EndSetting();

public: // Getter、Setter.
	
	/************************************************************
	* @brief フォントデータを設定.
	************************************************************/
	HRESULT SetFontData(const FontData& data);

	/************************************************************
	* @brief フォントリストからフォント名を取得.	
	************************************************************/	
	const std::wstring GetFontName(const int num);
	const std::vector<std::wstring>& GetFontNames() const;


	/************************************************************
	* @brief フォント名を取得・設定.
	* @note GetFont：取得.
	* @note SetFont：設定.
	************************************************************/
	const std::wstring& GetFont() const;	
	void SetFont(const std::wstring& font);

	
	/************************************************************
	* @brief フォントの太さを取得・設定.
	* @note GetWeight：取得.
	* @note SetWeight：設定.
	************************************************************/
	const DWRITE_FONT_WEIGHT GetWeight() const;
	void SetWeight(const DWRITE_FONT_WEIGHT weight);


	/************************************************************
	* @brief フォントスタイルを取得・設定.
	* @note GetStyle：取得.
	* @note SetStyle：設定.
	************************************************************/
	const DWRITE_FONT_STYLE GetStyle() const;
	void SetStyle(const DWRITE_FONT_STYLE style);


	/************************************************************
	* @brief フォントの幅を取得・設定.
	* @note GetStretch：取得.
	* @note SetStretch：設定.
	************************************************************/
	const DWRITE_FONT_STRETCH GetStretch() const;
	void SetStretch(const DWRITE_FONT_STRETCH stretch);


	/************************************************************
	* @brief フォントサイズを取得・設定.
	* @note GetSize：取得.
	* @note SetSize：設定.
	************************************************************/
	const FLOAT GetSize() const;
	void SetSize(const FLOAT size);


	/************************************************************
	* @brief テキストの配置（水平）を取得・設定.
	* @note GetTextAlignment：取得.
	* @note SetTextAlignment：設定.
	************************************************************/
	const DWRITE_TEXT_ALIGNMENT GetTextAlignment() const;
	void SetTextAlignment(const DWRITE_TEXT_ALIGNMENT textAlignment);


	/************************************************************
	* @brief テキストの配置（垂直）を取得・設定.
	* @note GetParagraphAlignment：取得.
	* @note SetParagraphAlignment：設定.
	************************************************************/
	const DWRITE_PARAGRAPH_ALIGNMENT GetParagraphAlignment() const;
	void SetParagraphAlignment(const DWRITE_PARAGRAPH_ALIGNMENT paragraphAlignment);


	/************************************************************
	* @brief フォントの色を取得・設定.
	* @note GetColor：取得.
	* @note SetColor：設定.
	************************************************************/
	const D2D1_COLOR_F GetColor() const;
	void SetColor(const D2D1_COLOR_F color);


	/************************************************************
	* @brief 影の色を取得・設定.
	* @note GetShadowColor：取得.
	* @note SetShadowColor：設定.
	************************************************************/
	const D2D1_COLOR_F GetShadowColor() const;
	void SetShadowColor(const D2D1_COLOR_F color);


	/************************************************************
	* @brief 影のオフセットを取得・設定.
	* @note GetShadowOffset：取得.
	* @note SetShadowOffset：設定.
	************************************************************/
	const D2D1_POINT_2F GetShadowOffset() const;
	void SetShadowOffset(const D2D1_POINT_2F offset); 


	/************************************************************
	* @brief Z軸回転の角度を設定.	
	************************************************************/	
	void SetAngle(const float& angle); 


	/************************************************************
	* @brief ピボットを設定.	
	************************************************************/	
	void SetPivot(const DirectX::XMFLOAT2& pivot); 


	/************************************************************
	* @brief 枠線を描画するか設定.
	************************************************************/
	void SetIsDrawOutline(const bool& isDrawOutline);
private:
	/*********************************************************
	* @brief 初期化.
	*********************************************************/
	bool Init();

	
	/*********************************************************
	* @brief フォントを読み込む.
	*********************************************************/
	HRESULT LoadFonts();


	/*********************************************************
	* @brief フォント名を取得し直す.
	*********************************************************/
	HRESULT GetFontFamilyName(IDWriteFontCollection* pCustomFontCollection, const WCHAR* pLocale = L"en-us");


	/*********************************************************
	* @brief 全てのフォント名を取得し直す.
	*********************************************************/
	HRESULT GetAllFontFamilyName(IDWriteFontCollection* pCustomFontCollection);


	/*********************************************************
	* @brief フォントのファイル名を取得する.
	*********************************************************/
	std::wstring GetFontFileNameWithoutExtension(const std::wstring& filePath);

private:
	Microsoft::WRL::ComPtr<ID2D1Factory>				m_pD2DFactory;			// Direct2Dリソース.
	Microsoft::WRL::ComPtr<ID2D1RenderTarget>			m_pRenderTarget;		// Direct2Dレンダーターゲット.
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>		m_pBrush;				// Direct2Dブラシ設定.
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>		m_pShadowBrush;			// Direct2Dブラシ設定（影）.
	Microsoft::WRL::ComPtr<IDWriteFactory>				m_pDWriteFactory;		// DirectWriteリソース.
	Microsoft::WRL::ComPtr<IDWriteTextFormat>			m_pTextFormat;			// DirectWriteテキスト形式.
	Microsoft::WRL::ComPtr<IDWriteTextLayout>			m_pTextLayout;			// DirectWriteテキスト書式.
	Microsoft::WRL::ComPtr<IDXGISurface>				m_pBackBuffer;			// サーフェス情報.
	Microsoft::WRL::ComPtr<CustomFontCollectionLoader>	m_pFontCollectionLoader;// フォントコレクションローダー.
	Microsoft::WRL::ComPtr<IDWriteFontCollection>		m_pFontCollection;		// カスタムフォントコレクション.

	FontData m_FontData;// フォントデータ.
	std::vector<Microsoft::WRL::ComPtr<IDWriteFontFile>> m_pFontFileList;// フォントファイルリスト.
	std::vector<std::wstring> m_FontNameList;// フォント名リスト.

	bool m_IsSetting;// 設定可能か.
	bool m_IsDrawOutline; // 枠線を描画するか.

	float		m_Angle;// Z軸回転角度.
	DirectX::XMFLOAT2 m_Pivot;// ピボット.	
};