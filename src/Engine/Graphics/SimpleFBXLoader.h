#pragma once
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <fstream>
#include "FBXModel.h"
#include "Math/Matrix4x4.h"

// FBX SDKを使わない簡易的なFBXローダー
// Assimp ライブラリを使用してFBXファイルを読み込む
class SimpleFBXLoader {
public:
    static SimpleFBXLoader* GetInstance();
    
    bool Initialize();
    void Finalize();
    
    // FBXファイルからモデルを読み込む
    std::unique_ptr<FBXModel> LoadModelFromFile(const std::string& filename);
    
private:
    SimpleFBXLoader() = default;
    ~SimpleFBXLoader() = default;
    SimpleFBXLoader(const SimpleFBXLoader&) = delete;
    SimpleFBXLoader& operator=(const SimpleFBXLoader&) = delete;
    
    // バイナリデータ読み込みヘルパー
    template<typename T>
    T ReadBinary(std::ifstream& stream) {
        T value;
        stream.read(reinterpret_cast<char*>(&value), sizeof(T));
        return value;
    }
    
    std::string ReadString(std::ifstream& stream, uint32_t length) {
        std::string result(length, '\0');
        stream.read(&result[0], length);
        return result;
    }
    
    // FBXバイナリフォーマット解析
    struct FBXNode {
        std::string name;
        std::vector<uint8_t> properties;
        std::vector<FBXNode> children;
    };
    
    bool ParseFBXBinary(const std::string& filename, FBXModel* model);
    bool ReadNode(std::ifstream& stream, FBXNode& node);
    void ProcessNode(const FBXNode& node, FBXModel* model);
    void ProcessGeometry(const FBXNode& node, FBXModel* model);
    void ProcessAnimation(const FBXNode& node, FBXModel* model);
    
    // アニメーションデータの一時保存
    struct TempAnimationData {
        std::string boneName;
        std::vector<float> times;
        std::vector<Vector3> positions;
        std::vector<Vector4> rotations;
        std::vector<Vector3> scales;
    };
    
    std::vector<TempAnimationData> tempAnimations_;
};