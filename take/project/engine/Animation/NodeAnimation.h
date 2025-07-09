#pragma once
#include "Animation/Keyflame.h"
#include <vector>
#include <string>
#include <map>


template <typename T>
struct AnimationCurve {
	std::vector<Keyflame<T>> keyflames;
};

struct NodeAnimation {
	AnimationCurve<Vector3> translate;
	AnimationCurve<Quaternion> rotate;
	AnimationCurve<Vector3> scale;
};

struct Animation {
	std::string name;
	float duration; //アニメーションの全体の尺(秒単位)
	std::map<std::string, NodeAnimation> nodeAnimations;
};
