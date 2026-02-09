#pragma once
#include "System/Singleton/SingletonTemplate.h"
#include "Game/01_GameObject/00_MeshObject/00_Character/03_SnowBall/SnowBall.h"
#include <vector>

// TODO: 簡易Manager.
class SnowBallManager : public Singleton<SnowBallManager>
{
public:
    friend class Singleton<SnowBallManager>;

    SnowBallManager() = default;
    ~SnowBallManager() = default;

    // 初期化.
    void Init();
    // 生成.
    void Spawn(const DirectX::XMFLOAT3& playerPos, const DirectX::XMFLOAT3& bossPos);
    void Update();
    void Draw();

private:
    std::vector<std::unique_ptr<SnowBall>> m_Pool;
};
