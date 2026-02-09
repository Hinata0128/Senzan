#include "CustomFontCollectionLoader.h"
#include "..\CustomFontFileEnumerator\CustomFontFileEnumerator.h"

CustomFontCollectionLoader::CustomFontCollectionLoader() 
	: m_RefCount( 0 )
{
}

CustomFontCollectionLoader::~CustomFontCollectionLoader()
{
}

//--------------------------------------------------------------------------------------.

IFACEMETHODIMP CustomFontCollectionLoader::QueryInterface(REFIID iid, void** ppvObject)
{
	if (iid == __uuidof(IUnknown) || iid == __uuidof(IDWriteFontCollectionLoader))
	{
		*ppvObject = this;
		AddRef();
		return S_OK;
	}
	else
	{
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}
}

//--------------------------------------------------------------------------------------.

ULONG __stdcall CustomFontCollectionLoader::AddRef()
{
	return InterlockedIncrement(&m_RefCount);
}

//--------------------------------------------------------------------------------------.

ULONG __stdcall CustomFontCollectionLoader::Release()
{
	ULONG newCount = InterlockedDecrement(&m_RefCount);
	if (newCount == 0) { delete this; }

	return newCount;
}

//--------------------------------------------------------------------------------------.

IFACEMETHODIMP CustomFontCollectionLoader::CreateEnumeratorFromKey(
	IDWriteFactory* factory, 
	void const* collectionKey, 
	UINT32 collectionKeySize, 
	OUT IDWriteFontFileEnumerator** fontFileEnumerator)
{
	// 読み込むフォントファイルのパスを渡す.
	std::vector<std::wstring> fontFilePaths(std::begin(FontList::FontPath), std::end(FontList::FontPath));

	// カスタムフォントファイル列挙子の作成.
	*fontFileEnumerator = new CustomFontFileEnumerator(factory, fontFilePaths);

	// メモリ不足の場合はエラーを返す.
	if (*fontFileEnumerator == nullptr) { return E_OUTOFMEMORY; }

	return S_OK;
}
