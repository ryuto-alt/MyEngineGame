#pragma once
#include "ResourceDataStructure.h"
#include "Transform.h"
#include "Matrix4x4.h"
#include "Animation/Animator.h"
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <map>

struct Joint {
	QuaternionTransform transform; 
	Matrix4x4 localMatrix;
	Matrix4x4 skeletonSpaceMatrix;    //SkeletonSpaceでの変換行列
	std::string name;
	std::vector<int32_t> children;    //子Jointのインデックス
	int32_t index;                    //自身のインデックス
	std::optional<uint32_t> parent;   //親Jointのインデックス
};

class Skeleton {
public:

	/// <summary>
	/// Skeletonの作成
	/// </summary>
	void Create(const Node& node);

	void Update();

	void Draw();

	/// <summary>
	/// アニメーションの適用
	/// </summary>
	void ApplyAnimation(Animation* animation,float animationTime);


public: //getter
	
	const int32_t GetRoot() const { return root; }

	const std::vector<Joint>& GetJoints() { return joints; }

	const std::map<std::string, int32_t>& GetJointMap() { return jointMap; }

private:

	/// <summary>
	/// NodeからJointを作成
	/// </summary>
	/// <param name="node"></param>
	/// <param name="parent"></param>
	/// <param name="joints"></param>
	/// <returns></returns>
	int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent);


	int32_t root; //RootJointのインデックス
	std::map<std::string, int32_t> jointMap; //Joint名からindexとのマップ
	std::vector<Joint> joints; //Jointのリスト
};