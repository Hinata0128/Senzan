#pragma once
#include "System/Singleton/SingletonTemplate.h"
#include "UIObject.h"

/****************************************************
*	UIObjectを管理するクラス.
****************************************************/
class UIObjectManager final
	: public Singleton<UIObjectManager>
{
private:
	friend class Singleton<UIObjectManager>;
	UIObjectManager();
public:
	~UIObjectManager();

	void Update();
	void Update(const std::string& sceneName);
	void Draw();
	void Draw(const std::string& sceneName);

	void AllUpdate();
	void AllDraw();

	/********************************************************
	* @brief UIObjectの追加.
	********************************************************/
	void AddUIObject(const std::shared_ptr<UIObject>& pUIObject);


	/********************************************************
	* @brief UIObjectの削除.
	* @param index：削除するObjectの要素番号.
	********************************************************/
	void RemoveUIObject(const int index);	


	template<typename T>
	const std::shared_ptr<T> GetUIObject(const std::string& name, const std::string& sceneName);


	/********************************************************
	* @brief 全てのUIObjectを取得.
	********************************************************/
	const std::vector<std::shared_ptr<UIObject>>& GetAllUIObject() const;
	const std::vector<std::shared_ptr<UIObject>>& GetAllUIObject(const std::string& sceneName) const;

	/********************************************************
	* @brief 全てのシーンのUIObjectを削除.	
	********************************************************/
	void AllUnLoadUI();

	/********************************************************
	* @brief 指定したシーンのUIObjectを削除.
	* @param filePath：削除するシーン名.
	********************************************************/
	void UnLoadUI(const std::string& sceneName);

	/********************************************************
	* @brief レイヤー順にソート.	
	********************************************************/	
	const int SortLayer(const int& selectID = 0);

private:
	std::vector<std::shared_ptr<UIObject>> m_pUIObjects;
	std::unordered_map<std::string, std::vector<std::shared_ptr<UIObject>>> m_pActiveUIObjects;
};

template<typename T>
inline const std::shared_ptr<T> UIObjectManager::GetUIObject(const std::string& name, const std::string& sceneName)
{
	static_assert(std::is_base_of_v<UIObject, T>);

	auto it = std::find_if(
		m_pActiveUIObjects.at(sceneName).begin(),
		m_pActiveUIObjects.at(sceneName).end(),
		[name](const std::shared_ptr<UIObject>& ui) {
			return ui->GetTag() == name;
		});

	return std::dynamic_pointer_cast<T>(*it);
}
