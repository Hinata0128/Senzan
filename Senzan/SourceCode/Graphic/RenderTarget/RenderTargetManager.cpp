#include "RenderTargetManager.h"

RenderTargetManager::RenderTargetManager()
    : m_pRenderTargets()
{
}

RenderTargetManager::~RenderTargetManager()
{
}

//-------------------------------------------------------------------------------------

void RenderTargetManager::CreateRenderTarget(
    std::string name, 
    int width, 
    int height, 
    DXGI_FORMAT colorFormat, 
    UINT colorBindFlags, 
    DXGI_FORMAT depthFormat, 
    UINT depthBindFlags, 
    DirectX::XMFLOAT4 clearColor)
{
    auto returnValue = m_pRenderTargets.emplace(name, std::make_unique<RenderTarget>());
    assert(returnValue.second && "–¼‘O‚ªd•¡‚µ‚Ä‚¢‚Ü‚·");

    returnValue.first->second->Init(
        width, 
        height, 
        colorFormat, 
        colorBindFlags, 
        depthFormat, 
        depthBindFlags, 
        clearColor);
}

//-------------------------------------------------------------------------------------

RenderTarget& RenderTargetManager::GetRenderTarget(std::string name)
{
    return *m_pRenderTargets.at(name);
}

//-------------------------------------------------------------------------------------

void RenderTargetManager::SetRenderTargets(std::string name)
{
    m_pRenderTargets.at(name)->SetRenderTargets();
}

//-------------------------------------------------------------------------------------

void RenderTargetManager::ClearRenderTarget(std::string name)
{
    m_pRenderTargets.at(name)->ClearRenderTarget();
}

//-------------------------------------------------------------------------------------

void RenderTargetManager::AllClearRenderTarget()
{
    for (const auto& renderTarget : m_pRenderTargets)
    {
        renderTarget.second->ClearRenderTarget();
    }
}
