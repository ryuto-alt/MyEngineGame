#include "Skeleton.h"
#include "Animator.h"
#include <Windows.h>
#include <algorithm>
#include <cstdio>

void Skeleton::Create(const Node& rootNode) {
	root = CreateJoint(rootNode, {});

	for (const Joint& joint : joints) {
		jointMap.emplace(joint.name, joint.index);
	}

	for (Joint& joint : joints) {
		joint.localMatrix = MakeAffineMatrix(
			joint.transform.scale,
			joint.transform.rotate,
			joint.transform.translate);

		if (joint.parent) {
			joint.skeletonSpaceMatrix = Multiply(joint.localMatrix, joints[*joint.parent].skeletonSpaceMatrix);
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Skeleton::Update() {
	for (Joint& joint : joints) {
		joint.localMatrix = MakeAffineMatrix(
			joint.transform.scale,
			joint.transform.rotate,
			joint.transform.translate);

		if (joint.parent) {
			joint.skeletonSpaceMatrix = Multiply(joint.localMatrix, joints[*joint.parent].skeletonSpaceMatrix);
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Skeleton::Draw() {

}

void Skeleton::ApplyAnimation(Animation* animation, float animationTime) {
	int appliedCount = 0;
	for (Joint& joint : joints) {
		if (auto it = animation->nodeAnimations.find(joint.name); it != animation->nodeAnimations.end()) {
			const NodeAnimation& rootNodeAnimation = (*it).second;
			joint.transform.scale = Animator::CalculateValue(rootNodeAnimation.scale.keyflames, animationTime);
			joint.transform.rotate = Animator::CalculateValue(rootNodeAnimation.rotate.keyflames, animationTime);
			joint.transform.translate = Animator::CalculateValue(rootNodeAnimation.translate.keyflames, animationTime);
			appliedCount++;
		}
	}
	
	// デバッグ出力
	char debugMsg[256];
	sprintf_s(debugMsg, "Skeleton::ApplyAnimation: Applied to %d/%d joints at time=%.2f\n", 
	         appliedCount, (int)joints.size(), animationTime);
	OutputDebugStringA(debugMsg);
	
	// 最初の数回だけジョイント名を詳細出力
	static int debugCounter = 0;
	if (debugCounter < 3) {
		debugCounter++;
		OutputDebugStringA("Joint names vs Animation nodes:\n");
		size_t maxJoints = joints.size() < 5 ? joints.size() : 5;
		for (size_t i = 0; i < maxJoints; ++i) {
			sprintf_s(debugMsg, "  Joint[%d]: '%s' -> %s\n", 
			         (int)i, joints[i].name.c_str(), 
			         animation->nodeAnimations.count(joints[i].name) ? "FOUND" : "NOT FOUND");
			OutputDebugStringA(debugMsg);
		}
		
		OutputDebugStringA("Available animation nodes:\n");
		int nodeCount = 0;
		for (const auto& pair : animation->nodeAnimations) {
			if (nodeCount < 5) {
				sprintf_s(debugMsg, "  Node[%d]: '%s'\n", nodeCount, pair.first.c_str());
				OutputDebugStringA(debugMsg);
			}
			nodeCount++;
		}
	}
}

int32_t Skeleton::CreateJoint(const Node& node, const std::optional<int32_t>& parent) {
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = static_cast<int32_t>(joints.size());
	joint.parent = parent;
	joints.push_back(joint);
	
	for (const Node& child : node.children) {
		int32_t childIndex = CreateJoint(child, joint.index);
		joints[joint.index].children.push_back(childIndex);
	}

	return joint.index;
}