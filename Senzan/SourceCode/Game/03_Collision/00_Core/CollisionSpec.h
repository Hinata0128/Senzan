#pragma once

#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <DirectXMath.h>

// 衝突設定構造体.
struct CollisionSpec
{
    using MaskType = uint32_t;

    // 持ち主を識別するタグ。"Player"や"Boss"等でマッピングして
    // 実際のオブジェクト（Transform やクラス）を解決する想定。
    std::string ownerTag; // 例: "Player", "Boss"

    // ボーン追従を使用する場合は true にして boneName を指定。
    bool useBone = false;
    std::string boneName; // ボーン名（useBone == true のとき有効）

    // オフセット座標（ボーン追従でない場合はワールド/ローカルオフセットとして使用）
    std::array<float,3> offset = {0.0f, 0.0f, 0.0f};

    // 回転（オイラー角：度またはラジアンの扱いは呼び出し側で統一）
    std::array<float,3> rotation = {0.0f, 0.0f, 0.0f};

    // 攻撃力
    float attackAmount = 0.0f;

    // 生成から何秒後にアクティブ化するか
    float spawnDelay = 0.0f;

    // アクティブ化から何秒後に非アクティブ化するか（0で無制限）
    float spawnDuration = 0.0f;

    // 半径/高さ（カプセルや球、ボックスのパラメータのうち該当するものを使用）
    float radius = 0.5f;
    float height = 1.0f;

    // マスクは uint32_t を使う（既存の eCollisionGroup と互換でビット演算可能）
    MaskType myMask = 0xFFFFFFFFu;
    MaskType targetMask = 0xFFFFFFFFu;

    // トリガー用フラグ/名前（エフェクトや効果音を識別するために利用）
    std::vector<std::string> triggerTags;

    // ボーンの回転に追従するか（true: 回転も追従、false: 座標のみ追従）
    bool trackRotation = true;

    // 可視化色（デバッグ用）
    DirectX::XMFLOAT4 debugColor = DirectX::XMFLOAT4(1.f, 0.f, 1.f, 1.f);

    // アクティブ開始/終了時間を内部管理するための補助フィールド（実行時用）
    // spawnDelay と spawnDuration を適用した開始・終了タイムスタンプを保持するなどに利用可能
    double runtimeStartTime = 0.0;
    double runtimeEndTime = 10.0;

    // デフォルトコンストラクタ
    CollisionSpec() = default;

    // 簡易コンストラクタ
    CollisionSpec(const std::string& owner,
                  const std::array<float,3>& ofs,
                  float r = 0.5f,
                  float h = 1.0f)
        : ownerTag(owner), offset(ofs), radius(r), height(h)
    {}

    // ヘルパー: ownerTag が Player を指すか
    inline bool IsOwnerPlayer() const noexcept { return ownerTag == "Player"; }
    inline bool IsOwnerBoss() const noexcept { return ownerTag == "Boss"; }

    // ヘルパー: triggerTags に存在するか
    inline bool HasTriggerTag(const std::string& tag) const noexcept
    {
        for (const auto& t : triggerTags) if (t == tag) return true;
        return false;
    }
};

