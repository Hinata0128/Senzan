#pragma once
#include "System/Singleton/SingletonTemplate.h"
#include "Resource/Mesh/01_Static/StaticMesh.h"
#include "Resource/Mesh/02_Skin/SkinMesh.h"

/************************************
*	メッシュ管理クラス.
************************************/
class MeshManager final
	: public Singleton<MeshManager>
{
private:
	friend class Singleton<MeshManager>;// Singletonからのアクセスを許可.
	MeshManager();
public:

	~MeshManager();

	/*******************************************
	* @brief 全てのメッシュの一括読み込み.
	*******************************************/
	static bool LoadAllMeshes();


	/************************************************
	* @brief スタティックメッシュを取得.
	* @param name：取得したいスタティックメッシュの名前.
	************************************************/
	static  std::shared_ptr<StaticMesh> GetStaticMesh(const std::string& name);


	/************************************************
	* @brief スキンメッシュを取得.
	* @param name：取得したいスキンメッシュの名前.
	************************************************/
	static  std::shared_ptr<SkinMesh> GetSkinMesh(const std::string& name);


	/************************************************
	* @brief スタティックメッシュのリストを取得.
	************************************************/
	static std::vector<std::string> GetStaticMeshList();


	/************************************************
	* @brief スキンメッシュのリストを取得.
	************************************************/
	static std::vector<std::string> GetSkinMeshList();

private:
	/**********************************************************************
	* @brief メッシュの一括読み込み.
	* @tparam MeshType：読み込むメッシュの型（CStaticMesh,CSkinMesh）.
	* @param[in]  directoryFilePath	：読み込むディレクトリのファイルパス.
	* @param[out] meshes			：格納するメッシュの変数.
	* @param[out] meshList			：メッシュリストの変数.
	**********************************************************************/
	template<typename MeshType>
	bool LoadMeshes(
		const std::string& directoryFilePath,
		std::unordered_map<std::string, std::shared_ptr<MeshType>>& meshes,
		std::vector<std::string>& meshList);
private:
	std::unordered_map<std::string, std::shared_ptr<StaticMesh>>	m_pStaticMeshes;// スタティックメッシュ.
	std::unordered_map<std::string, std::shared_ptr<SkinMesh>>		m_pSkinMeshes;	// スキンメッシュ.

	std::vector<std::string> m_StaticMeshList;	// スタティックメッシュリスト.
	std::vector<std::string> m_SkinMeshList;	// スキンメッシュリスト.
};