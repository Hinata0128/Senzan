#include "CustomFontFileEnumerator.h"

CustomFontFileEnumerator::CustomFontFileEnumerator(IDWriteFactory* pFactory, const std::vector<std::wstring>& fontFilePaths)
	: m_RefCount			( 0 )
	, m_pFactory			( pFactory )
	, m_FontFilePaths		( fontFilePaths )
	, m_CurrentFileIndex	( -1 )
{
	m_pFactory->AddRef();
	AddRef();
}

CustomFontFileEnumerator::~CustomFontFileEnumerator()
{
	m_pFactory->Release();
}

//-----------------------------------------------------------------------------------------.

IFACEMETHODIMP CustomFontFileEnumerator::QueryInterface(REFIID iid, void** ppvObject)
{
	return S_OK;
}

//-----------------------------------------------------------------------------------------.

IFACEMETHODIMP_(ULONG) CustomFontFileEnumerator::AddRef()
{
	return InterlockedIncrement(&m_RefCount);
}

//-----------------------------------------------------------------------------------------.

ULONG __stdcall CustomFontFileEnumerator::Release()
{
	ULONG newCount = InterlockedDecrement(&m_RefCount);
	if (newCount == 0) {
		delete this;
	}

	return newCount;
}

//-----------------------------------------------------------------------------------------.

IFACEMETHODIMP CustomFontFileEnumerator::MoveNext(OUT BOOL* hasCurrentFile)
{
	if (++m_CurrentFileIndex < static_cast<int>(m_FontFilePaths.size()))
	{
		*hasCurrentFile = TRUE;
		return S_OK;
	}
	else 
	{
		*hasCurrentFile = FALSE;
		return S_OK;
	}
}

//-----------------------------------------------------------------------------------------.

IFACEMETHODIMP CustomFontFileEnumerator::GetCurrentFontFile(OUT IDWriteFontFile** fontFile)
{
	return m_pFactory->CreateFontFileReference(m_FontFilePaths[m_CurrentFileIndex].c_str(), nullptr, fontFile);
}
