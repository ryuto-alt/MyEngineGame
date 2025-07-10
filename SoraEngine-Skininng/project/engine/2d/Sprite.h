#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "RenderingData.h"

#include <assert.h>
#include <cmath>
#include <stdio.h>
#include <string>
//#include <Windows.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <Camera.h>



class SpriteCommon;
class Sprite
{

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(SpriteCommon* spriteCommon, std::string textureFilePath);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();


	//大きさ
	const Vector2& GetSize()const { return size; }
	void SetSize(const Vector2& size) { this->size = size; }
	//位置
	const Vector2& GetPosition()const { return position; }
	void SetPosition(const Vector2& position) { this->position = position; }
	//回転
	const float& GetRotation()const { return rotation; }
	void SetRotation(const float& rotation) { this->rotation = rotation; }
	//色
	const Vector4& GetColor()const { return materialData->color; }
	void setColor(const Vector4& color) { materialData->color = color; }
	//アンカー_画像の中心

	const Vector2& GetAnchorPoint()const { return anchorPoint_; }

	void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }
	//左右フリップ
	const bool& GetIsFlipX()const { return isFlipX_; }
	void SetIsFlipX(const bool& isFlipX) { isFlipX_ = isFlipX; }
	//上下フリップ
	const bool& GetIsFlipY()const { return isFlipY_; }
	void SetIsFlipY(const bool& isFlipY) { isFlipY_ = isFlipY; }
	//テクスチャ左上
	const Vector2& GetTextureLeftTop()const { return textureLeftTop_; }

	void SetTextureLeftTop(const Vector2& textureLeftTop) { textureLeftTop_ = textureLeftTop; }
	//テクスチャサイズ
	const Vector2& GetTextureSize()const { return textureSize_; }
	void SetTextureSize(const Vector2& textureSize) { textureSize_ = textureSize; }

private:

	std::string textureFilePath_;

	//テクスチャサイズをイメージに合わせる
	void AdjustTextureSize();

	SpriteCommon* spriteCommon_ = nullptr;

	//バッファリソース
	//Sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vetexResource;
	//Sprite用のindexResourceを作成						
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource;
	//Sprite用のマテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	//transformation用のリソースを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource;

	//バッファー内のデータを示すポインタ
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	MaterialSprite* materialData = nullptr;
	TransformationMatrixsprite* transformaitionMatrixData = nullptr;

	////vetexResourceSprite頂点バッファーを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	//IndexBufferSprite頂点バッファーを作成する
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	//Transform
	EulerTransform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };

	//設定用SRT
	Vector2 size = { 640.0f,360.0f };
	Vector2 position = { 0.0f,0.0f };
	float rotation = 0.0f;

	uint32_t textureIndex = 0;

	//アンカーポイント 中心位置を変えれる
	Vector2 anchorPoint_ = { 0.0f,0.0f };

	//フリップ
	bool isFlipX_ = false;//左右フリップ
	bool isFlipY_ = false;//上下フリップ

	//テクスチャ左上座標
	Vector2 textureLeftTop_ = { 0.0f,0.0f };
	//テクスチャ切り出しサイズ
	Vector2 textureSize_ = { 512.0f,512.0f };


	Matrix4x4 worldMatrix;
	Matrix4x4 viewMatrix;
	Matrix4x4 projectionMatrix;
	Matrix4x4 worldViewProjectionMatrix;


	Camera* camera = nullptr;
	//カメラforGPU
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;//カメラのデータを送るためのリソース
	CaMeraForGpu* cameraForGpu = nullptr;//カメラのデータをGPUに送るための構造体



};

