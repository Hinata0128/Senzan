#pragma once
#include "../Action.h"

class Player;
class ColliderBase;

/**************************************************
*	プレイヤーの回避の共通処理ステート(基底).
*   このステート自体がインスタンス化されることはない.
*	担当:淵脇 未来.
**/

namespace PlayerState {
    class Dodge : public Action
    {
    public:
        Dodge(Player* owner);
        ~Dodge();

        virtual void Enter() override;
        virtual void Update() override;
        virtual void LateUpdate() override;
        virtual void Draw() override;
        virtual void Exit() override;

    private:
        ColliderBase* m_pDamageDetectionCollider = nullptr; // 回避時に消すためにポインタを保持しておく.

    protected:
        DirectX::XMFLOAT2   m_InputVec; // 入力方向の保存.

        float               m_Distance; // 回避距離.
        float               m_MaxTime;  // 回避時間.
        float               m_currentTime;  // 経過時間.
    };
}