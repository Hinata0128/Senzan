#include "SpriteManager.h"
#include "Resource/Mesh/00_Sprite/UISprite/UISprite.h"
#include "Resource/Mesh/00_Sprite/Sprite3D/Sprite3D.h"


namespace
{
	static constexpr char UI_FILE_PATH[] = "Data\\Image\\Sprite";
}

SpriteManager::SpriteManager()
	: m_pSprites2D	()
	, m_pSprites3D	()
	, m_Sprite2DList()
{
}

//--------------------------------------------------------------------------.

SpriteManager::~SpriteManager()
{
}

//--------------------------------------------------------------------------.

bool SpriteManager::LoadSprites()
{
	// スプライト2Dの読み込み.
	if (LoadSprites2D() == false) { return false; }
	return true;
}
//--------------------------------------------------------------------------.

std::shared_ptr<Sprite2D> SpriteManager::GetSprite2D(const std::string& name)
{
	// 指定したスプライトを返す.
	for (auto& Sprite : GetInstance().m_pSprites2D)
	{
		if (Sprite.first == name) { return Sprite.second; }
	}
	return nullptr;

}

//--------------------------------------------------------------------------.

std::shared_ptr<Sprite3D> SpriteManager::GetSprite3D(const std::string& name)
{
	// 指定したスプライトを返す.
	for (auto& Sprite : GetInstance().m_pSprites3D)
	{
		if (Sprite.first == name) { return Sprite.second; }
	}
	return nullptr;
}

//--------------------------------------------------------------------------.

std::vector<std::string> SpriteManager::GetSprite2DList()
{
	return GetInstance().m_Sprite2DList;
}

//--------------------------------------------------------------------------.

std::vector<std::string> SpriteManager::GetSprite2D2List()
{
	return GetInstance().m_Sprite2D2List;
}

//--------------------------------------------------------------------------.

bool SpriteManager::LoadSprites2D()
{
	SpriteManager& pI = GetInstance();

	auto LoadSprite2D2 = [&](const std::filesystem::directory_entry& Entry)
		{
			const std::string	extension = Entry.path().extension().string();	// 拡張子.
			const std::string	fileName = Entry.path().stem().string();		// ファイル名.
			const std::string	filePath = Entry.path().string();				// ファイルパス.

			// 拡張子が".png"でない場合読み込まない.
			if (extension != ".png" && extension != ".PNG") { return; }

			auto [iterator, result] = pI.m_pSprites2D.emplace(fileName, std::make_shared<Sprite2D>());

			if (!result) { throw std::runtime_error("名前が重複しています:" + fileName); }

			if (!iterator->second->Initialize(Entry.path()))
			{
				throw std::runtime_error("Not Init Sound: " + fileName);
			}

			// リストに追加.
			pI.m_Sprite2D2List.emplace_back(fileName);
		};

	try
	{
		std::filesystem::recursive_directory_iterator dirIt(UI_FILE_PATH), endIt;
		std::for_each(dirIt, endIt, LoadSprite2D2);
	}
	catch (const std::exception& e)
	{
		//_ASSERT_EXPR(false, e.what());
		return false;
	}

	return true;
}
