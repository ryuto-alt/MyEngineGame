#include "BlenderJSONLoader.h"
#include "JsonParser.h"
#include "Object3d.h"
#include "Model.h"
#include "DirectXCommon.h"
#include "../Utility/Logger.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

BlenderJSONLoader::BlenderJSONLoader() {
}

BlenderJSONLoader::~BlenderJSONLoader() {
}

bool BlenderJSONLoader::LoadScene(const std::string& jsonPath, DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    // JSONファイルを読み込み
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        errorMessage_ = "Failed to open JSON file: " + jsonPath;
        Logger::Log(errorMessage_);
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string jsonContent = buffer.str();
    file.close();

    // シーンデータを解析
    SceneData sceneData;
    if (!ParseSceneData(jsonContent, sceneData)) {
        return false;
    }

    // ベースパスを取得（JSONファイルのディレクトリ）
    std::filesystem::path basePath = std::filesystem::path(jsonPath).parent_path();

    // 各オブジェクトを処理
    for (const auto& obj : sceneData.objects) {
        if (obj.type == "MESH") {
            if (!LoadMeshObject(obj, basePath.string(), dxCommon, spriteCommon)) {
                Logger::Log("Warning: Failed to load mesh object: " + obj.name);
                // 続行（他のオブジェクトも読み込みたい）
            }
        }
        // TODO: LIGHTやCAMERAの処理も将来的に追加可能
    }

    Logger::Log("Loaded " + std::to_string(loadedMeshes_.size()) + " mesh objects from Blender JSON");
    return true;
}

bool BlenderJSONLoader::ParseSceneData(const std::string& jsonContent, SceneData& sceneData) {
    Json::Parser parser;
    auto root = parser.Parse(jsonContent);
    
    if (!root || !root->IsObject()) {
        errorMessage_ = "Invalid JSON format: " + parser.GetErrorMessage();
        Logger::Log(errorMessage_);
        return false;
    }

    // シーン名を取得
    auto nameNode = root->Get("name");
    if (nameNode && nameNode->IsString()) {
        sceneData.name = nameNode->AsString();
    }

    // オブジェクト配列を取得
    auto objectsNode = root->Get("objects");
    if (!objectsNode || !objectsNode->IsArray()) {
        errorMessage_ = "JSON missing 'objects' array";
        Logger::Log(errorMessage_);
        return false;
    }

    // 各オブジェクトを解析
    for (size_t i = 0; i < objectsNode->Size(); ++i) {
        auto objNode = objectsNode->Get(i);
        if (!objNode || !objNode->IsObject()) continue;

        BlenderObject obj;

        // タイプ
        auto typeNode = objNode->Get("type");
        if (typeNode && typeNode->IsString()) {
            obj.type = typeNode->AsString();
        }

        // 名前
        auto nameNode = objNode->Get("name");
        if (nameNode && nameNode->IsString()) {
            obj.name = nameNode->AsString();
        }

        // Transform
        auto transformNode = objNode->Get("transform");
        if (transformNode && transformNode->IsObject()) {
            // Translation
            auto translationNode = transformNode->Get("translation");
            if (translationNode && translationNode->IsArray() && translationNode->Size() >= 3) {
                obj.transform.translate.x = static_cast<float>(translationNode->Get(0)->AsNumber());
                obj.transform.translate.y = static_cast<float>(translationNode->Get(1)->AsNumber());
                obj.transform.translate.z = static_cast<float>(translationNode->Get(2)->AsNumber());
            }

            // Rotation (degrees to radians)
            auto rotationNode = transformNode->Get("rotation");
            if (rotationNode && rotationNode->IsArray() && rotationNode->Size() >= 3) {
                obj.transform.rotate.x = DegToRad(static_cast<float>(rotationNode->Get(0)->AsNumber()));
                obj.transform.rotate.y = DegToRad(static_cast<float>(rotationNode->Get(1)->AsNumber()));
                obj.transform.rotate.z = DegToRad(static_cast<float>(rotationNode->Get(2)->AsNumber()));
            }

            // Scaling
            auto scalingNode = transformNode->Get("scaling");
            if (scalingNode && scalingNode->IsArray() && scalingNode->Size() >= 3) {
                obj.transform.scale.x = static_cast<float>(scalingNode->Get(0)->AsNumber());
                obj.transform.scale.y = static_cast<float>(scalingNode->Get(1)->AsNumber());
                obj.transform.scale.z = static_cast<float>(scalingNode->Get(2)->AsNumber());
            }
        }

        // ファイル名（オプション）
        auto fileNameNode = objNode->Get("file_name");
        if (fileNameNode && fileNameNode->IsString()) {
            obj.fileName = fileNameNode->AsString();
        }

        sceneData.objects.push_back(obj);
    }

    return true;
}

bool BlenderJSONLoader::LoadMeshObject(const BlenderObject& obj, const std::string& basePath, DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    // OBJファイルのパスを構築
    std::string objPath;
    std::string cacheKey;
    
    // file_nameがある場合はそれを使用、なければobject名から推測
    if (!obj.fileName.empty()) {
        objPath = BuildObjectPath(basePath, obj.fileName);
        cacheKey = obj.fileName;  // file_nameをキャッシュキーとして使用
    } else {
        objPath = BuildObjectPath(basePath, obj.name);
        cacheKey = obj.name;  // nameをキャッシュキーとして使用
    }
    
    // モデルを読み込み（キャッシュチェック）
    std::unique_ptr<Model>* cachedModel = nullptr;
    auto it = modelCache_.find(cacheKey);
    if (it != modelCache_.end()) {
        cachedModel = &it->second;
    } else {
        // 新規読み込み
        auto model = std::make_unique<Model>();
        model->Initialize(dxCommon);
        
        // ディレクトリとファイル名を分離
        std::filesystem::path objFilePath(objPath);
        std::string directory = objFilePath.parent_path().string();
        std::string filename = objFilePath.filename().string();
        
        // OBJファイルを読み込み（void関数なのでエラーチェックなし）
        model->LoadFromObj(directory, filename);
        
        modelCache_[cacheKey] = std::move(model);
        cachedModel = &modelCache_[cacheKey];
    }

    // Object3dを作成
    auto object3d = std::make_unique<Object3d>();
    object3d->Initialize(dxCommon, spriteCommon);
    object3d->SetModel(cachedModel->get());

    // Transformを適用（Blenderの右手座標系からDirectXの左手座標系への変換）
    // Blender: X=右, Y=前, Z=上
    // DirectX: X=右, Y=上, Z=前
    Vector3 position;
    position.x = obj.transform.translate.x;   // X座標はそのまま（反転しない）
    position.y = obj.transform.translate.z;   // BlenderのZ（上下）をDirectXのY（上下）へ
    position.z = obj.transform.translate.y;   // BlenderのY（前後）をDirectXのZ（前後）へ
    object3d->SetPosition(position);
    
    // 回転も同様に軸を入れ替え
    Vector3 rotation;
    rotation.x = obj.transform.rotate.x;      // X軸回転はそのまま
    rotation.y = obj.transform.rotate.z;      // Z軸回転をY軸回転へ
    rotation.z = obj.transform.rotate.y;      // Y軸回転をZ軸回転へ
    object3d->SetRotation(rotation);
    
    // Blenderのスケール値をそのまま使用
    object3d->SetScale(obj.transform.scale);

    // LoadedMeshとして保存
    LoadedMesh loadedMesh;
    loadedMesh.object3d = std::move(object3d);
    loadedMesh.name = obj.name;
    loadedMesh.transform = obj.transform;
    
    loadedMeshes_.push_back(std::move(loadedMesh));
    
    Logger::Log("Loaded mesh: " + obj.name + " from " + objPath);
    return true;
}

std::string BlenderJSONLoader::BuildObjectPath(const std::string& basePath, const std::string& objectName) {
    // オブジェクト名に基づいてパスを構築
    // 例: "door" -> "Resources/Models/goal/goal.obj" (doorは存在しないのでgoalを使用)
    // 例: "key" -> "Resources/Models/key/key.obj"
    
    std::string lowerName = objectName;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    // basePathから相対パスを構築
    std::filesystem::path base(basePath);
    std::filesystem::path objPath;
    
    // 特殊なケース（ファイルが存在しない場合の代替マッピング）
    if (lowerName == "spring") {
        objPath = base / ".." / "Models" / "spring" / "Spring.obj";
    } else if (lowerName == "door") {
        // doorフォルダが存在しない場合、goalを代わりに使用
        objPath = base / ".." / "Models" / "goal" / "goal.obj";
    } else {
        // デフォルトケース（小文字フォルダ/小文字ファイル.obj）
        objPath = base / ".." / "Models" / lowerName / (lowerName + ".obj");
    }
    
    // パスを正規化（..を解決）して文字列として返す
    return objPath.lexically_normal().string();
}

Object3d* BlenderJSONLoader::GetMeshByName(const std::string& name) const {
    for (const auto& mesh : loadedMeshes_) {
        if (mesh.name == name) {
            return mesh.object3d.get();
        }
    }
    return nullptr;
}