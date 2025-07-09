#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

#include <dxcapi.h>

#include <wrl.h>
#include <cassert>
#include <string>
#include <vector>
#include <iostream>
#include <memory>

#include "Matrix4x4.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "ResourceDataStructure.h"
#include "Mesh/Mesh.h"
#include "SpriteCommon.h"

class DirectXCommon;
class Sprite {
public:

	Sprite() = default;
	~Sprite();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(SpriteCommon* spriteCommon,const std::string& filePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	#ifdef _DEBUG
	void UpdateImGui(const std::string& name);
	#endif // _DEBUG

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	///テクスチャ変更処理
	//void ChangeTexture(const std::string& textureFilePath);
	//テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();
private:

	//頂点データ更新
	void UpdateVertexData();

	
public:
	//================================================================================================
	// getter
	//================================================================================================

	//トランスフォーム取得
	EulerTransform GetTransform() { return transform_; }

	//アンカーポイント取得
	const Vector2& GetAnchorPoint() const { return anchorPoint_; }

	const Vector2& GetTranslate() const { return position_; }

	const float GetRotation() const { return rotation_; }

	const Vector2& GetSize() const { return size_; }

	const bool GetIsFlipX() const { return isFlipX_; }

	const bool GetIsFlipY() const { return isFlipY_; }

	const Vector2& GetTextureLeftTop() const { return textureLeftTop_; }

	const Vector2& GetTextureSize() const { return textureSize_; }
	
	//================================================================================================
	// setter
	//================================================================================================

	//アンカーポイント設定
	void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }

	void SetPosition(const Vector2& position) { position_ = position; }

	void SetRotation(const float rotation) { rotation_ = rotation; }

	void SetSize(const Vector2& size) { size_ = size; }

	void SetIsFlipX(const bool isFlipX) { isFlipX_ = isFlipX; }

	void SetIsFlipY(const bool isFlipY) { isFlipY_ = isFlipY; }

	void SetTextureLeftTop(const Vector2& textureLeftTop) { textureLeftTop_ = textureLeftTop; }

	void SetTextureSize(const Vector2& textureSize) { textureSize_ = textureSize; }

private:

	//SpriteCommon
	SpriteCommon* spriteCommon_ = nullptr;

	//メッシュ
	std::unique_ptr<Mesh> mesh_ = nullptr;

	//filePath
	std::string filePath_;

	//sprite用のTransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//sprite用のTransformationMatrix用の頂点データ
	TransformMatrix* wvpData_ = nullptr;

	//Transform
	EulerTransform transform_{};
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 worldViewProjectionMatrix_;

	Vector2 position_ = { 0.0f,0.0f };
	float rotation_ = 0.0f;
	Vector2 size_ = { 400.0f,200.0f };

	
	Vector2 anchorPoint_ = { 0.0f,0.0f }; //アンカーポイント
	bool isFlipX_ = false; //左右フリップ
	bool isFlipY_ = false; //上下フリップ
	bool adjustSwitch_ = false; //テクスチャサイズ調整スイッチ

	Vector2 textureLeftTop_ = { 0.0f,0.0f  }; //テクスチャの左上座標
	Vector2 textureSize_ = { 100.0f,100.0f }; //テクスチャの切り出しサイズ
};