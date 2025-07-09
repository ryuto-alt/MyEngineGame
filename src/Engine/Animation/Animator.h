#pragma once
#include "Animation/NodeAnimation.h"
#include "Matrix4x4.h"
#include "Vector3.h"
#include "Vector4.h"
#include <string>
#include <unordered_map>
#include <vector>

class Animator {
public:

	Animator() = default;
	~Animator() = default;

	void Finalize();

	void LoadAnimation(const std::string& filePath);

	Animation* FindAnimation(const std::string& filePath, const std::string& animName);
	
	// 利用可能なアニメーション名を取得
	std::vector<std::string> GetAnimationNames(const std::string& filePath);

	static std::map<std::string, Animation*> LoadAnimationFile(const std::string& filePath);

	static Vector3 CalculateValue(const std::vector<KeyflameVector3>& keyframes, float time);

	static Vector4 CalculateValue(const std::vector<KeyflameQuaternion>& keyframes, float time);

private:

	std::map<std::string, std::map<std::string, Animation*>> animations_;
};