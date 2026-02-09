#pragma once
#include "../System.h"
#include <DirectXMath.h>
#include <memory>

class Player;
class AfterImage;

/**************************************************
*	プレイヤーの必殺技のステート(派生).
*	担当:淵脇 未来.
**/

namespace PlayerState {

    class SpecialAttack final : public System
    {
    public:
        SpecialAttack(Player* owner);
        ~SpecialAttack();

        // IDの取得.
        constexpr  PlayerState::eID GetStateID() const override;

        void Enter() override;
        void Update() override;
        void LateUpdate() override;
        void Draw() override;
        void Exit() override;

    private:
        // 往復攻撃の更新
        void UpdateSlashRush(float deltaTime);
        // 残像の生成
        void SpawnAfterImage();

    private:
        float m_CurrentTime = 0.0f;       // 経過時間
        float m_AttackDuration;           // 攻撃演出時間


        float m_OraOraDamage = 40.0f;     // 連続ダメージ
        float m_AttackDamage = 300.0f;    // 最終段ダメージ

        // 往復斬撃設定
        enum class SlashPhase { TowardsBoss, FinalSlash, FinalPose, Finished };
        SlashPhase m_SlashPhase = SlashPhase::TowardsBoss;
        
        int m_TotalSlashes = 10;          // 往復回数
        int m_CurrentSlash = 0;           // 現在の斬撃回数
        float m_SlashSpeed = 100.0f;      // 移動速度（一定）
        float m_SlashDistance = 4.0f;     // ボスからの距離（より近く）
        float m_ReturnDistance = 15.0f;   // 戻る距離（短く）
        float m_FinalPoseDistance = 10.0f; // 最終ポーズ時のボス背後距離
        
        
        DirectX::XMFLOAT3 m_StartPos;     // 開始位置
        DirectX::XMFLOAT3 m_TargetPos;    // 目標位置
        float m_SlashTimer = 0.0f;        // 斬撃タイマー
        float m_SlashInterval = 0.12f;    // 斬撃間隔
        bool m_IsMoving = false;          // 移動中か
        float m_CurrentAngle = 0.0f;      // 現在の攻撃角度
        float m_FinalPoseTimer = 0.0f;    // 最終ポーズタイマー
        
        // 最終ポーズ固定用
        DirectX::XMFLOAT3 m_FinalPosePosition;  // 最終ポーズ時の固定座標
        float m_FinalPoseRotationY = 0.0f;      // 最終ポーズ時の固定Y回転

        // 残像用
        std::unique_ptr<AfterImage> m_pAfterImage;  // 残像エフェクト
        float m_AfterImageTimer = 0.0f;             // 残像生成タイマー
        float m_AfterImageInterval = 0.05f;         // 残像生成間隔（長め）
    };
}
