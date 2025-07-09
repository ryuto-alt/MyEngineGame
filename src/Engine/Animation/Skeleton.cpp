#include "Skeleton.h"
#include "Animator.h"

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
	for (Joint& joint : joints) {
		if (auto it = animation->nodeAnimations.find(joint.name); it != animation->nodeAnimations.end()) {
			const NodeAnimation& rootNodeAnimation = (*it).second;
			joint.transform.scale = Animator::CalculateValue(rootNodeAnimation.scale.keyflames, animationTime);
			joint.transform.rotate = Animator::CalculateValue(rootNodeAnimation.rotate.keyflames, animationTime);
			joint.transform.translate = Animator::CalculateValue(rootNodeAnimation.translate.keyflames, animationTime);
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