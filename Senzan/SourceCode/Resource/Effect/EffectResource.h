#pragma once
#include "System/Singleton/SingletonTemplate.h" // パスは環境に合わせてください
#include "../../../Data/Library/Effekseer/include/Effekseer.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <filesystem>

// ライブラリのリンク設定
#ifdef _DEBUG
#pragma comment( lib, "Effekseerd.lib" )
#pragma comment( lib, "EffekseerRendererDX11d.lib" )
#else
#pragma comment( lib, "Effekseer.lib" )
#pragma comment( lib, "EffekseerRendererDX11.lib" )
#endif

class EffectResource final : public Singleton<EffectResource>
{
public:
    EffectResource();
    ~EffectResource();

    // 読み込み関数（ResourceManagerから呼ばれる想定）
    bool Create();   // 初期化が必要な場合
    bool LoadData(); // 全データ読み込み

    // エフェクトの取得
    static ::Effekseer::EffectRef GetResource(const std::string& name);

private:
    // エフェクトデータのマップ（名前, エフェクト本体）
    std::unordered_map<std::string, ::Effekseer::EffectRef> m_pEffects;
};
