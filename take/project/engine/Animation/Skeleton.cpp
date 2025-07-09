#include "Skeleton.h"
#include "MatrixMath.h"

void Skeleton::Create(const Node& rootNode) {

	root = CreateJoint(rootNode, {});

	//名前とindexのマッピングを行いアクセスしやすくする
	for (const Joint& joint : joints) {
		jointMap.emplace(joint.name, joint.index);
	}

	for (Joint& joint : joints) {

		//ローカル行列を更新
		joint.localMatrix = MatrixMath::MakeAffineMatrix(
			joint.transform.scale,
			joint.transform.rotate,
			joint.transform.translate);

		if (joint.parent) { //親がいる場合親の行列を掛ける
			joint.skeletonSpaceMatrix = joint.localMatrix * joints[*joint.parent].skeletonSpaceMatrix;
		} else { //親がいない場合は自身の行列をそのまま使う
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Skeleton::Update() {
	//全てのJointを更新
	for (Joint& joint : joints) {

		//ローカル行列を更新
		joint.localMatrix = MatrixMath::MakeAffineMatrix(
			joint.transform.scale,
			joint.transform.rotate,
			joint.transform.translate);

		if (joint.parent) { //親がいる場合親の行列を掛ける
			joint.skeletonSpaceMatrix = joint.localMatrix * joints[*joint.parent].skeletonSpaceMatrix;
		} else { //親がいない場合は自身の行列をそのまま使う
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}

void Skeleton::Draw() {


}

void Skeleton::ApplyAnimation(Animation* animation, float animationTime) {
	for (Joint& joint : joints) {
		//対象のJointのAnimationがあれば、値の適用を行う。
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
	joint.skeletonSpaceMatrix = MatrixMath::MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = static_cast<int32_t>(joints.size());
	joint.parent = parent;
	joints.push_back(joint); //SkeletonのJoint列に追加
	for (const Node& child : node.children) {
		//子Jointを作成して、そのIndexを登録
		int32_t childIndex = CreateJoint(child, joint.index);
		joints[joint.index].children.push_back(childIndex);
	}

	//自身のIndexを返す
	return joint.index;
}
