#include "SnowBallManager.h"
#include "System/Singleton/CollisionDetector/CollisionDetector.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Game/01_GameObject/00_MeshObject/MeshObject.h"

void SnowBallManager::Update()
{
    for (auto it = m_Pool.begin(); it != m_Pool.end(); )
    {
        if (*it)
        {
            (*it)->Update();
            if ((*it)->ShouldDestroy())
            {
                it = m_Pool.erase(it);
                continue;
            }
        }
        ++it;
    }
}

void SnowBallManager::Draw()
{
    for (auto& SnowBall : m_Pool)
    {
        if (!SnowBall) continue;

        float alpha = SnowBall->GetAlpha();
        bool useAlpha = (alpha < 0.999f);
        if (useAlpha) DirectX11::GetInstance().SetAlphaBlend(true);

        SnowBall->Draw();

        if (useAlpha) DirectX11::GetInstance().SetAlphaBlend(false);
    }
}

// 初期化.
void SnowBallManager::Init()
{
    m_Pool.clear();
}

void SnowBallManager::Spawn(const DirectX::XMFLOAT3& playerPos, const DirectX::XMFLOAT3& bossPos)
{
    auto up = std::make_unique<SnowBall>();
    SnowBall* ptr = up.get();
    ptr->Fire(playerPos, bossPos);
    m_Pool.push_back(std::move(up));
    return ;
}
