#include "AnimationUtility.h"
#include <cassert>
#include <cmath>
#include <algorithm>

// 線形補間関数（削除：Mymath.cppで定義済み）

// クォータニオンの内積
float Dot(const Quaternion& q1, const Quaternion& q2) {
    return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

// クォータニオンの正規化
Quaternion Normalize(const Quaternion& q) {
    float length = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (length == 0.0f) {
        return { 0.0f, 0.0f, 0.0f, 1.0f };
    }
    return {
        q.x / length,
        q.y / length,
        q.z / length,
        q.w / length
    };
}



// 指定した時刻のVector3値を計算（線形補間）
Vector3 CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) {
    assert(!keyframes.empty()); // キーがないものは返す値がわからないのでダメ
    
    // キーが1つか、時刻がキーフレーム前なら最初の値を返す
    if (keyframes.size() == 1 || time <= keyframes[0].time) {
        return keyframes[0].value;
    }
    
    // 時刻範囲を探索して補間
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // indexとnextIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内を確認する
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Lerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    
    // ここまできた場合は一番後の時刻よりも後ろなので最後の値を返すことにする
    return keyframes.back().value;
}

// 指定した時刻のQuaternion値を計算（球面線形補間）
Quaternion CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time) {
    assert(!keyframes.empty()); // キーがないものは返す値がわからないのでダメ
    
    // キーが1つか、時刻がキーフレーム前なら最初の値を返す
    if (keyframes.size() == 1 || time <= keyframes[0].time) {
        return keyframes[0].value;
    }
    
    // 時刻範囲を探索して補間
    for (size_t index = 0; index < keyframes.size() - 1; ++index) {
        size_t nextIndex = index + 1;
        // indexとnextIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
        if (keyframes[index].time <= time && time <= keyframes[nextIndex].time) {
            // 範囲内を確認する
            float t = (time - keyframes[index].time) / (keyframes[nextIndex].time - keyframes[index].time);
            return Slerp(keyframes[index].value, keyframes[nextIndex].value, t);
        }
    }
    
    // ここまできた場合は一番後の時刻よりも後ろなので最後の値を返すことにする
    return keyframes.back().value;
}

// アニメーション読み込み関数（外部ライブラリなし版 - テスト用ダミーデータ）
Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename) {
    Animation animation;
    
    // ダミーアニメーションデータを作成（実際のファイル読み込みは後で実装）
    animation.duration = 2.0f; // 2秒のアニメーション
    
    // ルートノードのアニメーションを作成
    NodeAnimation rootNodeAnimation;
    
    // 回転アニメーション（Y軸周りの360度回転）
    KeyframeQuaternion rotKey1, rotKey2, rotKey3;
    rotKey1.time = 0.0f;
    rotKey1.value = { 0.0f, 0.0f, 0.0f, 1.0f }; // 回転なし
    
    rotKey2.time = 1.0f;
    rotKey2.value = { 0.0f, 0.707f, 0.0f, 0.707f }; // Y軸周り180度
    
    rotKey3.time = 2.0f;
    rotKey3.value = { 0.0f, 1.0f, 0.0f, 0.0f }; // Y軸周り360度
    
    rootNodeAnimation.rotate.push_back(rotKey1);
    rootNodeAnimation.rotate.push_back(rotKey2);
    rootNodeAnimation.rotate.push_back(rotKey3);
    
    // スケールアニメーション（固定）
    KeyframeVector3 scaleKey;
    scaleKey.time = 0.0f;
    scaleKey.value = { 1.0f, 1.0f, 1.0f };
    rootNodeAnimation.scale.push_back(scaleKey);
    
    // 平行移動アニメーション（固定）
    KeyframeVector3 translateKey;
    translateKey.time = 0.0f;
    translateKey.value = { 0.0f, 0.0f, 0.0f };
    rootNodeAnimation.translate.push_back(translateKey);
    
    // ルートノードアニメーションを追加（rootとAnimatedCube両方に対応）
    animation.nodeAnimations["root"] = rootNodeAnimation;
    animation.nodeAnimations["AnimatedCube"] = rootNodeAnimation;
    
    return animation;
}