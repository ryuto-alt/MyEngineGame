#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "../Math/Mymath.h"

class Object3d;
class Model;
class DirectXCommon;
class SpriteCommon;

// Blender JSONシーンローダー
class BlenderJSONLoader {
public:
    // シーン内のオブジェクト情報
    struct BlenderObject {
        std::string type;        // "MESH", "LIGHT", "CAMERA"
        std::string name;        // オブジェクト名
        Transform transform;     // 変換情報
        std::string fileName;    // MESHの場合のファイル名（オプション）
    };

    // シーン情報
    struct SceneData {
        std::string name;
        std::vector<BlenderObject> objects;
    };

    // ロードされたメッシュオブジェクト
    struct LoadedMesh {
        std::unique_ptr<Object3d> object3d;
        std::string name;
        Transform transform;
    };

public:
    BlenderJSONLoader();
    ~BlenderJSONLoader();

    // シーンファイルを読み込み
    bool LoadScene(const std::string& jsonPath, DirectXCommon* dxCommon, SpriteCommon* spriteCommon);

    // ロードされたメッシュオブジェクトを取得
    const std::vector<LoadedMesh>& GetLoadedMeshes() const { return loadedMeshes_; }
    
    // 特定の名前のメッシュを取得
    Object3d* GetMeshByName(const std::string& name) const;

    // エラーメッセージを取得
    const std::string& GetErrorMessage() const { return errorMessage_; }

private:
    // JSONからシーンデータを解析
    bool ParseSceneData(const std::string& jsonContent, SceneData& sceneData);
    
    // MESHオブジェクトをロード
    bool LoadMeshObject(const BlenderObject& obj, const std::string& basePath, DirectXCommon* dxCommon, SpriteCommon* spriteCommon);
    
    // ファイル名からOBJファイルパスを構築
    std::string BuildObjectPath(const std::string& basePath, const std::string& objectName);
    
    // 角度（度）をラジアンに変換
    float DegToRad(float degrees) const { return degrees * 3.14159265359f / 180.0f; }

private:
    std::vector<LoadedMesh> loadedMeshes_;
    std::unordered_map<std::string, std::unique_ptr<Model>> modelCache_;  // モデルのキャッシュ
    std::string errorMessage_;
};