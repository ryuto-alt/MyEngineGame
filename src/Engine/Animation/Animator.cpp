#include "Animator.h"
#include "Mymath.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cassert>
#include <cmath>
#include <Windows.h>

namespace {
	Vector3 Lerp(const Vector3& start, const Vector3& end, float t) {
		return {
			(1.0f - t) * start.x + t * end.x,
			(1.0f - t) * start.y + t * end.y,
			(1.0f - t) * start.z + t * end.z
		};
	}

	Vector4 Normalize(const Vector4& q) {
		float length = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
		return { q.x / length, q.y / length, q.z / length, q.w / length };
	}

	float Dot(const Vector4& q1, const Vector4& q2) {
		return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
	}

	Vector4 operator*(float scalar, const Vector4& q) {
		return { scalar * q.x, scalar * q.y, scalar * q.z, scalar * q.w };
	}

	Vector4 operator+(const Vector4& q1, const Vector4& q2) {
		return { q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w };
	}

	Vector4 operator-(const Vector4& q) {
		return { -q.x, -q.y, -q.z, -q.w };
	}

	Vector4 Slerp(const Vector4& q0, const Vector4& q1, float t) {
		float dot = Dot(q0, q1);

		Vector4 q0_fixed = q0;
		if (dot < 0.0f) {
			q0_fixed = -q0;
			dot = -dot;
		}

		const float THRESHOLD = 0.9995f;
		if (dot > THRESHOLD) {
			Vector4 result = (1.0f - t) * q0_fixed + t * q1;
			return Normalize(result);
		}

		float theta = std::acos(dot);
		float sinTheta = std::sin(theta);

		float scale0 = std::sin((1.0f - t) * theta) / sinTheta;
		float scale1 = std::sin(t * theta) / sinTheta;

		return scale0 * q0_fixed + scale1 * q1;
	}
}

void Animator::Finalize() {
	for (auto& animationSet : animations_) {
		for (auto& animation : animationSet.second) {
			delete animation.second;
		}
	}
	animations_.clear();
}

void Animator::LoadAnimation(const std::string& filePath) {
	if (animations_.contains(filePath)) {
		return;
	}

	std::map<std::string, Animation*> animation = LoadAnimationFile("Models/human", filePath);
	animations_.insert(std::make_pair(filePath, animation));
}

Animation* Animator::FindAnimation(const std::string& filePath, const std::string& animName) {
	if (animations_.contains(filePath)) {
		if (animations_.at(filePath).contains(animName)) {
			return animations_.at(filePath).at(animName);
		}
		// 指定されたアニメーション名が見つからない場合、最初のアニメーションを返す
		if (!animations_.at(filePath).empty()) {
			return animations_.at(filePath).begin()->second;
		}
	}

	return nullptr;
}

std::vector<std::string> Animator::GetAnimationNames(const std::string& filePath) {
	std::vector<std::string> names;
	if (animations_.contains(filePath)) {
		for (const auto& pair : animations_.at(filePath)) {
			names.push_back(pair.first);
		}
	}
	return names;
}

std::map<std::string, Animation*> Animator::LoadAnimationFile(const std::string& directoryPath, const std::string& filename) {
	std::map<std::string, Animation*> animations = {};
	Assimp::Importer importer;
	std::string filePath = "./Resources/" + directoryPath + "/" + filename;
	const aiScene* scene = importer.ReadFile(filePath.c_str(), 0);

	if (!scene || scene->mNumAnimations == 0) {
		return animations;
	}

	for (uint32_t animationIndex = 0; animationIndex < scene->mNumAnimations; ++animationIndex) {
		aiAnimation* animationAssimp = scene->mAnimations[animationIndex];
		Animation* animation = new Animation();
		animation->duration = float(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);

		for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; ++channelIndex) {
			aiNodeAnim* NodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
			NodeAnimation& nodeAnimation = animation->nodeAnimations[NodeAnimationAssimp->mNodeName.C_Str()];

			for (uint32_t keyIndex = 0; keyIndex < NodeAnimationAssimp->mNumPositionKeys; ++keyIndex) {
				aiVectorKey& keyAssimp = NodeAnimationAssimp->mPositionKeys[keyIndex];
				KeyflameVector3 keyflame;
				keyflame.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
				keyflame.value = { -keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
				nodeAnimation.translate.keyflames.push_back(keyflame);
			}

			for (uint32_t keyIndex = 0; keyIndex < NodeAnimationAssimp->mNumRotationKeys; ++keyIndex) {
				aiQuatKey& keyAssimp = NodeAnimationAssimp->mRotationKeys[keyIndex];
				KeyflameQuaternion keyflame;
				keyflame.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
				keyflame.value = { keyAssimp.mValue.x, -keyAssimp.mValue.y, -keyAssimp.mValue.z, keyAssimp.mValue.w };
				nodeAnimation.rotate.keyflames.push_back(keyflame);
			}

			for (uint32_t keyIndex = 0; keyIndex < NodeAnimationAssimp->mNumScalingKeys; ++keyIndex) {
				aiVectorKey& keyAssimp = NodeAnimationAssimp->mScalingKeys[keyIndex];
				KeyflameVector3 keyflame;
				keyflame.time = float(keyAssimp.mTime / animationAssimp->mTicksPerSecond);
				keyflame.value = { keyAssimp.mValue.x, keyAssimp.mValue.y, keyAssimp.mValue.z };
				nodeAnimation.scale.keyflames.push_back(keyflame);
			}
		}

		std::string animationName = animationAssimp->mName.C_Str();
		// アニメーション名が空の場合、デフォルト名を使用
		if (animationName.empty()) {
			animationName = "default_animation_" + std::to_string(animationIndex);
		}
		animation->name = animationName;
		
		// デバッグ情報を出力
		char debugMsg[256];
		sprintf_s(debugMsg, "Animator: Loaded animation '%s' from %s\n", animationName.c_str(), filename.c_str());
		OutputDebugStringA(debugMsg);

		animations.insert(std::make_pair(animationName, animation));
	}

	return animations;
}

Vector3 Animator::CalculateValue(const std::vector<KeyflameVector3>& keyframes, float time) {
	assert(!keyframes.empty());
	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}

	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;
		if (time >= keyframes[index].time && time <= keyframes[nextIndex].time) {
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}

	return keyframes.back().value;
}

Vector4 Animator::CalculateValue(const std::vector<KeyflameQuaternion>& keyframes, float time) {
	assert(!keyframes.empty());
	if (keyframes.size() == 1 || time <= keyframes[0].time) {
		return keyframes[0].value;
	}

	for (size_t index = 0; index < keyframes.size() - 1; ++index) {
		size_t nextIndex = index + 1;
		if (time >= keyframes[index].time && time <= keyframes[nextIndex].time) {
			float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
			return Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
		}
	}

	return keyframes.back().value;
}