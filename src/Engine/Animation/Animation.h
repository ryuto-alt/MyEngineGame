#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include <vector>
#include <map>
#include <string>

// Quaternionはベクトル4次元として定義
using Quaternion = Vector4;

// Vector3のキーフレーム構造体
struct KeyframeVector3 {
    Vector3 value;  // キーフレームの値
    float time;     // キーフレームの時刻（単位は秒）
};

// Quaternionのキーフレーム構造体
struct KeyframeQuaternion {
    Quaternion value;  // キーフレームの値
    float time;        // キーフレームの時刻（単位は秒）
};

// Nodeのアニメーション（translate, rotate, scaleのキーフレーム配列）
struct NodeAnimation {
    std::vector<KeyframeVector3> translate;    // 平行移動のキーフレーム
    std::vector<KeyframeQuaternion> rotate;    // 回転のキーフレーム
    std::vector<KeyframeVector3> scale;        // スケールのキーフレーム
};

// アニメーション全体を表すクラス
struct Animation {
    float duration;  // アニメーション全体の尺（単位は秒）
    std::map<std::string, NodeAnimation> nodeAnimations;  // NodeAnimationの集合。Node名で引けるようにstd::mapで格納
};