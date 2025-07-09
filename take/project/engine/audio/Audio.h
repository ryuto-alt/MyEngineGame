#pragma once
#include <xaudio2.h>
#include <wrl.h>
#include <fstream>
#include <string>

#pragma comment(lib, "xaudio2.lib")
class AudioManager {
public:

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	//===================================================================================
	///			構造体
	//===================================================================================

	//チャンクヘッダ
	struct ChunkHeader {
		char id[4]; //チャンクID
		int32_t size; //チャンクサイズ
	};

	//RIFFヘッダチャンク
	struct RiffHeader {
		ChunkHeader chunk; //"RIFF"
		char type[4]; // "WAVE"
	};

	//FMTチャンク
	struct FormatChunk {
		ChunkHeader chunk; //"fmt"
		WAVEFORMATEX fmt; //波形フォーマット
	};

	//音声データ
	struct SoundData {

		//波形フォーマット
		WAVEFORMATEX wfex;
		//バッファの先頭アドレス
		BYTE* pBuffer;
		//バッファのサイズ
		unsigned int bufferSize;
		IXAudio2SourceVoice* pSourceVoice;
	};

public:

	//===================================================================================
	///			publicメンバ関数
	//===================================================================================

	static AudioManager* GetInstance();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();

	/// <summary>
	/// 音声データ読み込み
	/// </summary>
	/// <param name="filename"></param>
	/// <returns></returns>
	SoundData SoundLoadWave(const char* filename);

	/// <summary>
	/// 音声データの開放
	/// </summary>
	/// <param name="soundData"></param>
	void SoundUnload(SoundData* soundData);

	/// <summary>
	/// サウンドの再生
	/// </summary>
	/// <param name="xAudio2"></param>
	/// <param name="soundData"></param>
	void SoundPlayWave(IXAudio2* xAudio2, SoundData& soundData,float volume);

	/// <summary>
	/// 解放処理
	/// </summary>
	void Finalize();

public:

	//===================================================================================
	///			Getter
	//===================================================================================

	IXAudio2* GetXAudio2() const { return xAudio2_.Get(); }

	IXAudio2MasteringVoice* GetMasteringVoice() const { return masteringVoice_; }

private:

	static AudioManager* instance_;

	AudioManager() = default;
	~AudioManager() = default;
	AudioManager(const AudioManager&) = delete;
	AudioManager& operator=(const AudioManager&) = delete;

private:

	ComPtr<IXAudio2> xAudio2_ = nullptr;
	IXAudio2MasteringVoice* masteringVoice_ = nullptr;

};