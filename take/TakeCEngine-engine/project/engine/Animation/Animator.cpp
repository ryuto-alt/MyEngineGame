#include "Animator.h"
#include "Easing.h"
//assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <cassert>
#include "Animator.h"

//=============================================================================
//	アニメーションの読み込み/登録
//=============================================================================

void Animator::Finalize() {
	animations_.clear();
}

void Animator::LoadAnimation(const std::string& filePath) {

	//読み込み済みアニメーションを検索
	if (animations_.contains(filePath)) {
		//すでに読み込み済みならreturn
		return;
	}

	//アニメーションの生成とファイル読み込み、初期化
	std::map<std::string, Animation*> animation = LoadAnimationFile("Animation", filePath);
	//アニメーションをコンテナに追加
	animations_.insert(std::make_pair(filePath, animation));
}

//=============================================================================
//	アニメーションの検索
//=============================================================================

Animation* Animator::FindAnimation(const std::string& filePath, const std::string& animName) {
	//読み込み済みアニメーションを検索
	if (animations_.contains(filePath)) {
		if (animations_.at(filePath).contains(animName)) {

			//読み込みアニメーションを戻り値としてreturn
			return animations_.at(filePath).at(animName);
		}
	}

	//ファイル名一致なし
	assert(0 && "アニメーションが見つかりませんでした");
	return {};
}

//=============================================================================
//	アニメーションファイルの読み込み
//=============================================================================
std::map<std::string, Animation*> Animator::LoadAnimationFile(const std::string& directoryPath, const std::string& filename) {
	std::map<std::string, Animation*> animations = {};
    Assimp::Importer importer;
    std::string filePath ="./Resources/" + directoryPath + "/" + filename;
    const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);
    //アニメーションがない場合
	if(scene->mNumAnimations == 0) {
		return animations = {};
	}

	//複数のアニメーションの情報を取得
	for (uint32_t animationIndex = 0; animationIndex < scene->mNumAnimations; ++animationIndex) {
		aiAnimation* animationAssimp = scene->mAnimations[animationIndex];
		//animationインスタンスの生成
		Animation* animation = new Animation();
		//時間の単位を秒に変換
		animation->duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);

		//assimpでは個々のNodeのAnimationをchannelと呼んでいるのでchannelを回してNodeAnimationの情報を取得する
		for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
			aiNodeAnim* NodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
			NodeAnimation& nodeAnimation = animation->nodeAnimations[NodeAnimationAssimp->mNodeName.C_Str()];

			//position, rotation, scaleのkeyflameを取得
			//position
			for (uint32_t keyIndex = 0; keyIndex < NodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
				aiVectorKey& keyAssimp = NodeAnimationAssimp->mPositionKeys[keyIndex];
				KeyflameVector3 keyflame;
				keyflame.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); //時間の単位を秒に変換
				keyflame.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z }; //右手->左手に変換するので手動で対処する
				nodeAnimation.translate.keyflames.push_back(keyflame);
			}
			//rotation
			for (uint32_t keyIndex = 0; keyIndex < NodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
				aiQuatKey& keyAssimp = NodeAnimationAssimp->mRotationKeys[keyIndex];
				KeyflameQuaternion keyflame;
				keyflame.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); //時間の単位を秒に変換
				keyflame.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w };
				nodeAnimation.rotate.keyflames.push_back(keyflame);
			}
			//scale
			for (uint32_t keyIndex = 0; keyIndex < NodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
				aiVectorKey& keyAssimp = NodeAnimationAssimp->mScalingKeys[keyIndex];
				KeyflameVector3 keyflame;
				keyflame.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond); //時間の単位を秒に変換
				keyflame.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
				nodeAnimation.scale.keyflames.push_back(keyflame);
			}

			//animation->nodeAnimations. = nodeAnimation;
		}

		//アニメーション名を取得
		std::string animationName = animationAssimp->mName.C_Str();
		animation->name = animationName;
		
		animations.insert(std::make_pair(animationName, animation));
	}


	return animations;
}

//=============================================================================
//	補間値の計算
//=============================================================================

Vector3 Animator::CalculateValue(const std::vector<KeyflameVector3>& keyframes, float time) {
	assert(!keyframes.empty()); //keyframesが空の場合はエラー
	if(keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}

	for(size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;
		if(time >= keyframes[index].time && time <= keyframes[nextIndex].time) {
			//範囲内を補間する
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Easing::Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}

	//最後のキーフレームを返す
	return (*keyframes.rbegin()).value;
}

Quaternion Animator::CalculateValue(const std::vector<KeyflameQuaternion>& keyframes, float time) {
	assert(!keyframes.empty()); //keyframesが空の場合はエラー
	if(keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}

	for(size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;
		if(time >= keyframes[index].time && time <= keyframes[nextIndex].time) {
			//範囲内を補間する
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Easing::Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}
	//最後のキーフレームを返す
	return (*keyframes.rbegin()).value;
}