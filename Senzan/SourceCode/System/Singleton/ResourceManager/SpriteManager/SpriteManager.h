#pragma once
#include "System/Singleton/SingletonTemplate.h"
#include "Resource/Mesh/00_Sprite/UISprite/UISprite.h"
#include "Resource/Mesh/00_Sprite/Sprite3D/Sprite3D.h"

/************************************
*	スプライト管理クラス.
* まだ綺麗になってないしスプライト3Dも読み込んでない
* エクセルとかでスプライトのサイズとかしていしてそれの情報を元に作成するとかありかも
************************************/
class SpriteManager final
	: public Singleton<SpriteManager>
{
private:
	friend class Singleton<SpriteManager>;
	SpriteManager();
public:
	~SpriteManager();

	/****************************************************
	* @brief スプライトの一括読み込み.
	****************************************************/
	static bool LoadSprites();


	/************************************************
	* @brief スプライト2Dを取得.
	* @param name：取得したいスプライト2Dの名前.
	************************************************/	
	static std::shared_ptr<Sprite2D> GetSprite2D(const std::string& name);


	/************************************************
	* @brief スプライト3Dを取得.
	* @param name：取得したいスプライト2Dの名前.
	************************************************/
	static std::shared_ptr<Sprite3D> GetSprite3D(const std::string& name);


	/************************************************
	* @brief スプライト2Dのリストを取得.
	************************************************/
	static std::vector<std::string> GetSprite2DList();
	static std::vector<std::string> GetSprite2D2List();
private:
	/************************************************
	* @brief スプライト2Dの一括読み込み.
	************************************************/
	static bool LoadSprites2D();

private:	
	std::unordered_map<std::string,std::shared_ptr<Sprite2D>> m_pSprites2D; // スプライト2D.
	std::unordered_map<std::string,std::shared_ptr<Sprite3D>> m_pSprites3D; // スプライト3D.

	std::vector<std::string> m_Sprite2DList; // 読み込んだスプライト2Dのリスト.
	std::vector<std::string> m_Sprite2D2List; // 読み込んだスプライト2Dのリスト.
};