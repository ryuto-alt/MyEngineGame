#include "AnimationUtility.h"
#include <cassert>
#include <cmath>
#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Windows.h>
#include <string>
#include "Mymath.h"



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

// アニメーション読み込み関数
Animation LoadAnimationFile(const std::string& directoryPath, const std::string& filename) {
    Animation animation;
    
    // assimpでファイルを読み込み
    Assimp::Importer importer;
    std::string fullPath = directoryPath + "/" + filename;
    
    const aiScene* scene = importer.ReadFile(fullPath,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenSmoothNormals
    );
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        OutputDebugStringA(("LoadAnimationFile: Error loading file: " + std::string(importer.GetErrorString()) + "\n").c_str());
        
        // エラーの場合はダミーアニメーションを返す
        animation.duration = 2.0f;
        NodeAnimation dummyNodeAnimation;
        KeyframeVector3 scaleKey;
        scaleKey.time = 0.0f;
        scaleKey.value = { 1.0f, 1.0f, 1.0f };
        dummyNodeAnimation.scale.push_back(scaleKey);
        animation.nodeAnimations["root"] = dummyNodeAnimation;
        return animation;
    }
    
    // アニメーションが存在しない場合
    if (scene->mNumAnimations == 0) {
        OutputDebugStringA("LoadAnimationFile: No animations found in file\n");
        animation.duration = 0.0f;
        return animation;
    }
    
    // 最初のアニメーションを処理
    const aiAnimation* assimpAnimation = scene->mAnimations[0];
    
    OutputDebugStringA(("LoadAnimationFile: Processing animation with " + std::to_string(assimpAnimation->mNumChannels) + " channels\n").c_str());
    
    // アニメーション時間を設定
    animation.duration = static_cast<float>(assimpAnimation->mDuration / assimpAnimation->mTicksPerSecond);
    animation.nodeAnimations.clear();
    
    // 各ノードアニメーションチャンネルを処理
    for (unsigned int i = 0; i < assimpAnimation->mNumChannels; i++) {
        const aiNodeAnim* nodeAnim = assimpAnimation->mChannels[i];
        std::string nodeName = nodeAnim->mNodeName.C_Str();
        
        NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeName];
        
        // 位置キーフレーム（右手座標系→左手座標系：Z座標を反転）
        for (unsigned int j = 0; j < nodeAnim->mNumPositionKeys; j++) {
            const aiVectorKey& key = nodeAnim->mPositionKeys[j];
            KeyframeVector3 keyframe;
            keyframe.time = static_cast<float>(key.mTime / assimpAnimation->mTicksPerSecond);
            keyframe.value = {key.mValue.x, key.mValue.y, -key.mValue.z};  // Z座標を反転
            nodeAnimation.translate.push_back(keyframe);
        }
        
        // 回転キーフレーム（右手座標系→左手座標系：クォータニオンの共役を取る）
        for (unsigned int j = 0; j < nodeAnim->mNumRotationKeys; j++) {
            const aiQuatKey& key = nodeAnim->mRotationKeys[j];
            KeyframeQuaternion keyframe;
            keyframe.time = static_cast<float>(key.mTime / assimpAnimation->mTicksPerSecond);
            
            // 右手座標系から左手座標系への変換：
            // 座標系変換のためクォータニオンの共役を取る（x,y,z成分の符号を反転）
            keyframe.value = {-key.mValue.x, -key.mValue.y, -key.mValue.z, key.mValue.w};
            nodeAnimation.rotate.push_back(keyframe);
        }
        
        // スケールキーフレーム（スケールは座標系に依存しない）
        for (unsigned int j = 0; j < nodeAnim->mNumScalingKeys; j++) {
            const aiVectorKey& key = nodeAnim->mScalingKeys[j];
            KeyframeVector3 keyframe;
            keyframe.time = static_cast<float>(key.mTime / assimpAnimation->mTicksPerSecond);
            keyframe.value = {key.mValue.x, key.mValue.y, key.mValue.z};
            nodeAnimation.scale.push_back(keyframe);
        }
        
       ///OutputDebugStringA(("LoadAnimationFile: Node " + nodeName + " - Position keys: " + std::to_string(nodeAnimation.translate.size()) + 
       ///                  ", Rotation keys: " + std::to_string(nodeAnimation.rotate.size()) + 
       ///                  ", Scale keys: " + std::to_string(nodeAnimation.scale.size()) + "\n").c_str());
    }
    
    ///OutputDebugStringA(("LoadAnimationFile: Animation duration: " + std::to_string(animation.duration) + " seconds\n").c_str());
    
    return animation;
}