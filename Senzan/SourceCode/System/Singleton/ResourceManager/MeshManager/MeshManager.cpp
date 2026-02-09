#include "MeshManager.h"
#include <filesystem>

namespace {
	static constexpr char STATICMESH_FILE_PATH[] = "Data\\Mesh\\Static";
	static constexpr char SKINMESH_FILE_PATH[] = "Data\\Mesh\\Skin";
}

MeshManager::MeshManager()
	: m_pStaticMeshes()
	, m_pSkinMeshes()
	, m_StaticMeshList()
	, m_SkinMeshList()
{
}

//--------------------------------------------------------------------.

MeshManager::~MeshManager()
{
}

//--------------------------------------------------------------------.

bool MeshManager::LoadAllMeshes()
{
	MeshManager& pI = GetInstance();

	// スタティックメッシュの一括読み込み.
	if (!pI.LoadMeshes<StaticMesh>(
		STATICMESH_FILE_PATH,
		pI.m_pStaticMeshes,
		pI.m_StaticMeshList))
	{
		return false;
	}

	// スキンメッシュの一括読み込み.
	if (!pI.LoadMeshes<SkinMesh>(
		SKINMESH_FILE_PATH,
		pI.m_pSkinMeshes,
		pI.m_SkinMeshList))
	{
		return false;
	}

	return true;
}

//--------------------------------------------------------------------.

std::shared_ptr<StaticMesh> MeshManager::GetStaticMesh(const std::string& name)
{
	// 指定したモデルを返す.
	for (auto& model : GetInstance().m_pStaticMeshes)
	{
		if (model.first == name) { return model.second; }
	}
	return nullptr;
}

//--------------------------------------------------------------------.

std::shared_ptr<SkinMesh> MeshManager::GetSkinMesh(const std::string& name)
{
	// 指定したモデルを返す.
	for (auto& model : GetInstance().m_pSkinMeshes)
	{
		if (model.first == name) { return model.second; }
	}
	return nullptr;
}

//--------------------------------------------------------------------.

std::vector<std::string> MeshManager::GetStaticMeshList()
{
	return GetInstance().m_StaticMeshList;
}

//--------------------------------------------------------------------.

std::vector<std::string> MeshManager::GetSkinMeshList()
{
	return GetInstance().m_SkinMeshList;
}

//--------------------------------------------------------------------.

template<typename MeshType>
bool MeshManager::LoadMeshes(
	const std::string& directoryFilePath,
	std::unordered_map<std::string, std::shared_ptr<MeshType>>& meshes,
	std::vector<std::string>& meshList)
{
	MeshManager& pI = GetInstance();

	auto loadMesh = [&](const std::filesystem::directory_entry& entry)
		{
			const std::string extension = entry.path().extension().string(); // 拡張子.
			const std::string fileName = entry.path().stem().string();		// ファイル名.
			const std::string filePath = entry.path().string();				// ファイルパス (const wchar_t* を返す).

			// 拡張子が".x"でない場合読み込まない.
			if (extension != ".x" && extension != ".X") { return; }

			auto [iterator, result] = meshes.emplace(fileName, std::make_shared<MeshType>());
			if (!result) { throw std::runtime_error("名前が重複しています:" + fileName); }

			if (FAILED(iterator->second->Init(filePath.c_str(), fileName)))
			{
				throw std::runtime_error("Not Init: " + fileName);
			}

			meshList.emplace_back(fileName);
		};

	try
	{
		// ... (省略)
		std::filesystem::recursive_directory_iterator dirIt(directoryFilePath), endIt;
		std::for_each(dirIt, endIt, loadMesh);
	}
	catch (const std::exception& e)
	{
		_ASSERT_EXPR(false, e.what());
		return false;
	}

	return true;
}
template bool MeshManager::LoadMeshes<StaticMesh>(const std::string&, std::unordered_map<std::string, std::shared_ptr<StaticMesh>>&, std::vector<std::string>&);
template bool MeshManager::LoadMeshes<SkinMesh>(const std::string&, std::unordered_map<std::string, std::shared_ptr<SkinMesh>>&, std::vector<std::string>&);

