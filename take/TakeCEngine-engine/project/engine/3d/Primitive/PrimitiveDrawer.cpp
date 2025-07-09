#include "PrimitiveDrawer.h"
#include "CameraManager.h"
#include "TextureManager.h"
#include "Object3dCommon.h"
#include "ImGuiManager.h"
#include "MatrixMath.h"
#include "Particle/Particle3d.h"
#include <numbers>

void PrimitiveDrawer::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {

	dxCommon_ = dxCommon;
	srvManager_ = srvManager;

}

void PrimitiveDrawer::Finalize() {

	// 各プリミティブデータの解放
	for (auto& pair : ringDatas_) {
		pair.second->primitiveData_.vertexBuffer_.Reset();
		pair.second->primitiveData_.indexBuffer_.Reset();
		delete pair.second->material_;
	}
	for (auto& pair : planeDatas_) {
		pair.second->primitiveData_.vertexBuffer_.Reset();
		pair.second->primitiveData_.indexBuffer_.Reset();
		delete pair.second->material_;
	}
	for (auto& pair : sphereDatas_) {
		pair.second->primitiveData_.vertexBuffer_.Reset();
		pair.second->primitiveData_.indexBuffer_.Reset();
		delete pair.second->material_;
	}
	ringDatas_.clear();
	planeDatas_.clear();
	sphereDatas_.clear();
}

void PrimitiveDrawer::Update() {

}

void PrimitiveDrawer::UpdateImGui(uint32_t handle, PrimitiveType type) {

	switch (type) {
	case PRIMITIVE_RING:
	{
		auto it = ringDatas_.find(handle);
		if (it != ringDatas_.end()) {
			auto& ringData = it->second;
			ImGui::Begin("Ring Data");
			ImGui::Text("Outer Radius: %.2f", ringData->outerRadius_);
			ImGui::Text("Inner Radius: %.2f", ringData->innerRadius_);
			ImGui::End();
		}
		break;
	}
	case PRIMITIVE_PLANE:
	{
		auto it = planeDatas_.find(handle);
		if (it != planeDatas_.end()) {
			auto& planeData = it->second;
			ImGui::Begin("Plane Data");
			ImGui::Text("Width: %.2f", planeData->width_);
			ImGui::Text("Height: %.2f", planeData->height_);
			ImGui::End();
		}
		break;
	}
	case PRIMITIVE_SPHERE:
	{
		auto it = sphereDatas_.find(handle);
		if (it != sphereDatas_.end()) {
			auto& sphereData = it->second;
			ImGui::Begin("Sphere Data");
			ImGui::Text("Radius: %.2f", sphereData->radius_);
			ImGui::End();
		}
		break;
	}
	default:
		assert(0 && "未対応の PrimitiveType が指定されました");
		break;
	}
}

void PrimitiveDrawer::UpdateImGui(uint32_t handle, PrimitiveType type, const Vector3& param) {

	switch (type) {
	case PRIMITIVE_RING:
	{
		auto it = ringDatas_.find(handle);
		if (it != ringDatas_.end()) {
			auto& ringData = it->second;
			ringData->outerRadius_ = param.x;
			ringData->innerRadius_ = param.y;
			ImGui::Begin("Ring Data");
			ImGui::Text("Outer Radius: %.2f", ringData->outerRadius_);
			ImGui::Text("Inner Radius: %.2f", ringData->innerRadius_);
			ImGui::End();
		}
		break;
	}
	case PRIMITIVE_PLANE:
	{
		auto it = planeDatas_.find(handle);
		if (it != planeDatas_.end()) {
			auto& planeData = it->second;
			planeData->width_ = param.x;
			planeData->height_ = param.y;
			ImGui::Begin("Plane Data");
			ImGui::Text("Width: %.2f", planeData->width_);
			ImGui::Text("Height: %.2f", planeData->height_);
			ImGui::End();
		}
		break;
	}
	case PRIMITIVE_SPHERE:
	{
		auto it = sphereDatas_.find(handle);
		if (it != sphereDatas_.end()) {
			auto& sphereData = it->second;
			sphereData->radius_ = param.x;
			ImGui::Begin("Sphere Data");
			ImGui::Text("Radius: %.2f", sphereData->radius_);
			ImGui::End();
		}
		break;
	}
	default:
		assert(0 && "未対応の PrimitiveType が指定されました");
		break;
	}
}

uint32_t PrimitiveDrawer::GenerateRing(const float outerRadius, const float innerRadius, const std::string& textureFilePath) {

	auto ring = std::make_unique<RingData>();
	ring->innerRadius_ = innerRadius;
	ring->outerRadius_ = outerRadius;
	CreateRingVertexData(ring.get());
	CreateRingMaterial(textureFilePath, ring.get());
	uint32_t useHandle = ringHandle_++;
	ringDatas_[useHandle] = std::move(ring);
	return useHandle;
}

uint32_t PrimitiveDrawer::GeneratePlane(const float width, const float height, const std::string& textureFilePath) {

	auto plane = std::make_unique<PlaneData>();
	plane->width_ = width;
	plane->height_ = height;
	CreatePlaneVertexData(plane.get());
	CreatePlaneMaterial(textureFilePath, plane.get());
	uint32_t useHandle = planeHandle_++;
	planeDatas_[useHandle] = std::move(plane);
	return useHandle;
}

uint32_t PrimitiveDrawer::GenerateSphere(const float radius, const std::string& textureFilePath) {
	
	auto sphere = std::make_unique<SphereData>();
	sphere->radius_ = radius;
	CreateSphereVertexData(sphere.get());
	CreateSphereMaterial(textureFilePath, sphere.get());
	uint32_t useHandle = sphereHandle_++;
	sphereDatas_[useHandle] = std::move(sphere);
	return useHandle;
}

void PrimitiveDrawer::DrawParticle(PSO* pso, UINT instanceCount, PrimitiveType type, uint32_t handle) {

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();


	switch (type) {
	case PRIMITIVE_RING:
	{

		//--------------------------------------------------
		//		ringの描画
		//--------------------------------------------------

		auto itRing = ringDatas_.find(handle);
		if (itRing == ringDatas_.end()) {
			return; // handleが無効な場合は何もしない
		}

		auto& ringData = itRing->second;

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//VertexBufferView
		commandList->IASetVertexBuffers(0, 1, &ringData->primitiveData_.vertexBufferView_);
		// materialResource
		commandList->SetGraphicsRootConstantBufferView(
			pso->GetGraphicBindResourceIndex("gMaterial"), ringData->material_->GetMaterialResource()->GetGPUVirtualAddress());
		// texture
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(ringData->material_->GetTextureFilePath()));

		//描画
		commandList->DrawInstanced(ringVertexCount_, instanceCount, 0, 0);

		break;
	}
	case PRIMITIVE_PLANE:
	{

		//--------------------------------------------------
		//		planeの描画
		//--------------------------------------------------

		auto itPlane = planeDatas_.find(handle);
		if (itPlane == planeDatas_.end()) return;

		auto& planeData = itPlane->second;

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//VertexBufferView
		commandList->IASetVertexBuffers(0, 1, &planeData->primitiveData_.vertexBufferView_);
		// materialResource
		commandList->SetGraphicsRootConstantBufferView(
			pso->GetGraphicBindResourceIndex("gMaterial"), planeData->material_->GetMaterialResource()->GetGPUVirtualAddress());
		// texture
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(planeData->material_->GetTextureFilePath()));
		//描画
		commandList->DrawInstanced(planeVertexCount_, instanceCount, 0, 0);

		break;
	}
	case PRIMITIVE_SPHERE:
	{
		//--------------------------------------------------
		//		sphereの描画
		//--------------------------------------------------

		auto itSphere = sphereDatas_.find(handle);
		if (itSphere == sphereDatas_.end()) return;

		auto& sphereData = itSphere->second;

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//VertexBufferView
		commandList->IASetVertexBuffers(0, 1, &sphereData->primitiveData_.vertexBufferView_);
		// materialResource
		commandList->SetGraphicsRootConstantBufferView(
			pso->GetGraphicBindResourceIndex("gMaterial"), sphereData->material_->GetMaterialResource()->GetGPUVirtualAddress());
		// texture
		srvManager_->SetGraphicsRootDescriptorTable(
			pso->GetGraphicBindResourceIndex("gTexture"), TextureManager::GetInstance()->GetSrvIndex(sphereData->material_->GetTextureFilePath()));

		//IBVの設定
		//commandList->IASetIndexBuffer(&sphereData->primitiveData_.indexBufferView_);

		//描画
		commandList->DrawInstanced(sphereVertexCount_, instanceCount, 0, 0);

		break;
	}

	case PRIMITIVE_COUNT:
		break;
	default:
		assert(0 && "未対応の PrimitiveType が指定されました");
		break;
	}
}

void PrimitiveDrawer::CreateRingVertexData(RingData* ringData) {

	UINT size = sizeof(VertexData) * ringDivide_ * kMaxVertexCount_;

	//bufferをカウント分確保
	ringData->primitiveData_.vertexBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	ringData->primitiveData_.vertexBuffer_->SetName(L"Ring::vertexResource_");
	//bufferview設定
	ringData->primitiveData_.vertexBufferView_.BufferLocation = ringData->primitiveData_.vertexBuffer_->GetGPUVirtualAddress();
	ringData->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	ringData->primitiveData_.vertexBufferView_.SizeInBytes = size;
	//mapping
	ringData->primitiveData_.vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&ringData->vertexData_));

	const float radianPerDivide = (2.0f * std::numbers::pi_v<float>) / static_cast<float>(ringDivide_);
	uint32_t ringVertexIndex = 0;

	for (uint32_t index = 0; index < ringDivide_; ++index) {
		float sin = std::sin(index * radianPerDivide);
		float cos = std::cos(index * radianPerDivide);
		float sinNext = std::sin((index + 1) * radianPerDivide);
		float cosNext = std::cos((index + 1) * radianPerDivide);
		float uv = static_cast<float>(index) / static_cast<float>(ringDivide_);
		float uvNext = static_cast<float>(index + 1) / static_cast<float>(ringDivide_);

		ringData->vertexData_[ringVertexIndex].position = { ringData->outerRadius_ * -sin,ringData->outerRadius_ * cos, 0.0f, 1.0f }; // outer[i]
		ringData->vertexData_[ringVertexIndex + 1].position = { ringData->innerRadius_ * -sin,     ringData->innerRadius_ * cos,    0.0f, 1.0f }; // inner[i]
		ringData->vertexData_[ringVertexIndex + 2].position = { ringData->innerRadius_ * -sinNext, ringData->innerRadius_ * cosNext,0.0f, 1.0f }; // inner[i+1]
		ringData->vertexData_[ringVertexIndex + 3].position = ringData->vertexData_[ringVertexIndex].position; // outer[i]
		ringData->vertexData_[ringVertexIndex + 4].position = ringData->vertexData_[ringVertexIndex + 2].position; // inner[i+1]
		ringData->vertexData_[ringVertexIndex + 5].position = { ringData->outerRadius_ * -sinNext, ringData->outerRadius_ * cosNext, 0.0f, 1.0f }; // outer[i+1]

		ringData->vertexData_[ringVertexIndex].texcoord = { uv, 0.0f };     // outer[i]
		ringData->vertexData_[ringVertexIndex + 1].texcoord = { uv, 1.0f };     // inner[i]
		ringData->vertexData_[ringVertexIndex + 2].texcoord = { uvNext, 1.0f }; // inner[i+1]
		ringData->vertexData_[ringVertexIndex + 3].texcoord = { uv, 0.0f };     // outer[i]
		ringData->vertexData_[ringVertexIndex + 4].texcoord = { uvNext, 1.0f }; // inner[i+1]
		ringData->vertexData_[ringVertexIndex + 5].texcoord = { uvNext, 0.0f }; // outer[i+1]

		ringData->vertexData_[ringVertexIndex].normal = { 0.0f, 0.0f, 1.0f };
		ringData->vertexData_[ringVertexIndex + 1].normal = { 0.0f, 0.0f, 1.0f };
		ringData->vertexData_[ringVertexIndex + 2].normal = { 0.0f, 0.0f, 1.0f };
		ringData->vertexData_[ringVertexIndex + 3].normal = { 0.0f, 0.0f, 1.0f };
		ringData->vertexData_[ringVertexIndex + 4].normal = { 0.0f, 0.0f, 1.0f };
		ringData->vertexData_[ringVertexIndex + 5].normal = { 0.0f, 0.0f, 1.0f };

		ringVertexIndex += 6;
	}

	ringVertexCount_ += ringDivide_ * 6;
}

void PrimitiveDrawer::CreatePlaneVertexData(PlaneData* planeData) {

	UINT size = sizeof(VertexData) * 6 * kMaxVertexCount_;
	//bufferをカウント分確保
	planeData->primitiveData_.vertexBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	planeData->primitiveData_.vertexBuffer_->SetName(L"Plane::vertexResource_");
	//bufferview設定
	planeData->primitiveData_.vertexBufferView_.BufferLocation = planeData->primitiveData_.vertexBuffer_->GetGPUVirtualAddress();
	planeData->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	planeData->primitiveData_.vertexBufferView_.SizeInBytes = size;
	//mapping
	planeData->primitiveData_.vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&planeData->vertexData_));

	//Planeの頂点データを生成(6頂点分)
	planeData->vertexData_[0].position = { -planeData->width_,  planeData->height_,0.0f, 1.0f }; //左下
	planeData->vertexData_[1].position = { -planeData->width_, -planeData->height_,0.0f, 1.0f }; //左上
	planeData->vertexData_[2].position = { planeData->width_,  planeData->height_,0.0f, 1.0f }; //右下
	planeData->vertexData_[3].position = { -planeData->width_, -planeData->height_,0.0f, 1.0f }; //左上
	planeData->vertexData_[4].position = { planeData->width_, -planeData->height_,0.0f, 1.0f }; //右上
	planeData->vertexData_[5].position = { planeData->width_,  planeData->height_,0.0f, 1.0f }; //右下
	//PlaneのUV座標を生成
	planeData->vertexData_[0].texcoord = { 0.0f, 1.0f };
	planeData->vertexData_[1].texcoord = { 0.0f, 0.0f };
	planeData->vertexData_[2].texcoord = { 1.0f, 1.0f };
	planeData->vertexData_[3].texcoord = { 0.0f, 0.0f };
	planeData->vertexData_[4].texcoord = { 1.0f, 0.0f };
	planeData->vertexData_[5].texcoord = { 1.0f, 1.0f };
	//Planeの法線を生成
	planeData->vertexData_[0].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData_[1].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData_[2].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData_[3].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData_[4].normal = { 0.0f, 0.0f, -1.0f };
	planeData->vertexData_[5].normal = { 0.0f, 0.0f, -1.0f };

	//Planeの頂点の
	planeVertexCount_ += 6;
}

void PrimitiveDrawer::CreateSphereVertexData(SphereData* sphereData) {

	auto CreateSphereVertex = [&](const Vector3& pos, uint32_t lonIndex, uint32_t latIndex, uint32_t kSubdivision, float radius) {
		VertexData v{};
		v.position = { pos.x, pos.y, pos.z, 1.0f };
		v.normal = { pos.x / radius, pos.y / radius, pos.z / radius };
		v.texcoord.x = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
		v.texcoord.y = 1.0f - (static_cast<float>(latIndex) / static_cast<float>(kSubdivision));
		return v;
	};

	const uint32_t kSubdivision = sphereDivide_;
	const float kLatEvery = std::numbers::pi_v<float> / static_cast<float>(kSubdivision);         // 緯度刻み
	const float kLonEvery = 2.0f * std::numbers::pi_v<float> / static_cast<float>(kSubdivision);  // 経度刻み

	// 頂点数 = 1マス6頂点 × マス数
	uint32_t vertexCount = kSubdivision * kSubdivision * 6;
	UINT size = static_cast<UINT>(sizeof(VertexData) * vertexCount);

	// bufferを確保
	sphereData->primitiveData_.vertexBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), size);
	sphereData->primitiveData_.vertexBuffer_->SetName(L"Sphere::vertexResource_");

	// bufferview設定
	sphereData->primitiveData_.vertexBufferView_.BufferLocation = sphereData->primitiveData_.vertexBuffer_->GetGPUVirtualAddress();
	sphereData->primitiveData_.vertexBufferView_.StrideInBytes = sizeof(VertexData);
	sphereData->primitiveData_.vertexBufferView_.SizeInBytes = size;

	// mapping
	sphereData->primitiveData_.vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&sphereData->vertexData_));

	VertexData* vertexData = sphereData->vertexData_;
	uint32_t vertexIndex = 0;

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery;

			// 各点の緯度・経度
			float latNext = lat + kLatEvery;
			float lonNext = lon + kLonEvery;

			// a: 左下
			Vector3 aPos = {
				sphereData->radius_ * cosf(lat) * cosf(lon),
				sphereData->radius_ * sinf(lat),
				sphereData->radius_ * cosf(lat) * sinf(lon)
			};

			// b: 左上
			Vector3 bPos = {
				sphereData->radius_ * cosf(latNext) * cosf(lon),
				sphereData->radius_ * sinf(latNext),
				sphereData->radius_ * cosf(latNext) * sinf(lon)
			};

			// c: 右下
			Vector3 cPos = {
				sphereData->radius_ * cosf(lat) * cosf(lonNext),
				sphereData->radius_ * sinf(lat),
				sphereData->radius_ * cosf(lat) * sinf(lonNext)
			};

			// d: 右上
			Vector3 dPos = {
				sphereData->radius_ * cosf(latNext) * cosf(lonNext),
				sphereData->radius_ * sinf(latNext),
				sphereData->radius_ * cosf(latNext) * sinf(lonNext)
			};

			// 三角形1 (a, b, c)
			vertexData[vertexIndex++] = CreateSphereVertex(aPos, lonIndex,     latIndex,     kSubdivision, sphereData->radius_);
			vertexData[vertexIndex++] = CreateSphereVertex(bPos, lonIndex,     latIndex + 1, kSubdivision, sphereData->radius_);
			vertexData[vertexIndex++] = CreateSphereVertex(cPos, lonIndex + 1, latIndex,     kSubdivision, sphereData->radius_);

			// 三角形2 (b, d, c)
			vertexData[vertexIndex++] = CreateSphereVertex(bPos, lonIndex,     latIndex + 1, kSubdivision, sphereData->radius_);
			vertexData[vertexIndex++] = CreateSphereVertex(dPos, lonIndex + 1, latIndex + 1, kSubdivision, sphereData->radius_);
			vertexData[vertexIndex++] = CreateSphereVertex(cPos, lonIndex + 1, latIndex,     kSubdivision, sphereData->radius_);
		}
	}

	sphereVertexCount_ += vertexIndex;
}


void PrimitiveDrawer::CreateRingMaterial(const std::string& textureFilePath, RingData* ringData) {

	ringData->material_ = new Material();
	ringData->material_->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	ringData->material_->SetEnableLighting(false);
	ringData->material_->SetEnvCoefficient(0.0f);
	ringData->material_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });
	ringData->material_->SetUvScale({ 2.5f, 0.5f,1.0f }); // UVスケールを設定
}

void PrimitiveDrawer::CreatePlaneMaterial(const std::string& textureFilePath, PlaneData* planeData) {

	planeData->material_ = new Material();
	planeData->material_->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	planeData->material_->SetEnableLighting(false);
	planeData->material_->SetEnvCoefficient(0.0f);
	planeData->material_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });
}

void PrimitiveDrawer::CreateSphereMaterial(const std::string& textureFilePath, SphereData* sphereData) {

	sphereData->material_ = new Material();
	sphereData->material_->Initialize(dxCommon_, textureFilePath, "rostock_laage_airport_4k.dds");
	sphereData->material_->SetEnableLighting(false);
	sphereData->material_->SetEnvCoefficient(0.0f);
	sphereData->material_->SetMaterialColor({ 1.0f,1.0f,1.0f,1.0f });
}

PrimitiveDrawer::PlaneData* PrimitiveDrawer::GetPlaneData(uint32_t handle) {
	return planeDatas_[handle].get();
}

PrimitiveDrawer::SphereData* PrimitiveDrawer::GetSphereData(uint32_t handle) {
	return sphereDatas_[handle].get();
}

PrimitiveDrawer::RingData* PrimitiveDrawer::GetRingData(uint32_t handle) {
	return ringDatas_[handle].get();
}