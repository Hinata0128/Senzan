#pragma once
#include "../Combat.h"

class Player;

/**************************************************
*	プレイヤーの攻撃二段目のステート(派生).
*	担当:淵脇 未来.
**/

namespace PlayerState {
class AttackCombo_1 final : public Combat
{
public:
    AttackCombo_1(Player* owner);
    ~AttackCombo_1();

    // IDの取得.
    constexpr PlayerState::eID GetStateID() const override;

    // 設定項目設置パス.
    std::string GetSettingsFileName() const override {
        return "Data\\Json\\Player\\AttackCombo\\AttackCombo_1.json";
    }

    void Enter() override;
    void Update() override;
    void LateUpdate() override;
    void Draw() override;
    void Exit() override;

private:
};

} // namespace PlayerState
