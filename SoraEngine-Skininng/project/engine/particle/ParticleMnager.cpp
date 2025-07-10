#include "ParticleMnager.h"
#include <ModelManager.h>
#include <TextureManager.h>
#include "CameraManager.h"
#include <MyMath.h>
#include <numbers>
#include <imgui.h>


//シングルトンインスタンスの取得

ParticleMnager* ParticleMnager::instance_ = nullptr;
ParticleMnager* ParticleMnager::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new ParticleMnager();
	}
	return instance_;


}


void ParticleMnager::Initialize(DirectXCommon* dxcommn, SrvManager* srvmaneger)
{
	//引数で受け取ったポインタをメンバ変数に代入
	dxCommon_ = dxcommn;
	srvManager_ = srvmaneger;
	//乱数エンジンの初期化
	std::random_device seedGenerator;
	std::mt19937 random(seedGenerator());
	randomEngine = random;
	//パイプラインの生成
	graphicsPipeline_ = std::make_unique<GraphicsPipeline>();
	graphicsPipeline_->Initialize(dxCommon_);
	graphicsPipeline_->CreateParticle();


	//カメラとモデルのTrandform変数
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	//worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	//ビルボード行列の作成
	backToFrontMatrix = MyMath::MakeRotateYMatrix(std::numbers::pi_v<float>);

	//マテリアル
	//modelマテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	materialResource = dxCommon_->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む
	materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//色
	materialData->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	materialData->enableLighting = false;//有効にするか否か
	materialData->uvTransform = materialData->uvTransform.MakeIdentity4x4();


	//// 修正: VertexData 構造体の初期化リストを正しく記述  
	//std::vector<VertexData> quadVertices = {
	//   {{-0.5f, -0.5f, 0.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},  // 左下  
	//   {{-0.5f,  0.5f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // 左上  
	//   {{ 0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},  // 右下  

	//   {{ 0.5f, -0.5f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},  // 右下  
	//   {{-0.5f,  0.5f, 0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // 左上  
	//   {{ 0.5f,  0.5f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // 右上  
	//};

	//std::vector<VertexData> quadVertices = MakeRingVertices(32, 1.0f,0.2f);
	std::vector<VertexData>quadVertices = MakeCylinderVertices();
	vertexCount = static_cast<uint32_t>(quadVertices.size());

	// GPUリソース作成
	vertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * quadVertices.size());

	// リソースアドレスを設定
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * quadVertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// GPUにデータ転送
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, quadVertices.data(), sizeof(VertexData) * quadVertices.size());
	vertexResource->Unmap(0, nullptr);


}




void ParticleMnager::Finalize()
{

	delete instance_;
	instance_ = nullptr;


}


void ParticleMnager::Update()
{
	//カメラからビュープロジェクション行列を取得
	//ビルボード行列の計算
	Matrix4x4 billboardMatrix = backToFrontMatrix * CameraManager::GetInstance()->GetActiveCamera()->GetWorldMatrix();
	billboardMatrix.m[3][0] = 0.0f;
	billboardMatrix.m[3][1] = 0.0f;
	billboardMatrix.m[3][2] = 0.0f;
	//ビルボード行列を使ってビルボード行列を計算
	Matrix4x4 viewMatrix = CameraManager::GetInstance()->GetActiveCamera()->GetViewMatrix();
	Matrix4x4 projectionMatrix = CameraManager::GetInstance()->GetActiveCamera()->GetProjextionMatrix();

	

	materialData->uvTransform.m[3][0] += 0.0001f; // X方向スクロール
	materialData->uvTransform.m[3][0] = std::fmod(materialData->uvTransform.m[3][0], 1.0f);
	if (materialData->uvTransform.m[3][0] < 0.0f) materialData->uvTransform.m[3][0] += 1.0f;



	//全パーティクル	グループ内の全パーティクルについて二重処理する
	for (auto& [name, particleGroup] : particleGroups) {
		uint32_t counter = 0;
		for (std::list<Particle>::iterator particleIterator = particleGroup.particles.begin(); particleIterator != particleGroup.particles.end();) {


			//パーティクルの寿命が尽きたらグループから外す
			//寿命に達していたらグループから外す
			if ((*particleIterator).lifetime <= (*particleIterator).currentTime) {
				particleIterator = particleGroup.particles.erase(particleIterator);
				continue;
			}



			//パーティクルの位置を更新
			(*particleIterator).transform.translate += (*particleIterator).Velocity * 1.0f / 60.0f;
			//パーティクルの寿命を減らす
			(*particleIterator).currentTime += 1.0f / 60.0f;
			float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifetime);
			/*float alpha = 1.0f;*/
			//ローテート
			Matrix4x4 rotateMatrix = MyMath::MakeRotateMatrix((*particleIterator).transform.rotate);

			//ワールド行列を計算
			Matrix4x4 worldMatrix = MyMath::MakeScaleMatrix((*particleIterator).transform.scale) * rotateMatrix * MyMath::MakeTranslateMatrix((*particleIterator).transform.translate);
			//waorldViewProjection行列を計算
			Matrix4x4 worldViewProjetionMatrix = worldMatrix * viewMatrix * projectionMatrix;


			if (counter < particleGroup.instanceCount) {
				particleGroup.instanceData[counter].WVP = worldViewProjetionMatrix;
				particleGroup.instanceData[counter].World = worldMatrix;
				particleGroup.instanceData[counter].color = particleIterator->color;
				particleGroup.instanceData[counter].color.w = alpha;
				++counter;
			}




			//次のパーティクルに進む
			++particleIterator;

		}

		// ここでインスタンス数を更新
		particleGroup.instanceCount = counter;

	}




}

void ParticleMnager::Draw()
{


	// パーティクルグループが設定されていない場合は描画しない
	if (particleGroups.empty()) {
		return;
	}

	//ルートシグネチャを設定
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignatureParticle());
	//psoを設定
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineStateParticle());
	//purimitetopologyを設定
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// パーティクルグループごとに描画
	for (const auto& [name, particleGroup] : particleGroups) {


		// インスタンス数が 0 の場合は描画しない
		if (particleGroup.instanceCount == 0) {
			continue;
		}


		dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
		//マテリアルのCBufferの場所を設定
		dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		// インスタンシングデータの SRV を設定
		srvManager_->SetGraficsRootDescriptorTable(2, particleGroup.materialdata.textureIndex);
		// テクスチャの SRV を設定
		srvManager_->SetGraficsRootDescriptorTable(1, particleGroup.srvIndex);
		//描画！
		dxCommon_->GetCommandList()->DrawInstanced(UINT(vertexCount), particleGroup.instanceCount, 0, 0);

	}

}

void ParticleMnager::CreateParticleGroup(const std::string name, const std::string textureFilePath, std::string modelFilePath)
{
	ModelManager::GetInstans()->LoadModel(modelFilePath);
	//モデルのセット
	SetModel(modelFilePath);

	//VertexBufferViewを設定
	//vertexBufferView = model_->GetVertexBufferView();



	//登録済みなら早期リターン
	if (particleGroups.contains(name)) {
		return;
	}

	//パーティクルグループを作成コンテナに登録
	ParticleGroup particleGroup;
	particleGroups.insert(std::make_pair(name, std::move(particleGroup)));//名前をキーにして登録
	//テクスチャファイルパスを登録
	particleGroups.at(name).materialdata.textureFilePath = textureFilePath;
	//テクスチャファイルを読み込んでSRVを取得
	TextureManager::GetInstance()->LoadTexture(textureFilePath);//テクスチャファイルの読み込み
	//SRVのインデックスを取得
	particleGroups.at(name).materialdata.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);	//テクスチャ番号の取得
	//最大インスタンスカウント
	uint32_t MaxInstanceCount = 1000;
	//インスタンス数を初期化
	particleGroups.at(name).instanceCount = 0;
	//インスタンス用のリソースを作成
	particleGroups.at(name).instanceResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * MaxInstanceCount);
	//インスタンス用のリソースをマップ
	particleGroups.at(name).instanceResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroups.at(name).instanceData));
	//インスタンスのデータを初期化
	ParticleForGPU particleForGPU;
	particleForGPU.WVP = particleForGPU.WVP.MakeIdentity4x4();
	particleForGPU.World = particleForGPU.World.MakeIdentity4x4();
	particleForGPU.color = { 1.0f,1.0f,1.0f,0.0f };
	//インスタンスのデータを登録
	for (uint32_t index = 0; index < MaxInstanceCount; ++index) {
		particleGroups.at(name).instanceData[index] = particleForGPU;
	}

	//insutansing用のsrvインデックス
	particleGroups.at(name).srvIndex = srvManager_->Allocate();
	//srv生成
	srvManager_->CreateSRVforStructuredBuffer(particleGroups.at(name).srvIndex, particleGroups.at(name).instanceResource.Get(), MaxInstanceCount, sizeof(ParticleForGPU));






}

void ParticleMnager::Emit(const std::string& name, const Vector3 position, uint32_t count)
{


	//パーティクルグループが存在するかチェックしてassert
	assert(particleGroups.contains(name));
	//パーティクルグループのパーティクルリストにパーティクルを追加
	for (uint32_t i = 0; i < count; ++i) {


		//パーティクルを追加
		particleGroups.at(name).particles.push_back(MakeNormalParticle(randomEngine, position));

	}

	//パーティクルグループのインスタンス数を更新
	particleGroups.at(name).instanceCount = count;
	////インスタンス用のリソースを作成
	//particleGroups.at(name).instanceResource = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * particleGroups.at(name).instanceCount);
	////インスタンス用のリソースをマップ
	//particleGroups.at(name).instanceResource->Map(0, nullptr, reinterpret_cast<void**>(&particleGroups.at(name).instanceData));
	//


}

void ParticleMnager::SetModel(const std::string& filepath)
{
	//もでるを検索してセットする
	model_ = ModelManager::GetInstans()->FindModel(filepath);
}

Particle ParticleMnager::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate)
{


	std::uniform_real_distribution<float>distribution(-1.0, 1.0f);
	std::uniform_real_distribution<float>distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float>distTime(1.0f, 3.0f);

	Particle particle;
	Vector3 randomTranslate{ distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };

	particle.transform.scale = { 1.0f,1.0f,1.0f };
	//particle.transform.rotate = { 0.0f,3.0f,0.0f };
	particle.transform.translate = translate + randomTranslate;
	particle.Velocity = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
	particle.color = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };
	particle.lifetime = distTime(randomEngine);
	particle.currentTime = 0;
	return particle;
}

Particle ParticleMnager::MakeAttackPaarticle(std::mt19937& randomEngine, const Vector3& translate)
{
	std::uniform_real_distribution<float>distribution(-1.0, 1.0f);
	std::uniform_real_distribution<float>distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float>distTime(1.0f, 3.0f);
	std::uniform_real_distribution<float>disRotate(-std::numbers::pi_v<float>, std::numbers::pi_v<float>);
	std::uniform_real_distribution<float>disScale(0.4f, 1.5f);

	Particle particle;
	Vector3 randomTranslate{ distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };

	particle.transform.scale = {0.5f,disScale(randomEngine),1.0f };
	//particle.transform.scale = { 1.0f,1.0f,1.0f };
	//particle.transform.rotate = { 0.0f,0.0f,0.0f };
	particle.transform.rotate = { disRotate(randomEngine),disRotate(randomEngine),disRotate(randomEngine) };
	//particle.transform.translate = translate + randomTranslate;
	particle.transform.translate = translate;
	//particle.Velocity = { distribution(randomEngine),distribution(randomEngine) ,distribution(randomEngine) };
	particle.Velocity = { 0.0f,0.0f,0.0f };
	particle.color = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };
	//particle.color = { 1.0f,1.0f,1.0f,1.0f };
	//particle.lifetime = distTime(randomEngine);
	particle.lifetime = 1.0f;
	particle.currentTime = 0;
	return particle;
}

Particle ParticleMnager::MakeNormalParticle(std::mt19937& randomEngine, const Vector3& translate)
{
	Particle particle;

	
	particle.transform.scale = { 1.0f,1.0f,1.0f };
	particle.transform.rotate = { 0.0f,0.0f,0.0f };
	particle.transform.translate = translate;
	particle.Velocity = { 0.0f,0.0f,0.0f };
	particle.color = { 1.0f,0.0f,1.0f,1.0f };
	
	particle.lifetime = 1.0f;
	particle.currentTime = 0;
	return particle;
}

std::vector<VertexData> ParticleMnager::MakeRingVertices(uint32_t  RingDivide, float outerRadius, float innerRadius)
{
	
	std::vector<VertexData> ringVertices;
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / static_cast<float>(RingDivide);
	
	for (uint32_t index = 0; index < RingDivide; ++index) {
		// 現在と次の角度
		float angle = index * radianPerDivide;
		float nextAngle = ((index + 1) % RingDivide) * radianPerDivide;

		// sin, cos
		float sin = std::sinf(angle);
		float cos = std::cosf(angle);
		float sinnext = std::sinf(nextAngle);
		float cosnext = std::cosf(nextAngle);

		// UV (ここもwrapを考慮)
		float u = (static_cast<float>(index) / RingDivide)  ;
		float unext = (static_cast<float>(index + 1) / RingDivide) ;

		VertexData v[] = {
			{ {-sin * outerRadius,  cos * outerRadius,  0.0f, 1.0f},     {u,     0.0f}, {0.0f, 0.0f, 1.0f} },
			{ {-sin * innerRadius,  cos * innerRadius,  0.0f, 1.0f},     {u,     1.0f}, {0.0f, 0.0f, 1.0f} },
			{ {-sinnext * outerRadius, cosnext * outerRadius, 0.0f, 1.0f}, {unext, 0.0f}, {0.0f, 0.0f, 1.0f} },

			{ {-sinnext * outerRadius, cosnext * outerRadius, 0.0f, 1.0f}, {unext, 0.0f}, {0.0f, 0.0f, 1.0f} },
			{ {-sin * innerRadius,  cos * innerRadius,  0.0f, 1.0f},     {u,     1.0f}, {0.0f, 0.0f, 1.0f} },
			{ {-sinnext * innerRadius, cosnext * innerRadius, 0.0f, 1.0f}, {unext, 1.0f}, {0.0f, 0.0f, 1.0f} }
		};

		for (const auto& vert : v) {
			ringVertices.push_back(vert);
		}
	}

	return ringVertices;
			
}

std::vector<VertexData> ParticleMnager::MakeCylinderVertices(uint32_t cylinderDivide, float topRadius, float bottomRadius, float height)
{
	const float radianPerDivide = 2.0f * std::numbers::pi_v<float> / static_cast<float>(cylinderDivide);

	std::vector<VertexData> cylinderVertices;

	for (uint32_t index = 0; index < cylinderDivide; ++index) {

		float sin = std::sinf(index * radianPerDivide);
		float cos = std::cosf(index * radianPerDivide);
		float sinnext = std::sinf((index + 1) * radianPerDivide);
		float cosnext = std::cosf((index + 1) * radianPerDivide);
		float u = float(index) / float(cylinderDivide);
		float unext = float(index + 1) / float(cylinderDivide);

		VertexData v[] = {
			{{-sin * topRadius,height,cos * topRadius,1.0f},				{u,0.0f} ,		{-sin,0.0f,cos}},
			{{-sinnext * topRadius,height,cosnext * topRadius,1.0f},		{unext,0.0f},	{-sinnext,0.0f,cosnext}},
			{{-sin * bottomRadius,0.0f,cos * bottomRadius,1.0f},			{u,1.0f} ,		{-sin,0.0f,cos}},
			{{-sinnext * topRadius,height,cosnext * topRadius,1.0f},		{unext,0.0f},	{-sinnext,0.0f,cosnext}},
			{{-sinnext * bottomRadius,0.0f,cosnext * bottomRadius,1.0f},	{unext,1.0f},	{-sinnext,0.0f,cosnext}},
			{{-sin * bottomRadius,0.0f,cos * bottomRadius,1.0f},{u,1.0f} ,	{-sin,0.0f,cos}}

		};
		for (const auto& vert : v) {
			cylinderVertices.push_back(vert);
		}

	}
	return cylinderVertices;
}

