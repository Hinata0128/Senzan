#pragma once
#include "../Combat.h"

class Player;

/**************************************************
*	プレイヤーの攻撃三段目のステート(派生).
*	担当:淵脇 未来.
**/

namespace PlayerState {
class AttackCombo_2 final
    : public Combat
{
public:
    AttackCombo_2(Player* owner);
    ~AttackCombo_2();

    // IDの取得.
    constexpr PlayerState::eID GetStateID() const override;

    // 設定項目設置パス.
    // 親クラスの戻り値の型と一致させる（トップレベル const を削除）。
    std::string GetSettingsFileName() const override {
        return "Data\\Json\\Player\\AttackCombo\\AttackCombo_2.json";
    }

    void Enter() override;
    void Update() override;
    void LateUpdate() override;
    void Draw() override;
    void Exit() override;

private:
};

} // namespace PlayerState
