#pragma once
#include "Particle/ParticleAttribute.h"
#include "scene/LevelData.h"
#include <iostream>
#include <variant>
#include <string>
#include <map>
#include <json.hpp>

#include "Vector3.h"

class JsonLoader {
public:

	using json = nlohmann::json;

	JsonLoader() = default;
	~JsonLoader() = default;

	/// <summary>
	/// ファイルから読み込む
	/// </summary>
	/// <param name="groupName">グループ名</param>
	LevelData* LoadLevelFile(const std::string& groupName);

	//=============================================================================================
	/// Particle Preset Functions
	//=============================================================================================

	// パーティクルプリセットの保存
	void SaveParticleAttribute(const std::string& presetName, const ParticleAttributes& attributes);
	void SaveParticlePreset(const std::string& presetName, const ParticlePreset& preset);

	// パーティクルプリセットの読み込み
	ParticleAttributes LoadParticleAttribute(const std::string& presetName) const;
	ParticlePreset LoadParticlePreset(const std::string& presetName) const;

	// パーティクルプリセットの削除
	void DeleteParticlePreset(const std::string& presetName);

	// パーティクルプリセットの一覧を取得
	std::vector<std::string> GetParticlePresetList() const;

	// パーティクルプリセットが存在するかチェック
	bool IsParticlePresetExists(const std::string& presetName) const;

private:

	//グローバル変数の保存先ファイルパス
	const std::string kDirectoryPath = "Resources/JsonLoader/";
	
	//パーティクルプリセットの保存先
	const std::string kParticlePresetPath = "Resources/JsonLoader/ParticlePresets/";
};

