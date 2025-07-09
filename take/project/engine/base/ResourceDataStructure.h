#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include <string>
#include <vector>
#include <cstdint>
#include <map>
#include <wrl.h>

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

//DirectionalLightのデータ
struct DirectionalLightData {
	Vector4 color_; //ライトの色
	Vector3 direction_; //ライトの向き
	float intensity_; //輝度
};

//PointLightのデータ
struct PointLightData {
	Vector4 color_; //ライトの色
	Vector3 position_; //ライトの位置
	float intensity_; //輝度
	float radius_; //影響範囲
	float decay_; //減衰率
	float padding[2];
};

//SpotLightのデータ
struct SpotLightData {
	Vector4 color_;       //ライトの色
	Vector3 position_;    //ライトの位置
	float intensity_;     //輝度
	Vector3 direction_;   //ライトの向き
	float distance_;      //影響範囲
	float decay_;         //減衰率
	float cosAngle_;     //スポットライトの角度
	float penumbraAngle_; //影のぼかし角度
};

//モデル1個分のマテリアルデータ
struct ModelMaterialData {

	std::string textureFilePath; //テクスチャファイルのパス
	std::string envMapFilePath; //環境マップのパス
	uint32_t srvIndex; //テクスチャのインデックス
};

struct Node {
	QuaternionTransform transform;
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

struct VertexWeightData {
	float weight;
	uint32_t vertexIndex;
};

struct JointWeightData {
	Matrix4x4 inverseBindPoseMatrix;
	std::vector<VertexWeightData> vertexWeights;
};

struct SkinningInfo {
	uint32_t numVertices;
};

//モデル1個分のデータ
struct ModelData {
	std::string fileName; //モデル名
	std::map<std::string, JointWeightData> skinClusterData;
	std::vector<VertexData> vertices;
	SkinningInfo skinningInfoData;
	std::vector<uint32_t> indices;
	ModelMaterialData material;
	Node rootNode;
	bool haveBone = false;
};

//パーティクル用の行列,色データ
struct ParticleForGPU {
	Vector3 translate;
	Vector3 rotate;
	Vector3 scale;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};
//View情報
struct PerView {
	Matrix4x4 viewProjection;
	Matrix4x4 billboardMatrix;
	bool isBillboard;
};

//フレーム情報
struct PerFrame {
	//ゲームを開始してからの時間
	float gameTime;
	//フレームの経過時間
	float deltaTime;
};

//エイリアステンプレート
template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;