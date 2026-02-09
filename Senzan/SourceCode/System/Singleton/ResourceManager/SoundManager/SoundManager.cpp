#include "SoundManager.h"
#include <filesystem>
#include <algorithm>

namespace {
constexpr char SOUND_DIRECTORY_PATH[] = "Data\\Sound";
}

SoundManager::SoundManager() : m_lpSoundInterface(nullptr) {}

SoundManager::~SoundManager() {
    m_pDxSounds.clear();
    if (m_lpSoundInterface) {
        m_lpSoundInterface->Release();
        m_lpSoundInterface = nullptr;
    }
}

bool SoundManager::LoadSounds(HWND hWnd) {
    return GetInstance().LoadAllInternal(hWnd);
}

bool SoundManager::LoadAllInternal(HWND hWnd) {
    // インターフェース生成
    if (FAILED(DirectSoundCreate8(NULL, &m_lpSoundInterface, NULL))) return false;
    if (FAILED(m_lpSoundInterface->SetCooperativeLevel(hWnd, DSSCL_NORMAL))) return false;

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(SOUND_DIRECTORY_PATH)) {
            std::string ext = entry.path().extension().string();
            if (ext != ".wav" && ext != ".WAV") continue;

            std::string name = entry.path().stem().string();
            std::wstring path = entry.path().wstring();

            auto sound = std::make_unique<DirectSound>();
            if (sound->Init(path, m_lpSoundInterface)) {
                m_pDxSounds[name] = std::move(sound);
            }
        }
    }
    catch (...) {
        return false;
    }

    UpdateKeyList();
    return true;
}

void SoundManager::Play(const std::string& name, bool isLoop) {
#if _SOUND_STOP_DEBUG == 1
    return;
#endif

    auto& inst = GetInstance();
    auto& sounds = inst.m_pDxSounds;
    if (sounds.count(name)) {
        // SEならピッチをランダムに変化させる.
        if (!isLoop) {
            auto ds = sounds[name].get();
            DWORD originalFreq = ds->GetOriginalFrequency();
            if (originalFreq != 0) {
                float factor = 1.0f;
                int r = std::rand();
                float rnd = (static_cast<float>(r) / static_cast<float>(RAND_MAX));
                float variation = (rnd * 0.16f * 5.f) - 0.08f * 5.f;
                factor = 1.0f + variation;
                DWORD newFreq = static_cast<DWORD>(originalFreq * factor);
                ds->SetFrequency(newFreq);
            }
        }
        // Play may return an extra buffer when overlapping SE is created
        LPDIRECTSOUNDBUFFER extraBuf = sounds[name]->Play(isLoop);
        if (extraBuf != nullptr)
        {
            // store to active list for cleanup when finished
            inst.m_activeSEBuffers.push_back(extraBuf);
        }
        // ループ再生ならBGMとして記録
        if (isLoop) inst.m_currentBGMName = name;
    }
}

void SoundManager::Stop(const std::string& name) {
    auto& sounds = GetInstance().m_pDxSounds;
    if (sounds.count(name)) sounds[name]->Stop();
}

void SoundManager::AllStop() {
    for (auto& pair : GetInstance().m_pDxSounds) {
        pair.second->Stop();
        pair.second->ResetPosition();
    }
}

void SoundManager::SetVolume(const std::string& name, int volume) {
    auto& sounds = GetInstance().m_pDxSounds;
    if (sounds.count(name)) sounds[name]->SetVolume(volume);
}

void SoundManager::LowerCurrentBGMVolume(int newVolume)
{
    auto& inst = GetInstance();
    if (inst.m_currentBGMName.empty()) return;

    auto it = inst.m_pDxSounds.find(inst.m_currentBGMName);
    if (it == inst.m_pDxSounds.end()) return;

    // Clamp volume
    if (newVolume < 0) newVolume = 0;
    if (newVolume > 10000) newVolume = 10000;

    it->second->SetVolume(newVolume);
}

void SoundManager::UpdateKeyList() {
    m_keys.clear();
    m_keys.reserve(m_pDxSounds.size());
    for (const auto& pair : m_pDxSounds) {
        m_keys.push_back(pair.first);
    }
}

const std::vector<std::string>& SoundManager::GetKeyList() {
    return GetInstance().m_keys;
}

// (old two-parameter overload removed)

void SoundManager::LowerCurrentBGMVolumeTemporarily(int newVolume, float fadeDownSeconds, float holdSeconds, float fadeUpSeconds)
{
    auto& inst = GetInstance();
    if (inst.m_currentBGMName.empty()) return;

    auto it = inst.m_pDxSounds.find(inst.m_currentBGMName);
    if (it == inst.m_pDxSounds.end()) return;

    // clamp
    if (newVolume < 0) newVolume = 0;
    if (newVolume > 10000) newVolume = 10000;

    inst.m_prevBGMVolume = it->second->GetVolume();
    inst.m_fadeStartVolume = inst.m_prevBGMVolume;
    inst.m_fadeTargetVolume = newVolume;
    inst.m_fadeDownDuration = fadeDownSeconds;
    inst.m_fadeHoldDuration = holdSeconds;
    inst.m_fadeUpDuration = fadeUpSeconds;
    inst.m_fadeTimer = 0.0f;
    inst.m_fadePhase = FadePhase::FadingDown;
    inst.m_fadeActive = true;
}

void SoundManager::Update(float deltaTime)
{
    // クリーンアップ: 再生が終了した一時バッファを解放
    for (auto it = GetInstance().m_activeSEBuffers.begin(); it != GetInstance().m_activeSEBuffers.end(); )
    {
        LPDIRECTSOUNDBUFFER buf = *it;
        if (!buf) { it = GetInstance().m_activeSEBuffers.erase(it); continue; }
        DWORD status = 0;
        HRESULT hr = buf->GetStatus(&status);
        if (FAILED(hr) || !(status & DSBSTATUS_PLAYING))
        {
            buf->Stop();
            buf->Release();
            it = GetInstance().m_activeSEBuffers.erase(it);
        }
        else
        {
            ++it;
        }
    }

    auto& inst = GetInstance();
    if (!inst.m_fadeActive) return;

    inst.m_fadeTimer += deltaTime;
    auto it = inst.m_pDxSounds.find(inst.m_currentBGMName);
    if (inst.m_fadePhase == SoundManager::FadePhase::FadingDown)
    {
        float t = (inst.m_fadeDownDuration > 0.0f) ? (inst.m_fadeTimer / inst.m_fadeDownDuration) : 1.0f;
        if (t >= 1.0f) {
            // ensure target
            if (it != inst.m_pDxSounds.end()) it->second->SetVolume(inst.m_fadeTargetVolume);
            inst.m_fadePhase = SoundManager::FadePhase::Holding;
            inst.m_fadeTimer = 0.0f;
        } else {
            int cur = inst.m_fadeStartVolume + static_cast<int>((inst.m_fadeTargetVolume - inst.m_fadeStartVolume) * t);
            if (it != inst.m_pDxSounds.end()) it->second->SetVolume(cur);
        }
    }
    else if (inst.m_fadePhase == SoundManager::FadePhase::Holding)
    {
        if (inst.m_fadeTimer >= inst.m_fadeHoldDuration) {
            inst.m_fadePhase = SoundManager::FadePhase::FadingUp;
            inst.m_fadeTimer = 0.0f;
        }
    }
    else if (inst.m_fadePhase == SoundManager::FadePhase::FadingUp)
    {
        float t = (inst.m_fadeUpDuration > 0.0f) ? (inst.m_fadeTimer / inst.m_fadeUpDuration) : 1.0f;
        if (t >= 1.0f) {
            // restore
            if (it != inst.m_pDxSounds.end()) it->second->SetVolume(inst.m_prevBGMVolume);
            inst.m_fadePhase = SoundManager::FadePhase::Idle;
            inst.m_fadeActive = false;
            inst.m_fadeTimer = 0.0f;
        } else {
            int cur = inst.m_fadeTargetVolume + static_cast<int>((inst.m_prevBGMVolume - inst.m_fadeTargetVolume) * t);
            if (it != inst.m_pDxSounds.end()) it->second->SetVolume(cur);
        }
    }
}
