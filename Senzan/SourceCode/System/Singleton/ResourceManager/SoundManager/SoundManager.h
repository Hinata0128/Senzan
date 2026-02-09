#pragma once
#include "System/Singleton/SingletonTemplate.h"
#include "Resource/Sound/DirectSound.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

class DirectSound;

class SoundManager final 
    : public Singleton<SoundManager>
{
public:
    SoundManager();
    ~SoundManager();

    // 音声データの一括読み込み (ResourceManagerから呼ばれる)
    static bool LoadSounds(HWND hWnd);

    // 操作系
    static void Play(const std::string& name, bool isLoop = false);
    static void Stop(const std::string& name);
    static void AllStop();
    static void SetVolume(const std::string& name, int volume);
    // 現在再生中のBGMの音量を下げる (0-10000 の範囲)
    static void LowerCurrentBGMVolume(int newVolume);
    // 一時的にBGMの音量をフェードで下げ、保持してからフェードで戻す
    // fadeDownSeconds: 下げるフェード時間
    // holdSeconds: 保持時間
    // fadeUpSeconds: 戻すフェード時間
    static void LowerCurrentBGMVolumeTemporarily(int newVolume, float fadeDownSeconds, float holdSeconds, float fadeUpSeconds);
    // 毎フレーム呼び出して自動復帰処理を行う
    static void Update(float deltaTime);

    // リスト取得
    static const std::vector<std::string>& GetKeyList();

private:
    // 内部での読み込み処理
    bool LoadAllInternal(HWND hWnd);
    void UpdateKeyList();

private:
    LPDIRECTSOUND8 m_lpSoundInterface = nullptr;
    // メモリ管理のため unique_ptr に変更
    std::unordered_map<std::string, std::unique_ptr<DirectSound>> m_pDxSounds;
    std::vector<std::string> m_keys;
    // 再生中のBGM名 (空なら無し)
    std::string m_currentBGMName;
    // 再生中の効果音バッファ（重複再生のために一時的に保持）
    std::vector<LPDIRECTSOUNDBUFFER> m_activeSEBuffers;
    // 自動復帰用
    int m_prevBGMVolume = 10000;
    // フェード用
    bool m_fadeActive = false;
    enum class FadePhase { Idle = 0, FadingDown, Holding, FadingUp };
    FadePhase m_fadePhase = FadePhase::Idle;
    int m_fadeTargetVolume = 10000;
    int m_fadeStartVolume = 10000;
    float m_fadeTimer = 0.0f; // current phase timer
    float m_fadeDownDuration = 0.0f;
    float m_fadeHoldDuration = 0.0f;
    float m_fadeUpDuration = 0.0f;
};
