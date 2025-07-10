#include "Sprite.h"
#include "SpriteCommon.h"
//#include "MyMath.h"

#include"TextureManager.h"

#include "Matrix4x4.h"
#include <MyMath.h>
#include <CameraManager.h>

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath)
{
	textureFilePath_ = textureFilePath;

	//Texturを読んで転送する
	TextureManager::GetInstance()->LoadTexture(textureFilePath);

	spriteCommon_ = spriteCommon;
	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

	vetexResource = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * 4);
	indexResource = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * 6);
	materialResource = spriteCommon->GetDxCommon()->CreateBufferResource(sizeof(MaterialSprite));
	transformationMatrixResource = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrixsprite));


	//リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vetexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	//1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	//書き込むためのアドレスを取得


	//Index
	//リソース先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス６つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;


	//Material
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//色
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	

	materialData->uvTransform = materialData->uvTransform.MakeIdentity4x4();


	//Trandformation
	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformaitionMatrixData));
	//単位行列を書き込んでおく

	transformaitionMatrixData->WVP = transformaitionMatrixData->WVP.MakeIdentity4x4();
	transformaitionMatrixData->World = transformaitionMatrixData->World.MakeIdentity4x4();

	//カメラforGPU
	cameraResource = SpriteCommon::GetInstance()->GetDxCommon()->CreateBufferResource(sizeof(CaMeraForGpu));
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGpu));
	cameraForGpu->worldPosition = { 0.0f,0.0f,0.0f };

	//画像のサイズに合わせる
	AdjustTextureSize();


}



void Sprite::Update()
{

	Camera* activeCamera = CameraManager::GetInstance()->GetActiveCamera();
	Vector3 cameraPosition = activeCamera->GetTransform().translate;
	cameraForGpu->worldPosition = cameraPosition;

	transform.rotate = { 0.0f,0.0f,rotation };
	transform.translate = { position.x,position.y,0.0f };
	transform.scale = { size.x,size.y,1.0f, };

	//アンカーポイント
	float left = 0.0f - anchorPoint_.x;
	float right = 1.0f - anchorPoint_.x;
	float top = 0.0f - anchorPoint_.y;
	float bottom = 1.0f - anchorPoint_.y;

	//フリップ
	//左右反転
	if (isFlipX_) {

		left = -left;
		right = -right;

	}
	//上下反転
	if (isFlipY_) {

		top = -top;
		bottom = -bottom;

	}

	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);
	float tex_left = textureLeftTop_.x / metadata.width;
	float tex_right = (textureLeftTop_.x + textureSize_.x) / metadata.width;
	float tex_top = textureLeftTop_.y / metadata.height;
	float tex_bottom = (textureLeftTop_.y + textureSize_.y) / metadata.height;

	vetexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//一個目
	vertexData[0].position = { left,bottom,0.0f,1.0f };	//左下
	vertexData[1].position = { left,top,0.0f,1.0f };	//左上
	vertexData[2].position = { right,bottom,0.0f,1.0f };//右下
	vertexData[3].position = { right,top,0.0f,1.0f };	//右上

	vertexData[0].texcoord = { tex_left,tex_bottom }; //左下
	vertexData[1].texcoord = { tex_left,tex_top };		  //左上
	vertexData[2].texcoord = { tex_right,tex_bottom };//右下
	vertexData[3].texcoord = { tex_right,tex_top };	  //右上

	vertexData[0].normal = { 0.0f,0.0f,-1.0f };//左下	
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };//左上
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };//右下
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };//右上

	//インデックスリソースにデータ書き込む
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;


	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	projectionMatrix = MyMath::MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWindth), float(WinApp::kClientHeight), 0.0f, 100.0f);
	worldViewProjectionMatrix = worldMatrix * viewMatrix.MakeIdentity4x4() * projectionMatrix;
	transformaitionMatrixData->WVP = worldViewProjectionMatrix;
	transformaitionMatrixData->World = worldMatrix;



}

void Sprite::Draw()
{
	// sprite用の描画
	spriteCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
	spriteCommon_->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	// MaterialのCBVを設定 (RootParameter[0])
	spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

	// テクスチャのSRVを設定 (RootParameter[1])
	spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(1, TextureManager::GetInstance()->GetSrvHandleGPU(textureFilePath_));
	// TransformationMatrixのCBVを設定 (RootParameter[2])
	spriteCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(2, transformationMatrixResource->GetGPUVirtualAddress());
	// インデックス付き描画

	spriteCommon_->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);


}

void Sprite::AdjustTextureSize()
{
	//テクスチャメタデータを取得
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetaData(textureFilePath_);
	//テクスチャ切り出しサイズ

	textureSize_ = { static_cast<float>(metadata.width),static_cast<float>(metadata.height) };

	//画像サイズをテクスチャサイズに合わせる
	size = textureSize_;
}
