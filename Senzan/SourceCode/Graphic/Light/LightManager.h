#pragma once
#include "System/Singleton/SingletonTemplate.h"


// 前方宣言.
class DirectionLight;

/**********************************
*   ライト管理クラス.
**********************************/
class LightManager final
	: public Singleton<LightManager>
{
private:
	friend class Singleton<LightManager>;
	LightManager();
public:

	~LightManager();


	/************************************************************************
	* @brief 全てのライトのビュー、プロジェクションの更新.
	************************************************************************/
	static void UpdateViewAndProjection();


	/************************************************************************
	* @brief ディレクションライトを接続.	
	************************************************************************/
	static void AttachDirectionLight(const std::shared_ptr<DirectionLight>& pLight);

public: // Getter.

	/************************************************************************
	* @brief ディレクションライトを取得.
	************************************************************************/
	static const std::shared_ptr<DirectionLight> GetDirectionLight();

private:	
	std::weak_ptr<DirectionLight> m_pDirectionLight;// ディレクションライト.
};