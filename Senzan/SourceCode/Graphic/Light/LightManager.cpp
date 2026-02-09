#include "LightManager.h"
#include "Graphic/Light/DirectionLight/DirectionLight.h"

LightManager::LightManager()
    : m_pDirectionLight()
{
}

//--------------------------------------------------------------------------.

LightManager::~LightManager()
{
}

//--------------------------------------------------------------------------.

void LightManager::UpdateViewAndProjection()
{
    LightManager& pI = GetInstance();

    if (const auto& pDirectionLight = pI.m_pDirectionLight.lock())
	{
		pDirectionLight->UpdateViewAndProjection();
	}
}

//--------------------------------------------------------------------------.

void LightManager::AttachDirectionLight(const std::shared_ptr<DirectionLight>& pLight)
{
    GetInstance().m_pDirectionLight = pLight;
}

//--------------------------------------------------------------------------.

const std::shared_ptr<DirectionLight> LightManager::GetDirectionLight()
{
    if (const auto& pDirectionLight = GetInstance().m_pDirectionLight.lock())
    {
        return pDirectionLight;
    }
    return nullptr;
}

