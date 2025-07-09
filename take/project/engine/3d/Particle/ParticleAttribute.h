#pragma once
#include "math/Vector3.h"
#include "Primitive/PrimitiveType.h"
#include <json.hpp>
#include <numbers>
#include <string>
#include <cstdint>
#include <unordered_map>

enum class ParticleModelType {
	Primitive,
	ExternalModel
};

struct AttributeRange {
	float min;
	float max;
};

enum class ScaleSetting {
	None = 0, //スケールの更新なし
	ScaleUp = 1, //スケールの更新(拡大)
	ScaleDown = 2, //スケールの更新(縮小)
};

// パーティクルの属性を保持する構造体
struct ParticleAttributes {
	Vector3 scale = { 1.0f,1.0f,1.0f };
	Vector3 color = { 1.0f,1.0f,1.0f };
	AttributeRange scaleRange = { 0.1f,3.0f };
	AttributeRange rotateRange = { -std::numbers::pi_v<float>, std::numbers::pi_v<float> };
	AttributeRange positionRange = {-1.0f, 1.0f};
	AttributeRange velocityRange = { -1.0f,1.0f };
	AttributeRange colorRange = { 0.0f,1.0f };
	AttributeRange lifetimeRange = { 1.0f,3.0f };
	float frequency; //パーティクルの発生頻度
	uint32_t emitCount; //1回あたりのパーティクル発生数
	bool isBillboard = false; //Billboardかどうか
	bool editColor = false; //色を編集するかどうか
	bool isTraslate_ = false; //位置を更新するかどうか
	uint32_t scaleSetting_;    //スケールの更新処理方法
	bool enableFollowEmitter_ = false; //エミッターに追従するかどうか
};

struct ParticlePreset {
	std::string presetName; //プリセットの名前
	ParticleAttributes attribute; //属性のマップ
	std::string textureFilePath; //テクスチャファイル名
	PrimitiveType primitiveType; //プリミティブの種類
	Vector3 primitiveParameters = {1.0f,1.0f,1.0f}; //プリミティブのパラメータ
};

using json = nlohmann::json;

// JSON形式に変換
void to_json(json& j, const ParticleAttributes& attributes);
void to_json(json& j, const AttributeRange& attributeRange);
void to_json(json& j, const ParticlePreset& preset);

// JSONから変換
void from_json(const json& j, ParticleAttributes& attributes);
void from_json(const json& j, AttributeRange& attributeRange);
void from_json(const json& j, ParticlePreset& preset);