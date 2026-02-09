#pragma once
#include "../Combat.h"

class Player;

/**************************************************
*	プレイヤーの攻撃一段目のステート(派生).
*	担当:淵脇 未来.
**/

namespace PlayerState {
class AttackCombo_0 final
    : public Combat
{
public:
    AttackCombo_0(Player* owner);
    ~AttackCombo_0();

    // IDの取得.
    constexpr PlayerState::eID GetStateID() const override;

    // 設定項目設置パス.
    std::string GetSettingsFileName() const override {
        return "Data\\Json\\Player\\AttackCombo\\AttackCombo_0.json";
    }

    void Enter() override;
    void Update() override;
    void LateUpdate() override;
    void Draw() override;
    void Exit() override;

private:    
};

} // namespace PlayerState

