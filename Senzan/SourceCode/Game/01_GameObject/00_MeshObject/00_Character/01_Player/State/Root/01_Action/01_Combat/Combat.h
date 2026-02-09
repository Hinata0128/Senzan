#pragma once
#include "../Action.h"
#include <vector>
#include <string>

class Player;

/**************************************************
*   プレイヤーの攻撃の共通処理ステート(基底).
*   このステート自体がインスタンス化されることはない.
*   担当:淵脇 未来.
**/

namespace PlayerState
{
    struct ColliderWindow
    {
        float Start = 0.0f;    // 開始時刻(ステート秒).
        float Duration = 0.1f; // 継続時間(秒).
        bool  IsAct = false;           // 起動済み.
        bool  IsEnd = false;           // 終了済み.
    };

    class Combat : public Action
    {
    public:
        Combat(Player* owner);
        virtual ~Combat();

        virtual void Enter() override;
        virtual void Update() override;
        virtual void LateUpdate() override;
        virtual void Draw() override;
        virtual void Exit() override;

        virtual std::string GetSettingsFileName() const { return std::string(); }
        virtual void LoadSettings();

        void AddColliderWindow(float start, float duration);
        void ProcessColliderWindows(float currentTime);
        void RenderColliderWindowsUI(const char* title = "コライダー ウィンドウ設定");

    protected:
        float m_Distance;           // Bossとの距離.
        float m_AnimSpeed;          // アニメーション速度 (倍率).
        float m_MinComboTransTime;  // コンボ確定時に次の攻撃へ移れる最短の時間 (秒).
        float m_currentTime;        // ステートに入ってからの経過時間 (秒).
        float m_ComboStartTime;     // コンボ受付開始時間 (秒).
        float m_ComboEndTime;       // ステートを自然に抜ける時間 (秒).
        bool  m_IsComboAccepted;    // コンボ確定.
        std::vector<ColliderWindow> m_ColliderWindows;  // 当たり判定設定.
    };

} // namespace PlayerState

