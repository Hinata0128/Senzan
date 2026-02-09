#pragma once
#include "System/Singleton/SingletonTemplate.h"
#include "MeshManager/MeshManager.h"
#include "SpriteManager/SpriteManager.h"
#include "SoundManager/SoundManager.h"
#include "Effect/EffectResource.h"
#include "EffectManager/EffekseerManager.h"

/*******************************************
*	アセット管理クラス.
*******************************************/
class ResourceManager final
	: public Singleton<ResourceManager>
{
private:
	friend class Singleton<ResourceManager>; 
	ResourceManager();
public:

	~ResourceManager();

	/******************************************************************
	* @brief グラフィック関連(3Dモデル、UIなど)のアセットの読み込み.
	******************************************************************/
	static bool LoadMesh();


	/******************************************************************
	* @brief サウンドの読み込み.
	******************************************************************/
	static bool LoadSounds();


	/******************************************************************
	* @brief エフェクトの読み込み.
	******************************************************************/
	static bool LoadEffects();


	/******************************************************************
	* @brief スタティックメッシュを取得.
	* @param name：取得したいスタティックメッシュの名前.
	******************************************************************/
	static std::shared_ptr<StaticMesh> GetStaticMesh(const std::string& name);
	 

	/************************************************
	* @brief スキンメッシュを取得.
	* @param name：取得したいスキンメッシュの名前.
	************************************************/
	static std::shared_ptr<SkinMesh> GetSkinMesh(const std::string& name);
	

	/************************************************
	* @brief スプライト2Dを取得.
	* @param name：取得したいスプライト2Dの名前.
	************************************************/
	static std::shared_ptr<Sprite2D> GetSprite2D(const std::string& name);


	/************************************************
	* @brief エフェクトを取得.
	* @param name：取得したいエフェクトの名前.
	************************************************/
	static ::Effekseer::EffectRef GetEffect(const std::string& name);


	/************************************************
	* @brief ウィンドウハンドルを設定.
	************************************************/
	static void SethWnd(HWND hWnd);


	/************************************************
	* @brief ウィンドウハンドルを取得.
	************************************************/
	static HWND GethWnd();

private:
	HWND m_hWnd;
};
