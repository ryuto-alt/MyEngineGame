#pragma once

#include "ResourceDataStructure.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "Animation/Animator.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include "Mesh/Mesh.h"
#include "base/PipelineStateObject.h"
#include "base/SrvManager.h"
#include "base/DirectXCommon.h"
#include "3d/ModelCommon.h"

#include <d3d12.h>
#include <wrl.h>
#include <memory>

class Model {
public:

	Model();
	~Model();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ModelCommon* ModelCommon,ModelData* modelData);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(Animation* animation, float animationTime);

	//void UpdateSkeleton();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	void DrawSkyBox();

	void DisPatch(PSO* skinningPso);

	//void DisPatchForASkinningModel();


	/// <summary>
	/// パーティクル描画
	/// </summary>
	/// <param name="instanceCount_"></param>
	void DrawForParticle(UINT instanceCount_);

	void DrawForGPUParticle(UINT instanceCount);

	/// <summary>
	/// アニメーションの適用
	/// </summary>
	//void ApplyAnimation();


public: //ゲッター

	//メッシュの取得
	Mesh* GetMesh() { return mesh_.get(); }

	//スケルトンの取得
	Skeleton* GetSkeleton() { return skeleton_.get(); }

	//ModelDataの取得
	ModelData* GetModelData() { return modelData_; }

	//ModelCommonの取得
	ModelCommon* GetModelCommon() { return modelCommon_; }

	//テクスチャファイルパスの取得
	const std::string& GetTextureFilePath() const { return modelData_->material.textureFilePath; }

	//ローカル行列の取得
	const Matrix4x4& GetLocalMatrix() const { return localMatrix_; }

public: //セッター

	//メッシュの設定
	void SetMesh(Mesh* mesh) { mesh_.reset(mesh); }

	//ModelCommonの設定
	void SetModelCommon(ModelCommon* modelCommon) { modelCommon_ = modelCommon; }

private:

	ModelCommon* modelCommon_ = nullptr;

	//メッシュ
	std::unique_ptr<Mesh> mesh_ = nullptr;
	//スケルトン
	std::unique_ptr<Skeleton> skeleton_;
	//スキンクラスター
	SkinCluster skinCluster_;

	//構築するModelData
	ModelData* modelData_;
	//Animation* animation_;

	Vector3 translate_;
	Quaternion rotate_;
	Vector3 scale_;
	Matrix4x4 localMatrix_;

	uint32_t uavIndex_ = 0;

	bool haveSkeleton_ = true;

	uint32_t inputIndex_ = 0;
};