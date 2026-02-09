#pragma once
#include "DWrite.h"

/*******************************************************************
*	カスタムファイルローダークラス.
*******************************************************************/
class CustomFontFileEnumerator final
	: public IDWriteFontFileEnumerator
{
public:
	CustomFontFileEnumerator(IDWriteFactory* pFactory, const std::vector<std::wstring>& fontFilePaths);
	~CustomFontFileEnumerator();

	/*************************************************************
	* @brief 要求されたインターフェースがサポートされているか確認.
	* @note この関数は呼ばれていなかったが親が純粋仮想関数なので宣言だけ.
	*************************************************************/
	IFACEMETHODIMP QueryInterface(REFIID iid, void** ppvObject) override;


	/*************************************************************
	* @brief COMオブジェクトの参照カウントを増やす.
	*************************************************************/
	IFACEMETHODIMP_(ULONG) AddRef() override;


	/*************************************************************
	* @brief COMオブジェクトの参照カウントを減らす.
	*************************************************************/
	IFACEMETHODIMP_(ULONG) Release() override;


	/*************************************************************
	* @brief 次のフォントファイルに移動.
	*************************************************************/
	IFACEMETHODIMP MoveNext(OUT BOOL* hasCurrentFile) override;


	/*************************************************************
	* @brief 現在のフォントファイルを取得.
	*************************************************************/
	IFACEMETHODIMP GetCurrentFontFile(OUT IDWriteFontFile** fontFile) override;
private:
	ULONG						m_RefCount;
	IDWriteFactory*				m_pFactory;			// DirectWriteファクトリ.	
	std::vector<std::wstring>	m_FontFilePaths;	// フォントファイルのパス.
	int							m_CurrentFileIndex;	// 現在のファイルインデックス.
};
