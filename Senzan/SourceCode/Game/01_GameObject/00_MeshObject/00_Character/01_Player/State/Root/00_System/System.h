#pragma once
#include "Game/01_GameObject/00_MeshObject/00_Character/01_Player/State/PlayerStateBase.h"

class Player;

/**************************************************
*	プレイヤーの強制遷移系の共通処理ステート(基底).
*   このステート自体がインスタンス化されることはない.
*	担当:淵脇 未来.
**/

namespace PlayerState
{
    class System : public PlayerStateBase
    {
    public:
        System(Player* owner);
		~System();

        void Enter() override;
        void Update() override;
        void LateUpdate() override;
        void Draw() override;
        void Exit() override;

    private:
    };
}