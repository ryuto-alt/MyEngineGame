#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "FBXModel.h"

// Forward declarations for FBX SDK types
namespace fbxsdk {
    class FbxManager;
    class FbxIOSettings;
    class FbxImporter;
    class FbxScene;
    class FbxNode;
    class FbxMesh;
    class FbxAnimStack;
    class FbxAnimLayer;
    class FbxAMatrix;
    class FbxVector4;
    class FbxQuaternion;
}

class FBXLoader {
public:
    static FBXLoader* GetInstance();
    
    bool Initialize();
    void Finalize();
    
    // FBXファイルからモデルを読み込む
    std::unique_ptr<FBXModel> LoadModelFromFile(const std::string& filename);
    
private:
    FBXLoader() = default;
    ~FBXLoader() = default;
    FBXLoader(const FBXLoader&) = delete;
    FBXLoader& operator=(const FBXLoader&) = delete;
    
    // FBX SDK マネージャー
    fbxsdk::FbxManager* fbxManager_ = nullptr;
    fbxsdk::FbxIOSettings* fbxIOSettings_ = nullptr;
    fbxsdk::FbxImporter* fbxImporter_ = nullptr;
    
    // ノード処理
    void ProcessNode(fbxsdk::FbxNode* node, FBXModel* model);
    void ProcessMesh(fbxsdk::FbxNode* node, FBXModel* model);
    
    // メッシュデータ読み込み
    void LoadVertices(fbxsdk::FbxMesh* fbxMesh, FBXModel::Mesh& mesh);
    void LoadIndices(fbxsdk::FbxMesh* fbxMesh, FBXModel::Mesh& mesh);
    void LoadNormals(fbxsdk::FbxMesh* fbxMesh, FBXModel::Mesh& mesh);
    void LoadUVs(fbxsdk::FbxMesh* fbxMesh, FBXModel::Mesh& mesh);
    void LoadSkinWeights(fbxsdk::FbxMesh* fbxMesh, FBXModel::Mesh& mesh, FBXModel* model);
    
    // ボーン読み込み
    void LoadBones(fbxsdk::FbxScene* scene, FBXModel* model);
    void ProcessBoneHierarchy(fbxsdk::FbxNode* node, int parentIndex, FBXModel* model);
    
    // アニメーション読み込み
    void LoadAnimations(fbxsdk::FbxScene* scene, FBXModel* model);
    void LoadAnimationStack(fbxsdk::FbxAnimStack* animStack, FBXModel* model);
    void LoadAnimationCurve(fbxsdk::FbxNode* node, fbxsdk::FbxAnimLayer* animLayer, 
                           FBXModel::Animation& animation, FBXModel* model);
    
    // マテリアル読み込み
    void LoadMaterials(fbxsdk::FbxScene* scene, FBXModel* model);
    
    // ヘルパー関数
    fbxsdk::FbxAMatrix GetGeometryTransformation(fbxsdk::FbxNode* node);
    Matrix4x4 ConvertFbxMatrixToMatrix4x4(const fbxsdk::FbxAMatrix& fbxMatrix);
    Vector3 ConvertFbxVector3(const fbxsdk::FbxVector4& fbxVector);
    Vector4 ConvertFbxQuaternion(const fbxsdk::FbxQuaternion& fbxQuat);
};