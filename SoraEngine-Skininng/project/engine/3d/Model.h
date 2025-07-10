#pragma once
#include "ModelCommon.h"
#include "RenderingData.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


const uint32_t kNumMaxInfluence = 4;//1頂点あたりのボーン行列の数
struct VertexInfluence {
	std::array<float, kNumMaxInfluence> weights;
	std::array<int32_t, kNumMaxInfluence>jointIndices;
};

struct WellForGPU {
	Matrix4x4 skeletonSpaceMatrix;//位置用
	Matrix4x4 skeletonSpaceInverseTransposeMatrix;//法線用
};

struct SkinCluster {

	// 各ジョイントの逆バインドポーズ行列（モデル空間→ボーン空間への変換）
	std::vector<Matrix4x4> inverseBindPoseMatrices;

	// 頂点がどのボーンにどれだけ影響されるかの情報を格納するGPUバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> influenceResource;
	D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
	std::span<VertexInfluence> mappedInfluence;

	// ボーン行列（palette）を格納するGPUリソース（各フレームで更新）
	Microsoft::WRL::ComPtr<ID3D12Resource> paletteResource;
	std::span<WellForGPU> mappedPalette;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;


};

class Model
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ModelCommon* modeleCommon, const std::string& directorypath, const std::string& filename);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	//ノードを読む
	Node ReadNode(aiNode* node);
	Skeleton CreateSkeleton(const Node& rootNode);
	int32_t CreateJoint(const Node& node, std::optional<int32_t> parent, std::vector<Joint>& joints);


	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView()const { return vertexBufferView; }//頂点バッファビューを取得
	ModelData GetModelData() { return modelData; }//モデルデータを取得
	Animation& GetAnimation() { return animation; }//アニメーションデータを取得
	Skeleton& GetSkeleton() { return skeleton; }//スケルトンデータを取得
	SkinCluster& GetSkinCluster() { return skinCluster; }//スキンクラスターを取得


	//ライトのオンオフ
	void SetEnableLighting(bool enable) { materialData->enableLighting = enable; }
	//色の設定
	void SetColor(const Vector4& color) { materialData->color = color; }
	//マテリアルファイルの読み込み
	MaterialData LoadMaterialTemplateFile(const std::string& directorypath, const std::string& filename);
	//モデルファイルの読み込み
	ModelData LoadModelFile(const std::string& ditrectoryPath, const std::string& filename);
	//アニメーションファイルの読み込み	
	Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename);

	SkinCluster CreateSkinCluster();
private:
	//モデル共通部のポインタ
	ModelCommon* modelCommon_ = nullptr;
	//モデル
	//modelファイルのデータ
	ModelData modelData;

	//スキンアニメーション
	//アニメーションデータ
	Animation animation;
	//スケルトン
	Skeleton skeleton;
	//SkinCluster
	SkinCluster skinCluster;

	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	//ばふぁリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	//バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	//マテリアル
	//modelマテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	//マテリアルにデータを書き込む	
	Material* materialData = nullptr;
	//std::string textureFilePath_;


	//index描画
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;//インデックスバッファリソース
	D3D12_INDEX_BUFFER_VIEW indexBufferView;//インデックスバッファビュー
	uint32_t* mappedIndex = nullptr;
};

