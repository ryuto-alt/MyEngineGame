#include "../Audio/Audio.h"
#include <cassert>

AudioManager* AudioManager::instance_ = nullptr;

//================================================================================================
// 初期化処理
//================================================================================================

AudioManager* AudioManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new AudioManager();
	}
	return instance_;
}

void AudioManager::Initialize() {

	//XAudio2オブジェクトの生成
	HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));

	//マスターボイスの生成
	hr = xAudio2_->CreateMasteringVoice(&masteringVoice_);
	assert(SUCCEEDED(hr));
}

//================================================================================================
// 音声データ読み込み
//================================================================================================

AudioManager::SoundData AudioManager::SoundLoadWave(const char* filename) {

#pragma region ファイルオープン
	//ファイル入力ストリームのインスタンス
	std::ifstream file;
	//.wavファイルをバイナリモードで開く
	file.open(filename, std::ios::binary);
	//ファイルオープン失敗を検出
	assert(file.is_open());
#pragma endregion

#pragma region .wavデータの読み込み
	//RIFFヘッダの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	//ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	//ファイルがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	//FMTチャンクの読み込み
	FormatChunk format = {};
	//チャンクヘッダの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	//チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	//Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	//JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur); //読み取り位置をチャンクの終わりまで進める
		file.read((char*)&data, sizeof(data));     //再読み込み
	}

	//LISTチャンクを検出した場合
	if (strncmp(data.id, "LIST", 4) == 0) {
		file.seekg(data.size, std::ios_base::cur);
		file.read((char*)&data, sizeof(data));
	}

	//Dataチャンクかチェック
	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	//Dataチャンクのデータ部（波形データ）の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	//waveファイルを閉じる
	file.close();

#pragma endregion

#pragma region 読み込んだ音声データをreturn

	SoundData soundData;
	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

#pragma endregion

	return soundData;
}

//================================================================================================
// 音声データの開放
//================================================================================================

void AudioManager::SoundUnload(SoundData* soundData) {
	// 再生中のボイスを停止して破棄
	if (soundData->pSourceVoice) {
		soundData->pSourceVoice->Stop();
		soundData->pSourceVoice->DestroyVoice();
		soundData->pSourceVoice = nullptr;
	}

	//バッファの解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

//================================================================================================
// 音声再生
//================================================================================================

void AudioManager::SoundPlayWave(IXAudio2* xAudio2, SoundData& soundData, float volume) {

	HRESULT result;

	//波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	soundData.pSourceVoice = pSourceVoice;

	//音量の設定
	pSourceVoice->SetVolume(volume);

	//再生する波形データの設定
	XAUDIO2_BUFFER buffer = {};
	buffer.pAudioData = soundData.pBuffer;
	buffer.AudioBytes = soundData.bufferSize;
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	//波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buffer);
	result = pSourceVoice->Start();
}

//================================================================================================
// 解放処理
//================================================================================================

void AudioManager::Finalize() {
	//XAudio2の解放
	xAudio2_.Reset();
}