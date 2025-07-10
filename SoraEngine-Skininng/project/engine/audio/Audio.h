#pragma once
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include <wrl.h>
#include <fstream>
#include <cassert>
#include <unordered_map>

struct ChunkHeader {
    char id[4];
    int32_t size;
};

struct RiffHeader {
    ChunkHeader chunk;
    char type[4];
};

struct FormatChunk {
    ChunkHeader chunk;
    WAVEFORMATEX fmt;
};

struct SoundData {
    WAVEFORMATEX wfex; // 波形フォーマット
    BYTE* PBuffer; // バッファの先頭アドレス
    unsigned int bufferSize; // バッファのサイズ
};

class Audio {
    static Audio* instance_;
    Audio() = default;
    ~Audio() = default;
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

public:
    template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
    static Audio* GetInstance();
    void Initialize();
    void Finalize();

    // Waveファイルの読み込み・解放
    SoundData SoundLoadWave(const char* filename);
    void SoundUnload(SoundData* soundData);

    // 音の再生・停止
    void SoundPlayWave(const SoundData& soundData);
    void StopAudio(); // 全音停止
    void StopSpecificAudio(SoundData* soundData); // 指定音停止

    // 一時停止・再開
    void PauseAudio(); // 全音一時停止
    void PauseSpecificAudio(SoundData* soundData); // 指定音一時停止
    void ResumeAudio(); // 全音再開
    void ResumeSpecificAudio(SoundData* soundData); // 指定音再開

    // 再生速度の設定
    void SetPlaybackSpeed(float speed); // 全音の速度変更
    void SetPlaybackSpeed(SoundData* soundData, float speed); // 指定音の速度変更

    // **音量調整**
    void SetVolume(float volume); // 全ての音の音量変更
    void SetVolume(SoundData* soundData, float volume); // 指定音の音量変更

    // 状態取得
    bool IsSoundPlaying() const; // 何か音が鳴っているか
    bool IsSoundPlaying(SoundData* soundData) const; // 指定音が鳴っているか

private:
    ComPtr<IXAudio2> xAudio2 = nullptr;
    IXAudio2MasteringVoice* masterVoice = nullptr;

    // 複数の SourceVoice を管理
    std::unordered_map<SoundData*, IXAudio2SourceVoice*> activeVoices;
};
