#include "Model.h"
#include <fstream>
#include <sstream>
#include <assert.h>
#include "TextureManager.h"
#include "SrvManager.h"



void Model::Initialize(ModelCommon* modeleCommon, const std::string& directorypath, const std::string& filename)
{
	/*textureFilePath_ = filename;*/

	modelCommon_ = modeleCommon;

	modelData = LoadModelFile(directorypath, filename);
	animation = LoadAnimationFile(directorypath, filename);
	skeleton = CreateSkeleton(modelData.rootNode);
	skinCluster = CreateSkinCluster();

	//モデルオブジェクト
	//モデル用のVetexResouceを作成
	vertexResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	//リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズは頂点分のサイズ
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());
	//1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());


	//インデックス設定
	//インデックスバッファ用のリソースを作成
	indexResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * modelData.indices.size());
	//リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス分のサイズ
	indexBufferView.SizeInBytes = UINT(sizeof(uint32_t) * modelData.indices.size());
	//format
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	//書き込むためのアドレスを取得
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndex));
	std::memcpy(mappedIndex, modelData.indices.data(), sizeof(uint32_t) * modelData.indices.size());


	//マテリアル
	//modelマテリアる用のリソースを作る。今回color1つ分のサイズを用意する
	materialResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(Material));
	//マテリアルにデータを書き込む	
	materialData = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	//色
	materialData->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };

	materialData->enableLighting = true;//有効にするか否か
	materialData->uvTransform = materialData->uvTransform.MakeIdentity4x4();
	materialData->shiniess = 60.0f;


	//.objの参照しているテクスチャファイル読み込み
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
	//読み込んだテクスチャ番号を取得
	modelData.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);

}

void Model::Draw()
{

	D3D12_VERTEX_BUFFER_VIEW vbvs[2] = {

		vertexBufferView,
		skinCluster.influenceBufferView

	};

	//VertexBufferViewを設定
	modelCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 2, vbvs);
	//インデックスバッファビューを設定
	modelCommon_->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView);
	//マテリアルのCBufferの場所を設定
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定
	modelCommon_->GetSRVManager()->SetGraficsRootDescriptorTable(2, TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath));
	//描画！
	//modelCommon_->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);

	 // インデックス描画（インスタンス数 = 1）
	modelCommon_->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(
		static_cast<UINT>(modelData.indices.size()), // Index数
		1,  // インスタンス数
		0,  // StartIndexLocation
		0,  // BaseVertexLocation
		0   // StartInstanceLocation
	);

}

Node Model::ReadNode(aiNode* node)
{
	Node result;
	aiVector3D scale, translate;
	aiQuaternion rotation;

	node->mTransformation.Decompose(scale, rotation, translate);//スケール、回転、平行移動を取得
	result.transform.scale = { scale.x,scale.y,scale.z };//スケールを取得
	result.transform.rotate = { rotation.x,-rotation.y,-rotation.z,rotation.w };//回転を取得
	result.transform.translate = { translate.x,translate.y,translate.z };//平行移動を取得
	result.localMatrix = MyMath::MakeAffineMatrix(result.transform.scale, result.transform.rotate, result.transform.translate);//ローカル行列を取得
	result.name = node->mName.C_Str();//名前を取得
	result.children.resize(node->mNumChildren);//子ノードの数だけリサイズ
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);//子ノードを読み込む
	}
	return result;
}

Skeleton Model::CreateSkeleton(const Node& rootNode)
{
	Skeleton skeleton;

	// ルートノードからジョイントツリーを構築
	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

	// 名前と index のマッピングを行いアクセスしやすくする
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointMap.emplace(joint.name, joint.index);
	}

	return skeleton;
}

int32_t Model::CreateJoint(const Node& node, std::optional<int32_t> parent, std::vector<Joint>& joints)
{
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = joint.skeletonSpaceMatrix.MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = int32_t(joints.size()); // 現在登録されてる数をIndexに
	joint.parent = parent;

	joints.push_back(joint); // SkeletonのJoint列に追加

	// 子Jointを作成し、そのIndexを登録
	for (const Node& child : node.children) {
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}

	// 自身のIndexを返す
	return joint.index;
}


MaterialData Model::LoadMaterialTemplateFile(const std::string& directorypath, const std::string& filename)
{

	MaterialData materialData;//構築するMaterialData
	std::string line;//ファイルから読んだ1行を格納するもの
	std::ifstream file(directorypath + "/" + filename);//ファイルを開く
	assert(file.is_open());//とりあえず開けなっかたら止める
	while (std::getline(file, line)) {
		std::string identifile;
		std::stringstream s(line);
		s >> identifile;

		//identifierの応じた処理
		if (identifile == "map_Kd") {

			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directorypath + "/" + textureFilename;

		}


	}

	return materialData;

}

ModelData Model::LoadModelFile(const std::string& ditrectoryPath, const std::string& filename)
{
	ModelData modelData;//構築するModekData
	Assimp::Importer importer;
	std::string path = ditrectoryPath + "/" + "models" + "/" + filename;
	const aiScene* scene = importer.ReadFile(path.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
	assert(scene->HasMeshes());//メッシュが何の歯対応しない

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());//法線情報がない
		assert(mesh->HasTextureCoords(0));//テクスチャ座標がない
		modelData.vertices.resize(mesh->mNumVertices);//頂点数分のメモリを確保

		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];

			// 右手系 -> 左手系への変換を忘れずに
			modelData.vertices[vertexIndex].position = { -position.x, position.y, position.z, 1.0f };
			modelData.vertices[vertexIndex].normal = { -normal.x, normal.y, normal.z };
			modelData.vertices[vertexIndex].texcoord = { texcoord.x, texcoord.y };
		}

		//インデックス情報を解析
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			aiFace& face = mesh->mFaces[faceIndex];
			//
			assert(face.mNumIndices == 3);//三角形以外は対応しない
			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				modelData.indices.push_back(vertexIndex);//インデックスを格納
			}
		}

		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {

			//meshに関連つけられたジョイントから情報を取得
			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName = bone->mName.C_Str();
			JointWeightData& jointWeightData = modelData.skinClusterData[jointName];

			//Bindpose時の各成分を抽出変換
			aiMatrix4x4 bindPoseMatrixAssimp = bone->mOffsetMatrix.Inverse();
			aiVector3D scale, translate;
			aiQuaternion rotation;
			bindPoseMatrixAssimp.Decompose(scale, rotation, translate);//スケール、回転、平行移動を取得
			Matrix4x4 bindposeMatrix = MyMath::MakeAffineMatrix(
				{ scale.x,scale.y,scale.z }, { rotation.x,-rotation.y,-rotation.z,rotation.w }, { -translate.x,translate.y,translate.z });//ローカル行列を取得
			jointWeightData.inverseBindPoseMatrix = bindposeMatrix.Inverse();//逆バインドポーズ行列を格納

			//ジョイントに関連つけられた頂点の重みとその頂点のインデックスを取り出して格納する
			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight,bone->mWeights[weightIndex].mVertexId });//頂点の重みを格納
			}
		}


	}

	//マテリアルの解析
	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		aiMaterial* material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString texturePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
			modelData.material.textureFilePath = ditrectoryPath + "/" + texturePath.C_Str();
		}
	}

	modelData.rootNode = ReadNode(scene->mRootNode);

	return modelData;
}

Animation Model::LoadAnimationFile(const std::string& directoryPath, const std::string& filename)
{
	Animation animation;
	Assimp::Importer importer;
	std::string filepath = directoryPath + "/" + "models" + "/" + filename;
	const aiScene* scene = importer.ReadFile(filepath.c_str(), 0);
	// アニメーションがない場合、空のAnimationを返す
	if (scene->mNumAnimations == 0) {
		return animation;
	}
	aiAnimation* animationAssimp = scene->mAnimations[0];//最初のアニメーションだけ採用
	animation.duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);//時間単位を秒に変換

	for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {

		aiNodeAnim* nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
		NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];

		//アニメーションのキーフレームを取得
		//位置
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mPositionKeys[keyIndex];
			KeyframeVector3 Keyframe;
			Keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);//時間単位を秒に変換
			Keyframe.value = { -keyAssimp.mValue.x,keyAssimp.mValue.y,keyAssimp.mValue.z };//右手-＞左手
			nodeAnimation.translate.push_back(Keyframe);
		}
		//回転
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
			aiQuatKey& keyAssimp = nodeAnimationAssimp->mRotationKeys[keyIndex];
			KeyframeQuaternion Keyframe;
			Keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);//時間単位を秒に変換
			Keyframe.value = { keyAssimp.mValue.x,-keyAssimp.mValue.y,-keyAssimp.mValue.z,keyAssimp.mValue.w };
			nodeAnimation.rotate.push_back(Keyframe);
		}
		//スケール
		for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
			aiVectorKey& keyAssimp = nodeAnimationAssimp->mScalingKeys[keyIndex];
			KeyframeVector3 Keyframe;
			Keyframe.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);//時間単位を秒に変換
			Keyframe.value = { keyAssimp.mValue.x,keyAssimp.mValue.y,keyAssimp.mValue.z };
			nodeAnimation.scale.push_back(Keyframe);
		}

	}
	return animation;

}


SkinCluster Model::CreateSkinCluster()
{

	//palette用のリソースを作成
	SkinCluster skinCluster;
	skinCluster.paletteResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(WellForGPU) * skeleton.joints.size());
	WellForGPU* mappedPalette = nullptr;
	skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
	skinCluster.mappedPalette = { mappedPalette,skeleton.joints.size() };
	//srvをアロケート
	uint32_t srvIndex = modelCommon_->GetSRVManager()->Allocate();
	modelCommon_->GetSRVManager()->CreateSRVforStructuredBuffer(srvIndex, skinCluster.paletteResource.Get(), (UINT)skeleton.joints.size(), sizeof(WellForGPU));
	skinCluster.paletteSrvHandle.first = modelCommon_->GetSRVManager()->GetCPUDescriptorHandle(srvIndex);
	skinCluster.paletteSrvHandle.second = modelCommon_->GetSRVManager()->GetGPUDescriptorHandle(srvIndex);

	//Influence用のリソースを作成
	skinCluster.influenceResource = modelCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexInfluence) * modelData.vertices.size());
	VertexInfluence* mappedInfluence = nullptr;
	skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
	std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData.vertices.size());
	skinCluster.mappedInfluence = { mappedInfluence,modelData.vertices.size() };
	//Influence用のVBVを作成
	skinCluster.influenceBufferView.BufferLocation = skinCluster.influenceResource->GetGPUVirtualAddress();
	skinCluster.influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * modelData.vertices.size());
	skinCluster.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);
	//InverseBindPoseMatrixを格納する場所を作成して、単位行列で埋める
	skinCluster.inverseBindPoseMatrices.resize(skeleton.joints.size());
	std::generate(skinCluster.inverseBindPoseMatrices.begin(), skinCluster.inverseBindPoseMatrices.end(), [] { return MyMath::MakeIdentity4x4(); });

	for (const auto& jointWeight : modelData.skinClusterData) {
		auto it = skeleton.jointMap.find(jointWeight.first);//jointweight名なので、skeletoに対象となるjointが含まれているか判断
		if (it == skeleton.jointMap.end()) {
			continue;//なければスキップ
		}
		// (*it).secondにはjointのindexが入っているので、該当のindexのinverseBindPoseMatrixを代入
		skinCluster.inverseBindPoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;

		for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
			auto& currentInfluence = skinCluster.mappedInfluence[vertexWeight.vectorIndex]; // 該当のvertexIndexのinfluence情報を参照しておく

			for (uint32_t index = 0; index < kNumMaxInfluence; ++index) { // 空いているところに入れる
				if (currentInfluence.weights[index] == 0.0f) { // weight==0が空いている状態なので、その場所にweightとjointのindexを代入
					currentInfluence.weights[index] = vertexWeight.weight;
					currentInfluence.jointIndices[index] = (*it).second;
					break;
				}
			}
		}

	}
	return skinCluster;
}