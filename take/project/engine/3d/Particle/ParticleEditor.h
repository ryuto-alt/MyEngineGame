#pragma once
#include "Particle/BaseParticleGroup.h"
#include "Particle/ParticleEmitter.h"
#include "Particle/ParticleManager.h"
#include "Particle/ParticleCommon.h"
#include <string>
#include <vector>
#include <memory>

class ParticleEditor {
public:

	ParticleEditor() = default;
	~ParticleEditor() = default;

	void Initialize(ParticleManager* particleManager,ParticleCommon* particleCommon);
	void Update();
	void UpdateImGui();
	void Finalize();
	void Draw();

private:
	//Preset Management
	//プリセットを保存
	void SavePreset(const std::string& presetName);
	//プリセットを適用
	void LoadPreset(const std::string& presetName);
	//プリセットを削除
	void DeletePreset(const std::string& presetName);

	//デフォルトプリセットを読み込む
	void LoadDefaultPreset();
	//全てのプリセットを読み込む
	void LoadAllPresets();
	//プリセットリストを更新
	void RefreshPresetList();

	//UIの描画
	void DrawParticleAttributesEditor();
	void DrawEmitterControls();
	void DrawPreviewSettings();
	void DrawPresetManager();

private:

	// パーティクルマネージャー
	ParticleManager* particleManager_ = nullptr;
	// パーティクル共通情報
	ParticleCommon* particleCommon_ = nullptr;
	//エディター専用エミッター
	std::unique_ptr<ParticleEmitter> previewEmitter_;

	//エディター設定
	ParticlePreset currentPreset_;
	std::string currentGroupName_;
	std::string selectedPresetName_;

	//プレビュー設定
	EulerTransform emitterTransform_;
	uint32_t emitCount_ = 10;
	float emitFrequency_ = 0.1f;
	bool autoEmit_ = false;
	bool autoApply_ = true; //属性を自動的に適用するかどうか

	// プリセット管理
	std::vector<std::string> presetNames_;
	std::map<std::string, ParticlePreset> presets_;
	// テクスチャファイル名のリスト(参照渡し)
	std::vector<std::string> textureFileNames_; 
};