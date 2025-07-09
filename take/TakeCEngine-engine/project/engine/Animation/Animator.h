#pragma once
#include "Animation/NodeAnimation.h"
#include "Matrix4x4.h"
#include <string>
#include <unordered_map>

class Animator {
public:

	Animator() = default;
	~Animator() = default;

	void Finalize();

	void LoadAnimation(const std::string& filePath);

	Animation* FindAnimation(const std::string& filePath,const std::string& animName);

	static std::map<std::string, Animation*> LoadAnimationFile(const std::string& directoryPath, const std::string& filename);
	
	/// <summary>
	/// ベクトルの補間値を計算
	/// </summary>
	static Vector3 CalculateValue(const std::vector<KeyflameVector3>& keyframes, float time);

	/// <summary>
	/// クォータニオンの補間値を計算
	/// </summary>
	/// <param name="keyframes"></param>
	/// <param name="time"></param>
	/// <returns></returns>
	static Quaternion CalculateValue(const std::vector<KeyflameQuaternion>& keyframes, float time);

private:

	/// <summary>
	/// アニメーションのコンテナ
	/// (ファイル名, (アニメーション名,アニメーション))
	/// </summary>
	std::map<std::string,std::map<std::string, Animation*>> animations_;
};

