#pragma once
#include "DWrite.h"

namespace FontList
{
	const std::wstring FontPath[] =
	{
		L"Data\\Assets\\Font\\arial.ttf",		
		L"Data\\Assets\\Font\\segoe-ui-black-2.ttf",		
		L"Data\\Assets\\Font\\SoukouMincho.ttf",		
	};
}

/*******************************************************************
*	カスタムフォントコレクションローダークラス.
*******************************************************************/
class CustomFontCollectionLoader final
	: public IDWriteFontCollectionLoader
{
public:
	CustomFontCollectionLoader();
	~CustomFontCollectionLoader();

	/**********************************************************************
	* @brief 要求されたインターフェースがサポートされているか確認.
	**********************************************************************/
	IFACEMETHODIMP QueryInterface(REFIID iid, void** ppvObject) override;


	/**********************************************************************
	* @brief COMオブジェクトの参照カウントを増やす.
	**********************************************************************/
	IFACEMETHODIMP_(ULONG) AddRef() override;


	/**********************************************************************
	* @brief COMオブジェクトの参照カウントを減らす.
	**********************************************************************/
	IFACEMETHODIMP_(ULONG) Release() override;


	/**********************************************************************
	* @brief 指定されたコレクションキーを基に、カスタムフォントコレクションの
	*		 列挙を行うためのIDWriteFontFileEnumeratorオブジェクトを作成する.
	**********************************************************************/
	IFACEMETHODIMP CreateEnumeratorFromKey(
		IDWriteFactory* factory,
		void const* collectionKey,
		UINT32 collectionKeySize,
		OUT IDWriteFontFileEnumerator** fontFileEnumerator) override;

private:
	ULONG m_RefCount;
};
