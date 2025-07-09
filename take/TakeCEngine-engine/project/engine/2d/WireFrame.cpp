#include "WireFrame.h"
#include "math/MatrixMath.h"
#include "camera/CameraManager.h"
#include "math/Easing.h"

void WireFrame::Initialize(DirectXCommon* directXCommon) {

	dxCommon_ = directXCommon;

	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"WireFrame/WireFrame.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"WireFrame/WireFrame.PS.hlsl");

	//線描画用のPSOの生成
	pso_->CreateGraphicPSO(
		directXCommon->GetDevice(),
		D3D12_FILL_MODE_WIREFRAME,
		D3D12_DEPTH_WRITE_MASK_ZERO,
		PSO::BlendState::NORMAL,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE
	);
	pso_->SetGraphicPipelineName("WireFramePSO");

	rootSignature_ = pso_->GetGraphicRootSignature();

	lineData_ = new LineData();
	CreateVertexData();

	CalculateSphereVertexData();

	//TransformationMatrix用のResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(WireFrameTransFormMatrixData));
	wvpResource_->SetName(L"WireFrame::wvpResource_");
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&TransformMatrixData_));
}

void WireFrame::Update() {

	TransformMatrixData_->WVP = CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
}

//=============================================================================
// 線の描画
//=============================================================================

void WireFrame::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color) {

	lineData_->vertexData_[lineIndex_].position = start;
	lineData_->vertexData_[lineIndex_ + 1].position = end;

	lineData_->vertexData_[lineIndex_].color = color;
	lineData_->vertexData_[lineIndex_ + 1].color = color;

	lineIndex_ += kLineVertexCount_;
}

//=============================================================================
// OBBの描画
//=============================================================================

void WireFrame::DrawOBB(const OBB& obb, const Vector4& color) {

	// OBBの各頂点を定義（ローカル座標）
	Vector3 localVertices[8] = {
		{-obb.halfSize.x, -obb.halfSize.y, -obb.halfSize.z}, {obb.halfSize.x, -obb.halfSize.y, -obb.halfSize.z},
		{obb.halfSize.x,  obb.halfSize.y, -obb.halfSize.z}, {-obb.halfSize.x,  obb.halfSize.y, -obb.halfSize.z},
		{-obb.halfSize.x, -obb.halfSize.y,  obb.halfSize.z}, {obb.halfSize.x, -obb.halfSize.y,  obb.halfSize.z},
		{obb.halfSize.x,  obb.halfSize.y,  obb.halfSize.z}, {-obb.halfSize.x,  obb.halfSize.y,  obb.halfSize.z}
	};

	// ワールド座標に変換（回転適用 & 平行移動）
	Vector3 worldVertices[8];
	for (int i = 0; i < 8; i++) {
		worldVertices[i] =
			obb.center +
			obb.axis[0] * localVertices[i].x +
			obb.axis[1] * localVertices[i].y +
			obb.axis[2] * localVertices[i].z;
	}

	// OBBのエッジを結ぶ
	int edges[12][2] = {
		{0, 1}, {1, 2}, {2, 3}, {3, 0}, // 底面
		{4, 5}, {5, 6}, {6, 7}, {7, 4}, // 上面
		{0, 4}, {1, 5}, {2, 6}, {3, 7}  // 側面
	};

	for (int i = 0; i < 12; i++) {
		DrawLine(worldVertices[edges[i][0]], worldVertices[edges[i][1]], color);
	}
}

//=============================================================================
// 球の描画
//=============================================================================

void WireFrame::DrawSphere(const Vector3& center, float radius, const Vector4& color) {

	Matrix4x4 worldMatrix = MatrixMath::MakeAffineMatrix(Vector3(radius, radius, radius), Vector3(0.0f, 0.0f, 0.0f), center);

	for (uint32_t i = 0; i + 2 < spheres_.size(); i += 3) {
		Vector3 a = spheres_[i];
		Vector3 b = spheres_[i + 1];
		Vector3 c = spheres_[i + 2];

		a = MatrixMath::Transform(a, worldMatrix);
		b = MatrixMath::Transform(b, worldMatrix);
		c = MatrixMath::Transform(c, worldMatrix);

		// 線描画
		DrawLine(a, b, color);
		//DrawLine(b, c, color);
		DrawLine(a, c, color); // 三角形を完成させるための線を追加
	}
}

//=============================================================================
// グリッド地面の描画
//=============================================================================

void WireFrame::DrawGridGround(const Vector3& center, const Vector3& size, uint32_t division) {

	Vector3 halfSize = size * 0.5f;
	Vector3 start = center + Vector3(-halfSize.x, 0.0f, -halfSize.z);
	Vector3 end = center + Vector3(halfSize.x, 0.0f, -halfSize.z);
	Vector4 color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	// 横線
	for (uint32_t i = 0; i <= division; i++) {
		float t = float(i) / float(division);
		Vector3 s = Easing::Lerp(start, end, t);
		Vector3 e = s + Vector3(0.0f, 0.0f, size.z);
		DrawLine(s, e, color);
	}
	start = center + Vector3(-halfSize.x, 0.0f, -halfSize.z);
	end = center + Vector3(-halfSize.x, 0.0f, halfSize.z);
	// 縦線
	for (uint32_t i = 0; i <= division; i++) {
		float t = float(i) / float(division);
		Vector3 s = Easing::Lerp(start, end, t);
		Vector3 e = s + Vector3(size.x, 0.0f, 0.0f);
		DrawLine(s, e, color);
	}

	//各座標軸の中心線のみ色変更
	color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	DrawLine(center, center + Vector3(size.x, 0.0f, 0.0f), color);

	color = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
	DrawLine(center, center + Vector3(0.0f, 0.0f, size.z), color);

	color = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
	DrawLine(center, center + Vector3(0.0f, size.y, 0.0f), color);
}

void WireFrame::DrawGridBox(const AABB& aabb, uint32_t division) {

	//グリッドを6面描画
	Vector3 size = aabb.max - aabb.min;
	Vector3 halfSize = size * 0.5f;
	Vector4 color = Vector4(1.0f, 1.0f, 0.5f, 1.0f);

	// 前面
	DrawGridLines(Vector3(-halfSize.x, -halfSize.y, -halfSize.z),
		Vector3(halfSize.x, -halfSize.y, -halfSize.z),
		Vector3(0.0f, halfSize.y * 2, 0.0f), division, color);
	DrawGridLines(Vector3(-halfSize.x, -halfSize.y, -halfSize.z),
		Vector3(-halfSize.x, halfSize.y, -halfSize.z),
		Vector3(halfSize.x * 2, 0.0f, 0.0f), division, color);

	// 後面
	DrawGridLines(Vector3(-halfSize.x, -halfSize.y, halfSize.z),
		Vector3(halfSize.x, -halfSize.y, halfSize.z),
		Vector3(0.0f, halfSize.y * 2, 0.0f), division, color);
	DrawGridLines(Vector3(-halfSize.x, -halfSize.y, halfSize.z),
		Vector3(-halfSize.x, halfSize.y, halfSize.z),
		Vector3(halfSize.x * 2, 0.0f, 0.0f), division, color);

	// 左面
	DrawGridLines(Vector3(-halfSize.x, -halfSize.y, -halfSize.z),
		Vector3(-halfSize.x, halfSize.y, -halfSize.z),
		Vector3(0.0f, 0.0f, halfSize.z * 2), division, color);
	DrawGridLines(Vector3(-halfSize.x, -halfSize.y, -halfSize.z),
		Vector3(-halfSize.x, -halfSize.y, halfSize.z),
		Vector3(0.0f, halfSize.y * 2, 0.0f), division, color);

	// 右面
	DrawGridLines(Vector3(halfSize.x, -halfSize.y, -halfSize.z),
		Vector3(halfSize.x, halfSize.y, -halfSize.z),
		Vector3(0.0f, 0.0f, halfSize.z * 2), division, color);
	DrawGridLines(Vector3(halfSize.x, -halfSize.y, -halfSize.z),
		Vector3(halfSize.x, -halfSize.y, halfSize.z),
		Vector3(0.0f, halfSize.y * 2, 0.0f), division, color);

	// 上面
	DrawGridLines(Vector3(-halfSize.x, halfSize.y, -halfSize.z),
		Vector3(halfSize.x, halfSize.y, -halfSize.z),
		Vector3(0.0f, 0.0f, halfSize.z * 2), division, color);
	DrawGridLines(Vector3(-halfSize.x, halfSize.y, -halfSize.z),
		Vector3(-halfSize.x, halfSize.y, halfSize.z),
		Vector3(halfSize.x * 2, 0.0f, 0.0f), division, color);

	// 下面
	DrawGridLines(Vector3(-halfSize.x, -halfSize.y, -halfSize.z),
		Vector3(halfSize.x, -halfSize.y, -halfSize.z),
		Vector3(0.0f, 0.0f, halfSize.z * 2), division, color);
	DrawGridLines(Vector3(-halfSize.x, -halfSize.y, -halfSize.z),
		Vector3(-halfSize.x, -halfSize.y, halfSize.z),
		Vector3(halfSize.x * 2, 0.0f, 0.0f), division, color);
	

}

void WireFrame::Draw() {

	//ルートシグネチャ設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(rootSignature_.Get());
	//PSO設定
	dxCommon_->GetCommandList()->SetPipelineState(pso_->GetGraphicPipelineState());
	//プリミティブトポロジー設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

	//頂点バッファの設定
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &lineData_->vertexBufferViews_);

	//transformMatrixResource
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, wvpResource_->GetGPUVirtualAddress());

	//描画
	dxCommon_->GetCommandList()->DrawInstanced(lineIndex_, lineIndex_ / kLineVertexCount_, 0, 0);

	//リセット
	lineIndex_ = 0;
}

void WireFrame::Reset() {
	lineIndex_ = 0;
}

void WireFrame::Finalize() {

	pso_.reset();
	rootSignature_.Reset();
	wvpResource_.Reset();
	lineData_->vertexBuffer_.Reset();
}

void WireFrame::CreateVertexData() {

	UINT vertexBufferSize = sizeof(WireFrameVertexData) * kLineVertexCount_ * kMaxLineCount_;

	//頂点バッファの生成
	lineData_->vertexBuffer_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), vertexBufferSize);
	lineData_->vertexBuffer_->SetName(L"WireFrame::VertexBuffer");

	//頂点バッファのビュー設定
	lineData_->vertexBufferViews_.BufferLocation = lineData_->vertexBuffer_->GetGPUVirtualAddress();
	lineData_->vertexBufferViews_.StrideInBytes = sizeof(WireFrameVertexData);
	lineData_->vertexBufferViews_.SizeInBytes = vertexBufferSize;

	//頂点リソースのマップ
	lineData_->vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&lineData_->vertexData_));
}

void WireFrame::CalculateSphereVertexData() {

	const float pi = 3.1415926535897932f;
	const uint32_t kSubdivision = 8; // 分割数
	const float kLonEvery = 2.0f * pi / float(kSubdivision); // 経度の1分割の角度
	const float kLatEvery = pi / float(kSubdivision); // 緯度の1分割の角度

	// 緯度方向
	for (uint32_t latIndex = 0; latIndex < kSubdivision; latIndex++) {
		float lat = -pi / 2.0f + kLatEvery * float(latIndex);
		// 経度方向
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; lonIndex++) {
			float lon = kLonEvery * float(lonIndex);
			// 球の表面上の点を求める
			Vector3 a, b, c;
			a.x = cos(lat) * cos(lon);
			a.y = sin(lat);
			a.z = cos(lat) * sin(lon);

			b.x = cos(lat + kLatEvery) * cos(lon);
			b.y = sin(lat + kLatEvery);
			b.z = cos(lat + kLatEvery) * sin(lon);

			c.x = cos(lat) * cos(lon + kLonEvery);
			c.y = sin(lat);
			c.z = cos(lat) * sin(lon + kLonEvery);

			// 座標を保存
			spheres_.push_back(a);
			spheres_.push_back(b);
			spheres_.push_back(c);
		}
	}
}

void WireFrame::DrawGridLines(const Vector3& start, const Vector3& end, const Vector3& offset, uint32_t division, const Vector4& color) {
	for (uint32_t i = 0; i <= division; i++) {
		float t = float(i) / float(division);
		Vector3 s = Easing::Lerp(start, end, t);
		Vector3 e = s + offset;
		DrawLine(s, e, color);
	}
}
