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
	AnimationCurve<Vector4> rotate;
	AnimationCurve<Vector3> scale;
};

struct Animation {
	std::string name;
	float duration;
	std::map<std::string, NodeAnimation> nodeAnimations;
};