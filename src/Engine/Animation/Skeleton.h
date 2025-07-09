#pragma once
#include "Mymath.h"
#include "Animation/NodeAnimation.h"
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <map>

struct Joint {
	QuaternionTransform transform;
	Matrix4x4 localMatrix;
	Matrix4x4 skeletonSpaceMatrix;
	std::string name;
	std::vector<int32_t> children;
	int32_t index;
	std::optional<uint32_t> parent;
};

class Skeleton {
public:

	void Create(const Node& node);

	void Update();

	void Draw();

	void ApplyAnimation(Animation* animation, float animationTime);

public:

	const int32_t GetRoot() const { return root; }

	const std::vector<Joint>& GetJoints() { return joints; }

	const std::map<std::string, int32_t>& GetJointMap() { return jointMap; }

private:

	int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent);

	int32_t root = -1;
	std::map<std::string, int32_t> jointMap;
	std::vector<Joint> joints;
};