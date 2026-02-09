#include "UIObjectManager.h"
#include "System/Singleton/ResourceManager/SpriteManager/SpriteManager.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Utility\FileManager\FileManager.h"
#include "Utility\String\String.h"


UIObjectManager::UIObjectManager()
    : m_pUIObjects      ()
    , m_pActiveUIObjects()
{
}

//---------------------------------------------------------------------------.

UIObjectManager::~UIObjectManager()
{
}

//---------------------------------------------------------------------------.

void UIObjectManager::Update()
{
    for (const auto& object : m_pUIObjects)
    {
        object->Update();
    }
}

void UIObjectManager::Update(const std::string& sceneName)
{
    for (const auto& object : m_pActiveUIObjects.at(sceneName))
    {
        object->Update();
    }
}

//---------------------------------------------------------------------------.

void UIObjectManager::Draw()
{
    DirectX11::GetInstance().SetDepth(false);

    for (const auto& object : m_pUIObjects)
    {
        object->Draw();
    }

    DirectX11::GetInstance().SetDepth(true);
}

//---------------------------------------------------------------------------.

void UIObjectManager::Draw(const std::string& sceneName)
{
    DirectX11::GetInstance().SetDepth(false);

    for (const auto& object : m_pActiveUIObjects.at(sceneName))
    {
        object->Draw();
    }

    DirectX11::GetInstance().SetDepth(true);
}

//---------------------------------------------------------------------------.

void UIObjectManager::AllUpdate()
{
    for (const auto& [scene, objects] : m_pActiveUIObjects)
    {
        for (const auto& object : objects)
        {
            object->Update();
        }
    }
}

//---------------------------------------------------------------------------.

void UIObjectManager::AllDraw()
{
    DirectX11::GetInstance().SetDepth(false);

    for (const auto& [scene, objects] : m_pActiveUIObjects)
    {
        for (const auto& object : objects)
        {
            object->Draw();
        }
    }

    DirectX11::GetInstance().SetDepth(true);
}

//---------------------------------------------------------------------------.

void UIObjectManager::AddUIObject(const std::shared_ptr<UIObject>& pUIObject)
{
    m_pUIObjects.emplace_back(pUIObject);
}

//---------------------------------------------------------------------------.

void UIObjectManager::RemoveUIObject(const int index)
{
    if (m_pUIObjects.empty()) { return; }    

    m_pUIObjects.erase(m_pUIObjects.begin() + index);    
}

//---------------------------------------------------------------------------.

const std::vector<std::shared_ptr<UIObject>>& UIObjectManager::GetAllUIObject() const
{
    return m_pUIObjects;
}

//---------------------------------------------------------------------------.

const std::vector<std::shared_ptr<UIObject>>& UIObjectManager::GetAllUIObject(const std::string& sceneName) const
{
    return m_pActiveUIObjects.at(sceneName);
}

//---------------------------------------------------------------------------.

void UIObjectManager::AllUnLoadUI()
{
    m_pUIObjects.clear();
    m_pActiveUIObjects.clear();
}

//---------------------------------------------------------------------------.

void UIObjectManager::UnLoadUI(const std::string& sceneName)
{
    // 指定されたシーンのUIを削除.
    m_pActiveUIObjects.at(sceneName).clear();
}

//---------------------------------------------------------------------------.

const int UIObjectManager::SortLayer(const int& selectID)
{    
    if (m_pUIObjects.empty()) { return 0; }

    auto selectUI = m_pUIObjects[selectID];

    std::sort(m_pUIObjects.begin(), m_pUIObjects.end(),
        [](const std::shared_ptr<UIObject>& a, const std::shared_ptr<UIObject>& b)
        {
            return a->GetLayer() < b->GetLayer();
        });

    for (size_t no = 0; no < m_pUIObjects.size(); no++)
    {
        if (selectUI == m_pUIObjects[no])
        {
            return static_cast<int>(no);
        }
    }

    return 0;
}
