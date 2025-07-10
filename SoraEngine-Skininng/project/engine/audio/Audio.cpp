#include "Audio.h"
#include <wrl.h>

Audio* Audio::instance_ = nullptr;

Audio* Audio::GetInstance()
{
    if (instance_ == nullptr) {
        instance_ = new Audio();
    }
    return instance_;
}

void Audio::Initialize()
{
    HRESULT hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    assert(SUCCEEDED(hr));

    hr = xAudio2->CreateMasteringVoice(&masterVoice);
    assert(SUCCEEDED(hr));
}

void Audio::Finalize()
{
    // すべての再生中の音を停止
    for (auto& [soundData, voice] : activeVoices) {
        if (voice) {
            voice->Stop();
            voice->DestroyVoice();
        }
    }
    activeVoices.clear();

    xAudio2.Reset();
    delete instance_;
}

SoundData Audio::SoundLoadWave(const char* filename)
{
    std::ifstream file(filename, std::ios_base::binary);
    assert(file.is_open());

    RiffHeader riff;
    file.read((char*)&riff, sizeof(riff));
    assert(strncmp(riff.chunk.id, "RIFF", 4) == 0);
    assert(strncmp(riff.type, "WAVE", 4) == 0);

    FormatChunk format = {};
    file.read((char*)&format, sizeof(ChunkHeader));
    assert(strncmp(format.chunk.id, "fmt ", 4) == 0);
    assert(format.chunk.size <= sizeof(format.fmt));
    file.read((char*)&format.fmt, format.chunk.size);

    ChunkHeader data;
    file.read((char*)&data, sizeof(data));

    if (strncmp(data.id, "JUNK", 4) == 0 || strncmp(data.id, "LIST", 4) == 0) {
        file.seekg(data.size, std::ios_base::cur);
        file.read((char*)&data, sizeof(data));
    }

    assert(strncmp(data.id, "data", 4) == 0);

    char* pbuffer = new char[data.size];
    file.read(pbuffer, data.size);
    file.close();

    SoundData soundData = {};
    soundData.wfex = format.fmt;
    soundData.PBuffer = reinterpret_cast<BYTE*>(pbuffer);
    soundData.bufferSize = data.size;

    return soundData;
}

void Audio::SoundUnload(SoundData* soundData)
{
    StopSpecificAudio(soundData);
    delete[] soundData->PBuffer;
    soundData->PBuffer = nullptr;
    soundData->bufferSize = 0;
    soundData->wfex = {};
}

void Audio::SoundPlayWave(const SoundData& soundData)
{
    HRESULT hr;

    StopSpecificAudio(const_cast<SoundData*>(&soundData));

    IXAudio2SourceVoice* newVoice = nullptr;
    hr = xAudio2->CreateSourceVoice(&newVoice, &soundData.wfex);
    assert(SUCCEEDED(hr));

    XAUDIO2_BUFFER buf{};
    buf.pAudioData = soundData.PBuffer;
    buf.AudioBytes = soundData.bufferSize;
    buf.Flags = XAUDIO2_END_OF_STREAM;

    hr = newVoice->SubmitSourceBuffer(&buf);
    hr = newVoice->Start();

    activeVoices[const_cast<SoundData*>(&soundData)] = newVoice;
}

void Audio::StopAudio()
{
    for (auto& [soundData, voice] : activeVoices) {
        if (voice) {
            voice->Stop();
            voice->DestroyVoice();
        }
    }
    activeVoices.clear();
}

void Audio::StopSpecificAudio(SoundData* soundData)
{
    auto it = activeVoices.find(soundData);
    if (it != activeVoices.end()) {
        IXAudio2SourceVoice* voice = it->second;
        if (voice) {
            voice->Stop();
            voice->DestroyVoice();
        }
        activeVoices.erase(it);
    }
}

void Audio::PauseAudio()
{
    for (auto& [soundData, voice] : activeVoices) {
        if (voice) {
            voice->Stop();
        }
    }
}

void Audio::PauseSpecificAudio(SoundData* soundData)
{
    auto it = activeVoices.find(soundData);
    if (it != activeVoices.end()) {
        IXAudio2SourceVoice* voice = it->second;
        if (voice) {
            voice->Stop();
        }
    }
}

void Audio::ResumeAudio()
{
    for (auto& [soundData, voice] : activeVoices) {
        if (voice) {
            voice->Start();
        }
    }
}

void Audio::ResumeSpecificAudio(SoundData* soundData)
{
    auto it = activeVoices.find(soundData);
    if (it != activeVoices.end()) {
        IXAudio2SourceVoice* voice = it->second;
        if (voice) {
            voice->Start();
        }
    }
}

void Audio::SetPlaybackSpeed(float speed)
{
    for (auto& [soundData, voice] : activeVoices) {
        if (voice) {
            HRESULT hr = voice->SetFrequencyRatio(speed);
            assert(SUCCEEDED(hr));
        }
    }
}

void Audio::SetPlaybackSpeed(SoundData* soundData, float speed)
{
    auto it = activeVoices.find(soundData);
    if (it != activeVoices.end()) {
        IXAudio2SourceVoice* voice = it->second;
        if (voice) {
            HRESULT hr = voice->SetFrequencyRatio(speed);
            assert(SUCCEEDED(hr));
        }
    }
}

bool Audio::IsSoundPlaying() const
{
    for (const auto& [soundData, voice] : activeVoices) {
        if (voice) {
            XAUDIO2_VOICE_STATE state;
            voice->GetState(&state);
            if (state.BuffersQueued > 0) {
                return true;
            }
        }
    }
    return false;
}



bool Audio::IsSoundPlaying(SoundData* soundData) const
{
    auto it = activeVoices.find(soundData);
    if (it != activeVoices.end()) {
        IXAudio2SourceVoice* voice = it->second;
        if (voice) {
            XAUDIO2_VOICE_STATE state;
            voice->GetState(&state);
            return state.BuffersQueued > 0;
        }
    }
    return false;
}

void Audio::SetVolume(float volume)
{
    for (auto& [soundData, voice] : activeVoices) {
        if (voice) {
            voice->SetVolume(volume);
        }
    }
}

void Audio::SetVolume(SoundData* soundData, float volume)
{
    auto it = activeVoices.find(soundData);
    if (it != activeVoices.end()) {
        IXAudio2SourceVoice* voice = it->second;
        if (voice) {
            voice->SetVolume(volume);
        }
    }
}
