#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include "ResourceDataStructure.h"
#include "camera/Camera.h"
#include "3d/Model.h"
#include "Animation/Animator.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "PSOType.h"


class Object3dCommon;
class Object3d {
public:

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	Object3d() = default;
	~Object3d();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Update();

	void AnimationUpdate();

	#ifdef _DEBUG
	void UpdateImGui(const std::string& name);
	#endif // _DEBUG

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	void DisPatch();

public: //getter

	EulerTransform GetTransform() const { return transform_; }
	Vector3 GetScale() const { return transform_.scale; }
	Vector3 GetRotation() const { return transform_.rotate; }
	Vector3 GetTranslate() const { return transform_.translate; }
	Vector3 GetCenterPosition() const;
	Model* GetModel() { return model_.get(); }

	//Animationの取得
	Animation* GetAnimation() { return animation_; }
	float GetDuration() { return animation_->duration; }
	float GetAnimationTime() { return animationTime_; }

public: //setter

	void SetCamera(Camera* camera) { camera_ = camera; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotation(const Vector3& rotation) { transform_.rotate = rotation; }
	void SetTranslate(const Vector3& position) { transform_.translate = position; }
	void SetAnimation(Animation* animation);
	void SetParent(Object3d* parent) { parent_ = parent; }
	void SetModelFilePath(const std::string& filePath) { modelFilePath_ = filePath; }

protected: // privateメンバ変数

	Object3dCommon* object3dCommon_ = nullptr;

	//ペアレント
	const Object3d* parent_ = nullptr;

	//モデル
	std::unique_ptr<Model> model_ = nullptr;
	//モデルファイルパス
	std::string modelFilePath_;

	Animation* animation_;
	float animationTime_ = 0.0f;
	bool isAnimation_ = true;

	//TransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用の頂点データ
	TransformMatrix* TransformMatrixData_ = nullptr;

	//Transform
	EulerTransform transform_{};
	//TransformMatrix
	Matrix4x4 worldMatrix_;
	Matrix4x4 WVPMatrix_;
	Matrix4x4 WorldInverseTransposeMatrix_;
	//Camera
	Camera* camera_ = nullptr;

};