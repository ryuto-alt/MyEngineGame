#include "Object3DCommon.h"
#include "Object3D.h"
#include "MyMath.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "CameraManager.h"
#include <numbers>




void Object3D::Initialize(Object3DCommon* object3DCommon)
{
	//引数で受け取って、メンバ変数に記録する
	object3DCommon_ = object3DCommon;

	//トランスフォーム
	//ModelTransform用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource = object3DCommon_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	//書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformaitionMatrixData));
	//単位行列を書き込む

	transformaitionMatrixData->WVP = transformaitionMatrixData->WVP.MakeIdentity4x4();
	transformaitionMatrixData->World = transformaitionMatrixData->World.MakeIdentity4x4();
	transformaitionMatrixData->worldInberseTranspose = transformaitionMatrixData->worldInberseTranspose.MakeIdentity4x4();


	//平行光源
	//平行光源用のResoureceを作成
	directionalLightResource = object3DCommon_->GetDxCommon()->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,1.0f };
	directionalLightData->intensity = 1.0f;
	directionalLightData->enable = 0;

	//ポイントライト
	//ポイントライト用のリソースを作成
	pointLightResource = object3DCommon_->GetDxCommon()->CreateBufferResource(sizeof(PointLight));
	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));
	pointLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	pointLightData->position = { 0.0f,0.0f,0.0f };
	pointLightData->intensity = 1.0f;
	pointLightData->radius = 10.0f;
	pointLightData->decay = 1.0f;
	pointLightData->enable = 1;

	//スポットライト
	//スポットライト用のリソースを作成
	spotLightResource = object3DCommon_->GetDxCommon()->CreateBufferResource(sizeof(SpotLight));//
	spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));
	spotLightData->color = { 1.0f,1.0f,1.0f,1.0f };//色
	spotLightData->position = { 0.0f,2.0f,0.0f };//ライトの位置
	spotLightData->intensity = 4.0f;//明るさ
	spotLightData->direction = MyMath::Normlize({ 0.0f,-1.0f,0.0f });//ライトの方向
	spotLightData->distance = 7.0f;//ライトの届く最大距離
	spotLightData->decay = 2.0f;//減衰率
	spotLightData->consAngle = std::cos(std::numbers::pi_v<float> / 3.0f);//スポットライトの余弦
	spotLightData->cosFalloffstrt = 1.0f;
	spotLightData->enable = 1;



	//カメラとモデルのTrandform変数
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };


	//カメラforGPU
	cameraResource = object3DCommon_->GetDxCommon()->CreateBufferResource(sizeof(CaMeraForGpu));
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGpu));
	/*cameraForGpu->worldPosition = { 0.0f,0.0f,0.0f };*/



}

void Object3D::Update()
{


	if (enableAnimation_ && model_ && model_->GetAnimation().nodeAnimations.size() > 0) {
		ApplyAnimation(model_->GetSkeleton(), model_->GetAnimation(), animationTime);
		SkeletonUpdate(model_->GetSkeleton());
		SkinClusterUpdate(model_->GetSkinCluster(), model_->GetSkeleton());

		

		animationTime += 1.0f / 60.0f;//アニメーションの時間を加算
		animationTime = std::fmod(animationTime, model_->GetAnimation().duration);//アニメーションの時間をループさせる
	}





	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Camera* activeCamera = CameraManager::GetInstance()->GetActiveCamera();

	if (model_) {
		//ライトのオンオフ
		model_->SetEnableLighting(enableLighting);
		// **ここでモデルの色を設定**
		model_->SetColor(color_);
	}

	if (activeCamera) {




		const Matrix4x4& viewProjectionMatrix = activeCamera->GetViewprojectionMatrix();
		worldViewProjectionMatrix = worldMatrix * viewProjectionMatrix;
		transformaitionMatrixData->WVP = worldViewProjectionMatrix;
		transformaitionMatrixData->World = worldMatrix;
		Vector3 cameraPosition = activeCamera->GetTransform().translate;
		cameraForGpu->worldPosition = cameraPosition;
		



	} else {
		worldViewProjectionMatrix = worldMatrix;
		transformaitionMatrixData->WVP = worldViewProjectionMatrix;
		transformaitionMatrixData->World = worldMatrix;
	}

	
}

void Object3D::SkeletonUpdate(Skeleton& skeleton)
{
	// ← ここでサイズを合わせるのが絶対必要！！
	skeletonPose_.resize(skeleton.joints.size());
	//すべてのjointを更新。親が若いので通常ループで処理可能になっている
	for (Joint& joint : skeleton.joints)
	{
		joint.localMatrix = MyMath::MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		if (joint.parent)
		{
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;

		} else
		{
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
		skeletonPose_[joint.index] = joint.skeletonSpaceMatrix;
	}
	
}

void Object3D::ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime)
{
	for (Joint& joint : skeleton.joints) {
		// 対象のJointのAnimationがあれば、値の適用を行う。
		// 下記のif文はC++17から可能になった初期化付きif文。
		if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end()) {
			const NodeAnimation& nodeAnimation = it->second;

			joint.transform.translate = CalculateValue(nodeAnimation.translate, animationTime);
			joint.transform.rotate = CalculateValue(nodeAnimation.rotate, animationTime);
			joint.transform.scale = CalculateValue(nodeAnimation.scale, animationTime);
		}
	}
}

void Object3D::SkinClusterUpdate(SkinCluster& skinCluster, const Skeleton& skeleton)
{
	for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex)
	{
		assert(jointIndex < skinCluster.inverseBindPoseMatrices.size());
		skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix =
			skinCluster.inverseBindPoseMatrices[jointIndex] * skeleton.joints[jointIndex].skeletonSpaceMatrix;
		skinCluster.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
			MyMath::Transpose(skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix.Inverse());
	}


}




void Object3D::Draw()
{


	object3DCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	//平行光源Cbufferの場所を設定
	object3DCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
	//カメラのデータをセット
	object3DCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraResource->GetGPUVirtualAddress());
	//ポイントライトのCBufferの場所を設定
	object3DCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(5, pointLightResource->GetGPUVirtualAddress());
	//スポットライトのCBufferの場所を設定
	object3DCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(6, spotLightResource->GetGPUVirtualAddress());
	//skeletonのデータをセット
	object3DCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(7, model_->GetSkinCluster().paletteSrvHandle.second);
	//3Dモデルが割り当てられているなら描画する
	if (model_) {
		model_->Draw();
	}


}


void Object3D::SetModel(const std::string& filepath)
{
	//もでるを検索してセットする
	model_ = ModelManager::GetInstans()->FindModel(filepath);
}



Vector3 Object3D::CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time)
{
	assert(!keyframes.empty());
	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}

	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndenx = index + 1;
		//indexとnextIndexの2つのキーフレームを取得して範囲内に時刻があるか判定する
		if (keyframes[index].time <= time && time <= keyframes[nextIndenx].time) {
			//補間する
			float t = (time - keyframes[index].time) / (keyframes[nextIndenx].time - keyframes[index].time);
			return MyMath::Lerp(keyframes[index].value, keyframes[nextIndenx].value, t);
		}

	}
	return (*keyframes.rbegin()).value;
}

Quaternion Object3D::CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time)
{

	assert(!keyframes.empty());
	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}
	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndenx = index + 1;
		//indexとnextIndexの2つのキーフレームを取得して範囲内に時刻があるか判定する
		if (keyframes[index].time <= time && time <= keyframes[nextIndenx].time) {
			//補間する
			float t = (time - keyframes[index].time) / (keyframes[nextIndenx].time - keyframes[index].time);
			return MyMath::Slerp(keyframes[index].value, keyframes[nextIndenx].value, t);
		}
	}
	return (*keyframes.rbegin()).value;
}







