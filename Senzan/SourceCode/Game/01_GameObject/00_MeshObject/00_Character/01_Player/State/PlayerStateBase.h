#pragma once

#include "System/Utility/StateMachine/StateBase.h"
#include "PlayerStateID.h"

// TODO : 前方宣言でよい
//        が各ステートに書くのめんどくさいからinclude. 
class Player;

namespace PlayerState { 
class Root;
}


/**************************************************
*	プレイヤーの状態の基底(Base).
*   StateBaseを継承してPlayer独自の関数を隠蔽する.
*	担当:淵脇 未来.
**/
class PlayerStateBase : public StateBase<Player>
{
public:
    PlayerStateBase(Player* owner);

    // 取得系.
    constexpr virtual PlayerState::eID GetStateID() const = 0;    // ステートIDの取得.

    Player* GetPlayer() const;   // オーナー(プレイヤー)の取得.
	

protected:

    // 正面へラープ回転.
    void RotetToFront();
        
private:

};
