#include "Mesh.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include <numbers>

Mesh::~Mesh() {
	material_.reset();
	inputVertexResource_.Reset();
	outputVertexResource_.Reset();
	indexBuffer_.Reset();
}

void Mesh::InitializeMesh(DirectXCommon* dxCommon, const std::string& filePath, const std::string& envMapfilePath) {

	material_ = std::make_unique<Material>();
	material_->Initialize(dxCommon, filePath,envMapfilePath);
	vertexBufferViews_.resize(1);
}


void Mesh::SetVertexBuffers(ID3D12GraphicsCommandList* commandList, UINT startSlot) {
	//頂点バッファビューの設定
	assert(vertexBufferViews_.size() > 0);
	commandList->IASetVertexBuffers(startSlot, static_cast<UINT>(vertexBufferViews_.size()), vertexBufferViews_.data());
}

void Mesh::SetSkinnedVertexBuffer(ID3D12GraphicsCommandList* commandList, UINT startSlot) {

	commandList->IASetVertexBuffers(startSlot, 1, &skinnedVBV_);
}

void Mesh::AddVertexBufferView(D3D12_VERTEX_BUFFER_VIEW vbv) {
	vertexBufferViews_.push_back(vbv);
}

void Mesh::InitializeVertexResourceSphere(ID3D12Device* device) {

	// 頂点数の設定
	uint32_t vertexCount = (kSubdivision * kSubdivision) * 6;

	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kSubdivision); // 経度分割1つ分の角度
	const float kLatEvery = std::numbers::pi_v<float> / static_cast<float>(kSubdivision); // 緯度分割1つ分の角度

	//VertexResource生成
	inputVertexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * vertexCount);

	// リソースの先頭のアドレスから使う
	vertexBufferViews_[0].BufferLocation = inputVertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferViews_[0].SizeInBytes = sizeof(VertexData) * vertexCount;
	// 1頂点あたりのサイズ
	vertexBufferViews_[0].StrideInBytes = sizeof(VertexData);

	//頂点データ
	VertexData* vertexData;
	// 書き込むためのアドレスを取得
	inputVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex; //現在の緯度(シータ)

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery; //現在の経度(ファイ)
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;

			//1枚目の三角形
			//頂点データの入力。基準点a(左下)
			vertexData[start].position.x = cos(lat) * cos(lon);
			vertexData[start].position.y = sin(lat);
			vertexData[start].position.z = cos(lat) * sin(lon);
			vertexData[start].position.w = 1.0f;
			vertexData[start].texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
			vertexData[start].texcoord.y = 1.0f - (static_cast<float>(latIndex) / static_cast<float>(kSubdivision));
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;

			//基準点b1(左上)
			vertexData[start + 1].position.x = cos(lat + kLatEvery) * cos(lon);
			vertexData[start + 1].position.y = sin(lat + kLatEvery);
			vertexData[start + 1].position.z = cos(lat + kLatEvery) * sin(lon);
			vertexData[start + 1].position.w = 1.0f;
			vertexData[start + 1].texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
			vertexData[start + 1].texcoord.y = 1.0f - (static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision));
			vertexData[start + 1].normal.x = vertexData[start + 1].position.x;
			vertexData[start + 1].normal.y = vertexData[start + 1].position.y;
			vertexData[start + 1].normal.z = vertexData[start + 1].position.z;

			//基準点c1(右下)
			vertexData[start + 2].position.x = cos(lat) * cos(lon + kLonEvery);
			vertexData[start + 2].position.y = sin(lat);
			vertexData[start + 2].position.z = cos(lat) * sin(lon + kLonEvery);
			vertexData[start + 2].position.w = 1.0f;
			vertexData[start + 2].texcoord.x = static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision);
			vertexData[start + 2].texcoord.y = 1.0f - (static_cast<float>(latIndex) / static_cast<float>(kSubdivision));
			vertexData[start + 2].normal.x = vertexData[start + 2].position.x;
			vertexData[start + 2].normal.y = vertexData[start + 2].position.y;
			vertexData[start + 2].normal.z = vertexData[start + 2].position.z;

			//基準点d(右上)
			vertexData[start + 3].position.x = cos(lat + kLatEvery) * cos(lon + kLonEvery);
			vertexData[start + 3].position.y = sin(lat + kLatEvery);
			vertexData[start + 3].position.z = cos(lat + kLatEvery) * sin(lon + kLonEvery);
			vertexData[start + 3].position.w = 1.0f;
			vertexData[start + 3].texcoord.x = static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision);
			vertexData[start + 3].texcoord.y = 1.0f - (static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision));
			vertexData[start + 3].normal.x = vertexData[start + 3].position.x;
			vertexData[start + 3].normal.y = vertexData[start + 3].position.y;
			vertexData[start + 3].normal.z = vertexData[start + 3].position.z;

		}
	}

}

void Mesh::InitializeVertexResourceSprite(ID3D12Device* device, Vector2 anchorPoint) {


	//VertexResource生成
	inputVertexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * 4);
	//頂点バッファビューの作成
	vertexBufferViews_[0].BufferLocation = inputVertexResource_->GetGPUVirtualAddress();
	vertexBufferViews_[0].SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferViews_[0].StrideInBytes = sizeof(VertexData);

	//頂点データ
	VertexData* vertexData;
	inputVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//anchorPoint
	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;
	//1枚目の三角形
	vertexData[0].position = { left,bottom,0.0f,1.0f }; //左下
	vertexData[1].position = { left,top,0.0f,1.0f }; //左上
	vertexData[2].position = { right,bottom,0.0f,1.0f }; //右下
	vertexData[3].position = { right,top,0.0f,1.0f }; //右上
	vertexData[0].texcoord = { 0.0f,1.0f };
	vertexData[1].texcoord = { 0.0f,0.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };
	vertexData[3].texcoord = { 1.0f,0.0f };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };
}

void Mesh::InitializeVertexResourceTriangle(ID3D12Device* device) {

	//VertexResource生成
	inputVertexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * 3);

	// リソースの先頭のアドレスから使う
	vertexBufferViews_[0].BufferLocation = inputVertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferViews_[0].SizeInBytes = sizeof(VertexData) * 3;
	// 1頂点あたりのサイズ
	vertexBufferViews_[0].StrideInBytes = sizeof(VertexData);

	//頂点データ
	VertexData* vertexData;
	// 書き込むためのアドレスを取得
	inputVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	//左下
	vertexData[0].position = { -0.5f,-0.5f,0.0f,1.0f };
	vertexData[0].texcoord = { 0.0f,1.0f };
	//上
	vertexData[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexData[1].texcoord = { 0.5f,0.0f };
	//右下
	vertexData[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexData[2].texcoord = { 1.0f,1.0f };

}

//=============================================================================
// objモデルの頂点バッファリソース初期化
//=============================================================================

void Mesh::InitializeInputVertexResourceModel(ID3D12Device* device, ModelData* modelData) {


	//頂点リソースを作る
	inputVertexResource_ = DirectXCommon::CreateBufferResource(device, sizeof(VertexData) * modelData->vertices.size());
	

	//頂点バッファビューを作る
	vertexBufferViews_[0].BufferLocation = inputVertexResource_->GetGPUVirtualAddress();
	vertexBufferViews_[0].SizeInBytes = UINT(sizeof(VertexData) * modelData->vertices.size());
	vertexBufferViews_[0].StrideInBytes = sizeof(VertexData);

	//リソースにデータを書き込む
	VertexData* vertexData;
	inputVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData->vertices.data(), sizeof(VertexData) * modelData->vertices.size());
}

void Mesh::InitializeOutputVertexResourceModel(ID3D12Device* device, ModelData* modelData, ID3D12GraphicsCommandList* commandList) {
	//頂点リソースを作る
	outputVertexResource_ = DirectXCommon::CreateBufferResourceUAV(device, sizeof(VertexData) * modelData->vertices.size(),commandList);
	outputVertexResource_->SetName(L"Mesh::outputVertexResource_");
	//頂点バッファビューを作る
	vertexBufferViews_[0].BufferLocation = outputVertexResource_->GetGPUVirtualAddress();
	vertexBufferViews_[0].SizeInBytes = UINT(sizeof(VertexData) * modelData->vertices.size());
	vertexBufferViews_[0].StrideInBytes = sizeof(VertexData);
}

void Mesh::InitializeIndexResourceSphere(ID3D12Device* device) {

	// インデックスバッファのサイズを設定
	uint32_t indexCount = (kSubdivision * kSubdivision) * 6;
	indexBuffer_ = DirectXCommon::CreateBufferResource(device, sizeof(uint32_t) * indexCount);

	// インデックスバッファビューの設定
	indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * indexCount;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;

			// 最初の三角形のインデックス
			indexData[start] = start;         //左下
			indexData[start + 1] = start + 1; //左上
			indexData[start + 2] = start + 2; //右下

			// 二つ目の三角形のインデックス
			indexData[start + 3] = start + 1; //左上
			indexData[start + 4] = start + 3; //右上
			indexData[start + 5] = start + 2; //右下
		}
	}
}

void Mesh::InitializeIndexResourceSprite(ID3D12Device* device) {

	//リソースの作成
	indexBuffer_ = DirectXCommon::CreateBufferResource(device, sizeof(uint32_t) * 6);
	//リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	//インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;
}

//================================================================================================
// モデルのIndexResourceの初期化
//================================================================================================

void Mesh::InitializeIndexResourceModel(ID3D12Device* device, ModelData* modelData) {

	indexBuffer_ = DirectXCommon::CreateBufferResource(device, sizeof(uint32_t) * modelData->indices.size());
	indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = UINT(sizeof(uint32_t) * modelData->indices.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexData = nullptr;
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, modelData->indices.data(), sizeof(uint32_t) * modelData->indices.size());
}

void Mesh::MapInputVertexResource(ModelData* modelData) {
	//リソースにデータを書き込む
	VertexData* vertexData;
	inputVertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData->vertices.data(), sizeof(VertexData) * modelData->vertices.size());
}
