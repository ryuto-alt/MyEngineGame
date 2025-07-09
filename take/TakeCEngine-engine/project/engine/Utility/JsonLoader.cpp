#include "JsonLoader.h"
#include "Windows.h"

#include <fstream>
#include <cassert>
#include <imgui.h>

//////////////////////////////////////////////////////////////////////////////////////
///			レベルデータファイル読み込み
//////////////////////////////////////////////////////////////////////////////////////

LevelData* JsonLoader::LoadLevelFile(const std::string& groupName) {

	//読み込むJSONファイルのフルパスを合成する
	std::string filePath = kDirectoryPath + groupName + ".json";
	//読み込み用ファイルストリーム
	std::ifstream ifs;
	//ファイルを読み込み用に開く
	ifs.open(filePath);

	//ファイルオープン失敗？
	if (ifs.fail()) {
		//エラーメッセージを出力
		std::string message = "Failed open data file for read.";
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
		return nullptr;

	}

	// JSONファイルから解凍したデータ
	json deserializedJson;

	//解凍する
	ifs >> deserializedJson;

	//正しいレベルデータファイルかチェック
	assert(deserializedJson.is_object());
	assert(deserializedJson.contains("name"));
	assert(deserializedJson["name"].is_string());

	//"name"を文字列として取得
	std::string name = deserializedJson["name"].get<std::string>();
	//正しいレベルデータファイルかチェック
	assert(name.compare("scene") == 0);	

	//TODO:レベルデータ格納用インスタンスを生成
	LevelData* levelData = new LevelData();

	//"object"の全オブジェクト走査
	for (nlohmann::json& object : deserializedJson["objects"]) {
		assert(object.contains("type"));

		//種類を取得
		std::string type = object["type"].get<std::string>();

		//種類によって処理を分岐
		//MESH
		if (type.compare("MESH") == 0) {
			levelData->objects.emplace_back(LevelData::ObjectData{});
			LevelData::ObjectData& objectData= levelData->objects.back();

			//モデルファイル名
			if (object.contains("file_name")) {
				objectData.file_name = object["file_name"].get<std::string>();
			}

			//トランスフォーム情報
			nlohmann::json& transform = object["transform"];
			//translation
			objectData.translation = Vector3(
				transform["translation"][0].get<float>(),
				transform["translation"][1].get<float>(),
				transform["translation"][2].get<float>()
			);
			//rotation
			objectData.rotation = Vector3(
				-transform["rotation"][0].get<float>(),
				-transform["rotation"][1].get<float>(),
				-transform["rotation"][2].get<float>()
			);
			//scale
			objectData.scale =  Vector3(
				transform["scaling"][0].get<float>(),
				transform["scaling"][1].get<float>(),
				transform["scaling"][2].get<float>()
			);
		}
	}

	return levelData;
}

//===============================================================================================
///			パーティクルプリセットの保存
//===============================================================================================

void JsonLoader::SaveParticleAttribute(const std::string& presetName, const ParticleAttributes& attributes) {
	
	std::filesystem::path dirctory(kParticlePresetPath);
	//ディレクトリがなければ作成する
	if (!std::filesystem::exists(dirctory)) {
		std::filesystem::create_directories(dirctory);
	}

	// JSONオブジェクトに変換
	json presetJson = attributes;
	std::string filePath = kParticlePresetPath + presetName + ".json";
	std::ofstream ofs(filePath);

	//ファイルオープンが失敗した場合
	if (ofs.fail()) {
		std::string message = "Failed open particle preset file for write:" + presetName;
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
		return;
	}

	// JSONファイルに書き込む
	ofs << std::setw(4) << presetJson << std::endl;
	// ファイルを閉じる
	ofs.close();
}

void JsonLoader::SaveParticlePreset(const std::string& presetName, const ParticlePreset& preset) {

	std::filesystem::path dirctory(kParticlePresetPath);
	//ディレクトリがなければ作成する
	if (!std::filesystem::exists(dirctory)) {
		std::filesystem::create_directories(dirctory);
	}
	// JSONオブジェクトに変換
	json presetJson = preset;
	std::string filePath = kParticlePresetPath + presetName;
	std::ofstream ofs(filePath);
	//ファイルオープンが失敗した場合
	if (ofs.fail()) {
		std::string message = "Failed open particle preset file for write:" + presetName;
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
		return;
	}
	// JSONファイルに書き込む
	ofs << std::setw(4) << presetJson << std::endl;
	// ファイルを閉じる
	ofs.close();
}

//===============================================================================================
///			パーティクル属性の読み込み
//===============================================================================================

ParticleAttributes JsonLoader::LoadParticleAttribute(const std::string& presetName) const {
	
	std::string filePath = kParticlePresetPath + presetName + ".json";

	std::ifstream ifs(filePath);

	//ファイルオープンが失敗した場合
	if (ifs.fail()) {
		std::string message = "Failed open particle preset file for read:" + presetName;
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
		return ParticleAttributes();
	}

	// JSONファイルから読み込む
	json presetJson;
	ifs >> presetJson;
	ifs.close();

	// JSONからParticleAttributesに変換
	return presetJson.get<ParticleAttributes>();
}

//===============================================================================================
///			パーティクルプリセットの読み込み
//===============================================================================================

ParticlePreset JsonLoader::LoadParticlePreset(const std::string& presetName) const {
	
	std::string filePath = kParticlePresetPath + presetName;
	std::ifstream ifs(filePath);
	//ファイルオープンが失敗した場合
	if (ifs.fail()) {
		std::string message = "Failed open particle preset file for read:" + presetName;
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
		return ParticlePreset();
	}
	// JSONファイルから読み込む
	json presetJson;
	ifs >> presetJson;
	ifs.close();
	// JSONからParticlePresetに変換
	return presetJson.get<ParticlePreset>();
}

//================================================================================================
///			パーティクルプリセットの削除
//================================================================================================

void JsonLoader::DeleteParticlePreset(const std::string& presetName) {

	std::string filePath = kParticlePresetPath + presetName + ".json";
	//ファイルが存在する場合
	if (std::filesystem::exists(filePath)) {
		//ファイルを削除
		std::filesystem::remove(filePath);
	} else {
		std::string message = "Particle preset not found: " + presetName;
		MessageBoxA(nullptr, message.c_str(), "JsonLoader", 0);
		assert(0);
	}
}

//================================================================================================
///			グループの作成
//================================================================================================

std::vector<std::string> JsonLoader::GetParticlePresetList() const {
	
	std::vector<std::string> presetList;

	std::filesystem::path dir(kParticlePresetPath);
	//ディレクトリが存在しない場合は空のベクターを返す
	if (!std::filesystem::exists(dir)) {
		return presetList;
	}

	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		//ファイル拡張子が.jsonである場合
		if (entry.path().extension() == ".json") {
			//ファイル名を取得してリストに追加
			presetList.push_back(entry.path().stem().string());
		}
	}

	return presetList;
}

//================================================================================================
///			パーティクルプリセットが存在するかチェック
//================================================================================================

bool JsonLoader::IsParticlePresetExists(const std::string& presetName) const {
	
	std::string filePath = kParticlePresetPath + presetName + ".json";
	//ファイルが存在するかチェック
	return std::filesystem::exists(filePath);
}