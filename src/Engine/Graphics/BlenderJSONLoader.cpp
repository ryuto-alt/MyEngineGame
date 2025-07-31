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

    SceneData sceneData;
    if (!ParseSceneData(jsonContent, sceneData)) {
        return false;
    }

    std::filesystem::path basePath = std::filesystem::path(jsonPath).parent_path();

    for (const auto& obj : sceneData.objects) {
        if (obj.type == "MESH") {
            if (!LoadMeshObject(obj, basePath.string(), dxCommon, spriteCommon)) {
                Logger::Log("Warning: Failed to load mesh object: " + obj.name);
            }
        }
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

    auto nameNode = root->Get("name");
    if (nameNode && nameNode->IsString()) {
        sceneData.name = nameNode->AsString();
    }

    auto objectsNode = root->Get("objects");
    if (!objectsNode || !objectsNode->IsArray()) {
        errorMessage_ = "JSON missing 'objects' array";
        Logger::Log(errorMessage_);
        return false;
    }

    for (size_t i = 0; i < objectsNode->Size(); ++i) {
        auto objNode = objectsNode->Get(i);
        if (!objNode || !objNode->IsObject()) continue;

        BlenderObject obj;

        auto typeNode = objNode->Get("type");
        if (typeNode && typeNode->IsString()) {
            obj.type = typeNode->AsString();
        }

        auto nameNode = objNode->Get("name");
        if (nameNode && nameNode->IsString()) {
            obj.name = nameNode->AsString();
        }

        auto transformNode = objNode->Get("transform");
        if (transformNode && transformNode->IsObject()) {
            auto translationNode = transformNode->Get("translation");
            if (translationNode && translationNode->IsArray() && translationNode->Size() >= 3) {
                obj.transform.translate.x = static_cast<float>(translationNode->Get(0)->AsNumber());
                obj.transform.translate.y = static_cast<float>(translationNode->Get(1)->AsNumber());
                obj.transform.translate.z = static_cast<float>(translationNode->Get(2)->AsNumber());
            }

            auto rotationNode = transformNode->Get("rotation");
            if (rotationNode && rotationNode->IsArray() && rotationNode->Size() >= 3) {
                obj.transform.rotate.x = DegToRad(static_cast<float>(rotationNode->Get(0)->AsNumber()));
                obj.transform.rotate.y = DegToRad(static_cast<float>(rotationNode->Get(1)->AsNumber()));
                obj.transform.rotate.z = DegToRad(static_cast<float>(rotationNode->Get(2)->AsNumber()));
            }

            auto scalingNode = transformNode->Get("scaling");
            if (scalingNode && scalingNode->IsArray() && scalingNode->Size() >= 3) {
                obj.transform.scale.x = static_cast<float>(scalingNode->Get(0)->AsNumber());
                obj.transform.scale.y = static_cast<float>(scalingNode->Get(1)->AsNumber());
                obj.transform.scale.z = static_cast<float>(scalingNode->Get(2)->AsNumber());
            }
        }

        auto fileNameNode = objNode->Get("file_name");
        if (fileNameNode && fileNameNode->IsString()) {
            obj.fileName = fileNameNode->AsString();
        }

        sceneData.objects.push_back(obj);
    }

    return true;
}

bool BlenderJSONLoader::LoadMeshObject(const BlenderObject& obj, const std::string& basePath, DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    std::string objPath;
    std::string cacheKey;
    
    if (!obj.fileName.empty()) {
        objPath = BuildObjectPath(basePath, obj.fileName);
        cacheKey = obj.fileName;
    } else {
        objPath = BuildObjectPath(basePath, obj.name);
        cacheKey = obj.name;
    }
    
    std::unique_ptr<Model>* cachedModel = nullptr;
    auto it = modelCache_.find(cacheKey);
    if (it != modelCache_.end()) {
        cachedModel = &it->second;
    } else {
        auto model = std::make_unique<Model>();
        model->Initialize(dxCommon);
        
        std::filesystem::path objFilePath(objPath);
        std::string directory = objFilePath.parent_path().string();
        std::string filename = objFilePath.filename().string();
        
        model->LoadFromObj(directory, filename);
        
        modelCache_[cacheKey] = std::move(model);
        cachedModel = &modelCache_[cacheKey];
    }

    auto object3d = std::make_unique<Object3d>();
    object3d->Initialize(dxCommon, spriteCommon);
    object3d->SetModel(cachedModel->get());

    Vector3 position;
    position.x = obj.transform.translate.x;   
    position.y = obj.transform.translate.z;   
    position.z = obj.transform.translate.y;   
    object3d->SetPosition(position);
    
    Vector3 rotation;
    rotation.x = obj.transform.rotate.x;     
    rotation.y = obj.transform.rotate.z;     
    rotation.z = obj.transform.rotate.y;     
    object3d->SetRotation(rotation);
    
    object3d->SetScale(obj.transform.scale);

    LoadedMesh loadedMesh;
    loadedMesh.object3d = std::move(object3d);
    loadedMesh.name = obj.name;
    loadedMesh.transform = obj.transform;
    
    loadedMeshes_.push_back(std::move(loadedMesh));
    
    Logger::Log("Loaded mesh: " + obj.name + " from " + objPath);
    return true;
}

std::string BlenderJSONLoader::BuildObjectPath(const std::string& basePath, const std::string& objectName) {
    std::string lowerName = objectName;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    std::filesystem::path base(basePath);
    std::filesystem::path objPath;
    
    if (lowerName == "spring") {
        objPath = base / ".." / "Models" / "spring" / "Spring.obj";
    } else if (lowerName == "door") {
        objPath = base / ".." / "Models" / "goal" / "goal.obj";
    } else {
        objPath = base / ".." / "Models" / lowerName / (lowerName + ".obj");
    }
    
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