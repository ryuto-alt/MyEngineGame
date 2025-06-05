// Windows.hのmin/maxマクロを無効化
#define NOMINMAX

// FBX SDKのグローバル名前空間を無効化
#define FBXSDK_NAMESPACE_USING 0

#include "FBXLoader.h"
#include <fbxsdk.h>
#include <cassert>
#include <algorithm>
#include <string>
#include <functional>
#include <DirectXMath.h>
#include <Windows.h>

using namespace DirectX;
using namespace fbxsdk;

FBXLoader* FBXLoader::GetInstance() {
    static FBXLoader instance;
    return &instance;
}

bool FBXLoader::Initialize() {
    // FBX Managerの作成
    fbxManager_ = FbxManager::Create();
    if (!fbxManager_) {
        return false;
    }
    
    // IOSettingsの作成
    fbxIOSettings_ = FbxIOSettings::Create(fbxManager_, IOSROOT);
    fbxManager_->SetIOSettings(fbxIOSettings_);
    
    // Importerの作成
    fbxImporter_ = FbxImporter::Create(fbxManager_, "");
    
    return true;
}

void FBXLoader::Finalize() {
    if (fbxImporter_) {
        fbxImporter_->Destroy();
        fbxImporter_ = nullptr;
    }
    
    if (fbxIOSettings_) {
        fbxIOSettings_->Destroy();
        fbxIOSettings_ = nullptr;
    }
    
    if (fbxManager_) {
        fbxManager_->Destroy();
        fbxManager_ = nullptr;
    }
}

std::unique_ptr<FBXModel> FBXLoader::LoadModelFromFile(const std::string& filename) {
    auto model = std::make_unique<FBXModel>();
    
    // ファイルを開く
    if (!fbxImporter_->Initialize(filename.c_str(), -1, fbxManager_->GetIOSettings())) {
        OutputDebugStringA(("Failed to initialize FBX importer: " + filename + "\n").c_str());
        return nullptr;
    }
    
    // シーンの作成
    FbxScene* scene = FbxScene::Create(fbxManager_, "myScene");
    
    // インポート
    if (!fbxImporter_->Import(scene)) {
        OutputDebugStringA("Failed to import FBX scene\n");
        scene->Destroy();
        return nullptr;
    }
    
    fbxImporter_->Destroy();
    fbxImporter_ = FbxImporter::Create(fbxManager_, "");
    
    // 三角形化
    FbxGeometryConverter converter(fbxManager_);
    converter.Triangulate(scene, true);
    
    // 単位をメートルに変換
    FbxSystemUnit::m.ConvertScene(scene);
    
    // 座標系をDirectX用に変換（右手系から左手系へ）
    FbxAxisSystem directXAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eLeftHanded);
    directXAxisSystem.ConvertScene(scene);
    
    // マテリアルの読み込み
    LoadMaterials(scene, model.get());
    
    // ボーンの読み込み
    LoadBones(scene, model.get());
    
    // ルートノードから処理開始
    FbxNode* rootNode = scene->GetRootNode();
    if (rootNode) {
        ProcessNode(rootNode, model.get());
    }
    
    // アニメーションの読み込み
    LoadAnimations(scene, model.get());
    
    // ログ出力
    OutputDebugStringA(("FBXLoader: Model loaded - Meshes: " + std::to_string(model->GetMeshes().size()) + 
                       ", Bones: " + std::to_string(model->GetBones().size()) + 
                       ", Animations: " + std::to_string(model->GetAnimations().size()) + "\n").c_str());
    
    // シーンの破棄
    scene->Destroy();
    
    return model;
}

void FBXLoader::ProcessNode(FbxNode* node, FBXModel* model) {
    FbxNodeAttribute* attribute = node->GetNodeAttribute();
    
    if (attribute) {
        switch (attribute->GetAttributeType()) {
        case FbxNodeAttribute::eMesh:
            ProcessMesh(node, model);
            break;
        }
    }
    
    // 子ノードを再帰的に処理
    for (int i = 0; i < node->GetChildCount(); i++) {
        ProcessNode(node->GetChild(i), model);
    }
}

void FBXLoader::ProcessMesh(FbxNode* node, FBXModel* model) {
    FbxMesh* fbxMesh = node->GetMesh();
    if (!fbxMesh) return;
    
    FBXModel::Mesh mesh;
    
    // 頂点データの読み込み
    LoadVertices(fbxMesh, mesh);
    LoadIndices(fbxMesh, mesh);
    LoadNormals(fbxMesh, mesh);
    LoadUVs(fbxMesh, mesh);
    LoadSkinWeights(fbxMesh, mesh, model);
    
    // マテリアルインデックスの設定
    if (node->GetMaterialCount() > 0) {
        FbxSurfaceMaterial* material = node->GetMaterial(0);
        for (size_t i = 0; i < model->GetMaterials().size(); i++) {
            if (model->GetMaterials()[i].name == material->GetName()) {
                mesh.materialIndex = static_cast<uint32_t>(i);
                break;
            }
        }
    }
    
    model->AddMesh(mesh);
}

void FBXLoader::LoadVertices(FbxMesh* fbxMesh, FBXModel::Mesh& mesh) {
    int vertexCount = fbxMesh->GetControlPointsCount();
    mesh.vertices.resize(vertexCount);
    
    FbxVector4* vertices = fbxMesh->GetControlPoints();
    
    for (int i = 0; i < vertexCount; i++) {
        mesh.vertices[i].position = ConvertFbxVector3(vertices[i]);
    }
}

void FBXLoader::LoadIndices(FbxMesh* fbxMesh, FBXModel::Mesh& mesh) {
    int polygonCount = fbxMesh->GetPolygonCount();
    
    for (int i = 0; i < polygonCount; i++) {
        int polygonSize = fbxMesh->GetPolygonSize(i);
        assert(polygonSize == 3); // 三角形化されているはず
        
        for (int j = 0; j < polygonSize; j++) {
            mesh.indices.push_back(fbxMesh->GetPolygonVertex(i, j));
        }
    }
}

void FBXLoader::LoadNormals(FbxMesh* fbxMesh, FBXModel::Mesh& mesh) {
    FbxGeometryElementNormal* normalElement = fbxMesh->GetElementNormal();
    if (!normalElement) return;
    
    int indexCount = static_cast<int>(mesh.indices.size());
    
    for (int i = 0; i < indexCount; i++) {
        int vertexIndex = mesh.indices[i];
        int normalIndex = 0;
        
        if (normalElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
            if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect) {
                normalIndex = vertexIndex;
            } else if (normalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
                normalIndex = normalElement->GetIndexArray().GetAt(vertexIndex);
            }
        } else if (normalElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
            if (normalElement->GetReferenceMode() == FbxGeometryElement::eDirect) {
                normalIndex = i;
            } else if (normalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
                normalIndex = normalElement->GetIndexArray().GetAt(i);
            }
        }
        
        FbxVector4 normal = normalElement->GetDirectArray().GetAt(normalIndex);
        mesh.vertices[vertexIndex].normal = ConvertFbxVector3(normal);
    }
}

void FBXLoader::LoadUVs(FbxMesh* fbxMesh, FBXModel::Mesh& mesh) {
    FbxGeometryElementUV* uvElement = fbxMesh->GetElementUV(0);
    if (!uvElement) return;
    
    int indexCount = static_cast<int>(mesh.indices.size());
    
    for (int i = 0; i < indexCount; i++) {
        int vertexIndex = mesh.indices[i];
        int uvIndex = 0;
        
        if (uvElement->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
            if (uvElement->GetReferenceMode() == FbxGeometryElement::eDirect) {
                uvIndex = vertexIndex;
            } else if (uvElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
                uvIndex = uvElement->GetIndexArray().GetAt(vertexIndex);
            }
        } else if (uvElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
            if (uvElement->GetReferenceMode() == FbxGeometryElement::eDirect) {
                uvIndex = i;
            } else if (uvElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
                uvIndex = uvElement->GetIndexArray().GetAt(i);
            }
        }
        
        FbxVector2 uv = uvElement->GetDirectArray().GetAt(uvIndex);
        mesh.vertices[vertexIndex].uv = Vector2(static_cast<float>(uv[0]), static_cast<float>(1.0 - uv[1]));
    }
}

void FBXLoader::LoadSkinWeights(FbxMesh* fbxMesh, FBXModel::Mesh& mesh, FBXModel* model) {
    int skinCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
    if (skinCount == 0) {
        // スキニングなしの場合、デフォルト値を設定
        for (auto& vertex : mesh.vertices) {
            vertex.boneWeights = Vector4(1, 0, 0, 0);
            vertex.boneIndices[0] = 0;
            vertex.boneIndices[1] = 0;
            vertex.boneIndices[2] = 0;
            vertex.boneIndices[3] = 0;
        }
        return;
    }
    
    FbxSkin* skin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(0, FbxDeformer::eSkin));
    int clusterCount = skin->GetClusterCount();
    
    // 各頂点のウェイト情報を初期化
    std::vector<std::vector<std::pair<int, float>>> vertexWeights(mesh.vertices.size());
    
    for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
        FbxCluster* cluster = skin->GetCluster(clusterIndex);
        
        // ボーンインデックスを取得
        std::string boneName = cluster->GetLink()->GetName();
        int boneIndex = model->GetBoneIndex(boneName);
        if (boneIndex < 0) continue;
        
        // このクラスターに影響を受ける頂点数
        int indexCount = cluster->GetControlPointIndicesCount();
        int* indices = cluster->GetControlPointIndices();
        double* weights = cluster->GetControlPointWeights();
        
        for (int i = 0; i < indexCount; i++) {
            int vertexIndex = indices[i];
            float weight = static_cast<float>(weights[i]);
            
            if (vertexIndex < static_cast<int>(vertexWeights.size())) {
                vertexWeights[vertexIndex].push_back({boneIndex, weight});
            }
        }
        
        // バインドポーズ行列を取得
        FbxAMatrix transformMatrix;
        FbxAMatrix transformLinkMatrix;
        
        // GetTransformMatrixはメッシュのワールド変換行列
        // GetTransformLinkMatrixはボーンのワールド変換行列
        cluster->GetTransformMatrix(transformMatrix);
        cluster->GetTransformLinkMatrix(transformLinkMatrix);
        
        // オフセット行列 = ボーンのワールド変換行列の逆行列 * メッシュのワールド変換行列
        FbxAMatrix offsetMatrix = transformMatrix.Inverse() * transformLinkMatrix;
        
        model->SetBoneOffsetMatrix(boneIndex, ConvertFbxMatrixToMatrix4x4(offsetMatrix));
    }
    
    // 各頂点のウェイトを正規化して設定
    for (size_t i = 0; i < mesh.vertices.size(); i++) {
        auto& weights = vertexWeights[i];
        
        // ウェイトを大きい順にソート
        std::sort(weights.begin(), weights.end(), 
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        // 最大4つまでのウェイトを設定
        float totalWeight = 0.0f;
        int numWeights = (std::min)(4, static_cast<int>(weights.size()));
        
        for (int j = 0; j < numWeights; j++) {
            totalWeight += weights[j].second;
        }
        
        if (totalWeight > 0.0f) {
            for (int j = 0; j < 4; j++) {
                if (j < numWeights) {
                    mesh.vertices[i].boneIndices[j] = weights[j].first;
                    if (j == 0) mesh.vertices[i].boneWeights.x = weights[j].second / totalWeight;
                    else if (j == 1) mesh.vertices[i].boneWeights.y = weights[j].second / totalWeight;
                    else if (j == 2) mesh.vertices[i].boneWeights.z = weights[j].second / totalWeight;
                    else if (j == 3) mesh.vertices[i].boneWeights.w = weights[j].second / totalWeight;
                } else {
                    mesh.vertices[i].boneIndices[j] = 0;
                    if (j == 0) mesh.vertices[i].boneWeights.x = 0.0f;
                    else if (j == 1) mesh.vertices[i].boneWeights.y = 0.0f;
                    else if (j == 2) mesh.vertices[i].boneWeights.z = 0.0f;
                    else if (j == 3) mesh.vertices[i].boneWeights.w = 0.0f;
                }
            }
        } else {
            // ウェイトがない場合はデフォルト値
            mesh.vertices[i].boneWeights = Vector4(1, 0, 0, 0);
            mesh.vertices[i].boneIndices[0] = 0;
            mesh.vertices[i].boneIndices[1] = 0;
            mesh.vertices[i].boneIndices[2] = 0;
            mesh.vertices[i].boneIndices[3] = 0;
        }
    }
}

void FBXLoader::LoadBones(FbxScene* scene, FBXModel* model) {
    // スキンデフォーマーから参照されているボーンを収集
    std::unordered_map<std::string, FbxNode*> boneNodes;
    std::vector<FbxNode*> orderedBoneNodes;
    
    // デバッグ: ノード構造を出力
    OutputDebugStringA("FBXLoader::LoadBones - Starting bone detection\n");
    
    // すべてのメッシュからスキンクラスターを検査
    int nodeCount = scene->GetNodeCount();
    OutputDebugStringA(("FBXLoader::LoadBones - Total nodes in scene: " + std::to_string(nodeCount) + "\n").c_str());
    
    for (int i = 0; i < nodeCount; i++) {
        FbxNode* node = scene->GetNode(i);
        FbxNodeAttribute* attr = node->GetNodeAttribute();
        
        if (attr && attr->GetAttributeType() == FbxNodeAttribute::eMesh) {
            FbxMesh* mesh = node->GetMesh();
            OutputDebugStringA(("FBXLoader::LoadBones - Found mesh node: " + std::string(node->GetName()) + "\n").c_str());
            
            int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
            OutputDebugStringA(("FBXLoader::LoadBones - Skin count for mesh: " + std::to_string(skinCount) + "\n").c_str());
            
            for (int s = 0; s < skinCount; s++) {
                FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(s, FbxDeformer::eSkin));
                int clusterCount = skin->GetClusterCount();
                OutputDebugStringA(("FBXLoader::LoadBones - Cluster count for skin " + std::to_string(s) + ": " + std::to_string(clusterCount) + "\n").c_str());
                
                for (int c = 0; c < clusterCount; c++) {
                    FbxCluster* cluster = skin->GetCluster(c);
                    FbxNode* boneNode = cluster->GetLink();
                    
                    if (boneNode) {
                        std::string boneName = boneNode->GetName();
                        OutputDebugStringA(("FBXLoader::LoadBones - Found linked bone: " + boneName + "\n").c_str());
                        
                        if (boneNodes.find(boneName) == boneNodes.end()) {
                            boneNodes[boneName] = boneNode;
                            orderedBoneNodes.push_back(boneNode);
                            OutputDebugStringA(("FBXLoader::LoadBones - Added bone from skin cluster: " + boneName + "\n").c_str());
                        }
                    } else {
                        OutputDebugStringA("FBXLoader::LoadBones - Cluster has no linked bone\n");
                    }
                }
            }
        }
    }
    
    // ボーンノードが見つからない場合は、スケルトンノードを探す
    if (boneNodes.empty()) {
        OutputDebugStringA("FBXLoader: No bones found in skin clusters, searching for skeleton nodes\n");
        FbxNode* rootNode = scene->GetRootNode();
        if (rootNode) {
            // 再帰的にスケルトンノードを探す
            std::function<void(FbxNode*, int)> findSkeletons = [&](FbxNode* node, int depth) {
                if (node->GetNodeAttribute() && 
                    node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
                    if (boneNodes.find(node->GetName()) == boneNodes.end()) {
                        boneNodes[node->GetName()] = node;
                        orderedBoneNodes.push_back(node);
                        OutputDebugStringA(("FBXLoader: Found skeleton node: " + std::string(node->GetName()) + " at depth " + std::to_string(depth) + "\n").c_str());
                    }
                }
                
                for (int i = 0; i < node->GetChildCount(); i++) {
                    findSkeletons(node->GetChild(i), depth + 1);
                }
            };
            
            findSkeletons(rootNode, 0);
        }
    }
    
    // ボーンの階層構造を構築
    if (!orderedBoneNodes.empty()) {
        // 親ボーンも含めて収集（スキンクラスターに含まれない親ノードも追加）
        std::vector<FbxNode*> allBoneNodes;
        std::unordered_map<std::string, bool> processedNodes;
        
        // 各ボーンノードとその親を収集
        for (FbxNode* boneNode : orderedBoneNodes) {
            FbxNode* current = boneNode;
            std::vector<FbxNode*> path;
            
            // ルートまでの経路を収集
            while (current && current != scene->GetRootNode()) {
                if (processedNodes.find(current->GetName()) == processedNodes.end()) {
                    path.push_back(current);
                    processedNodes[current->GetName()] = true;
                }
                current = current->GetParent();
            }
            
            // 親から子の順に追加
            for (auto it = path.rbegin(); it != path.rend(); ++it) {
                allBoneNodes.push_back(*it);
            }
        }
        
        // 重複を削除
        std::vector<FbxNode*> uniqueBoneNodes;
        std::unordered_map<std::string, bool> addedNodes;
        for (FbxNode* node : allBoneNodes) {
            if (addedNodes.find(node->GetName()) == addedNodes.end()) {
                uniqueBoneNodes.push_back(node);
                addedNodes[node->GetName()] = true;
            }
        }
        
        // 各ボーンノードをモデルに追加
        std::unordered_map<std::string, int> boneIndexMap;
        
        for (FbxNode* boneNode : uniqueBoneNodes) {
            FBXModel::Bone bone;
            bone.name = boneNode->GetName();
            bone.offsetMatrix = Matrix4x4::MakeIdentity();  // 後でスキンクラスターから設定される
            
            // ローカル変換を取得（親に対する相対変換）
            FbxAMatrix localTransform = boneNode->EvaluateLocalTransform();
            bone.currentTransform = ConvertFbxMatrixToMatrix4x4(localTransform);
            bone.parentIndex = -1;
            
            // 親ボーンのインデックスを探す
            FbxNode* parentNode = boneNode->GetParent();
            if (parentNode && parentNode != scene->GetRootNode() && 
                boneIndexMap.find(parentNode->GetName()) != boneIndexMap.end()) {
                bone.parentIndex = boneIndexMap[parentNode->GetName()];
            }
            
            int boneIndex = static_cast<int>(model->GetBones().size());
            boneIndexMap[bone.name] = boneIndex;
            model->AddBone(bone);
            
            OutputDebugStringA(("FBXLoader: Added bone " + bone.name + 
                              " with parent index " + std::to_string(bone.parentIndex) + "\n").c_str());
        }
    }
    
    // ボーンが見つからない場合は、ルートボーンを作成
    if (model->GetBones().empty()) {
        FBXModel::Bone rootBone;
        rootBone.name = "Root";
        rootBone.offsetMatrix = Matrix4x4::MakeIdentity();
        rootBone.currentTransform = Matrix4x4::MakeIdentity();
        rootBone.parentIndex = -1;
        model->AddBone(rootBone);
        OutputDebugStringA("FBXLoader: No bones found, created default root bone\n");
    }
    
    OutputDebugStringA(("FBXLoader: Loaded " + std::to_string(model->GetBones().size()) + " bones\n").c_str());
    for (const auto& bone : model->GetBones()) {
        OutputDebugStringA(("  Bone: " + bone.name + " (parent: " + std::to_string(bone.parentIndex) + ")\n").c_str());
    }
}

void FBXLoader::ProcessBoneHierarchy(FbxNode* node, int parentIndex, FBXModel* model) {
    FBXModel::Bone bone;
    bone.name = node->GetName();
    bone.parentIndex = parentIndex;
    bone.offsetMatrix = Matrix4x4::MakeIdentity();
    bone.currentTransform = ConvertFbxMatrixToMatrix4x4(node->EvaluateLocalTransform());
    
    int boneIndex = static_cast<int>(model->GetBones().size());
    model->AddBone(bone);
    
    // 子ボーンを再帰的に処理
    for (int i = 0; i < node->GetChildCount(); i++) {
        FbxNode* child = node->GetChild(i);
        if (child->GetNodeAttribute() && 
            child->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
            ProcessBoneHierarchy(child, boneIndex, model);
        }
    }
}

void FBXLoader::LoadAnimations(FbxScene* scene, FBXModel* model) {
    int animStackCount = scene->GetSrcObjectCount<FbxAnimStack>();
    
    for (int i = 0; i < animStackCount; i++) {
        FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(i);
        LoadAnimationStack(animStack, model);
    }
}

void FBXLoader::LoadAnimationStack(FbxAnimStack* animStack, FBXModel* model) {
    FBXModel::Animation animation;
    animation.name = animStack->GetName();
    
    // タイムスパンを取得
    FbxTimeSpan timeSpan = animStack->GetLocalTimeSpan();
    FbxTime start = timeSpan.GetStart();
    FbxTime end = timeSpan.GetStop();
    
    animation.duration = static_cast<float>(end.GetSecondDouble() - start.GetSecondDouble());
    animation.ticksPerSecond = static_cast<float>(FbxTime::GetFrameRate(FbxTime::eFrames30));
    
    // アニメーションレイヤーを取得
    int layerCount = animStack->GetMemberCount<FbxAnimLayer>();
    if (layerCount > 0) {
        FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(0);
        
        // 各ボーンのアニメーションカーブを読み込む
        for (const auto& bone : model->GetBones()) {
            FbxNode* node = animStack->GetScene()->FindNodeByName(bone.name.c_str());
            if (node) {
                LoadAnimationCurve(node, animLayer, animation, model);
            }
        }
    }
    
    model->AddAnimation(animation.name, animation);
    
    OutputDebugStringA(("FBXLoader: Added animation '" + animation.name + "' with duration " + 
                       std::to_string(animation.duration) + " seconds\n").c_str());
}

void FBXLoader::LoadAnimationCurve(FbxNode* node, FbxAnimLayer* animLayer, 
                                  FBXModel::Animation& animation, FBXModel* model) {
    FBXModel::AnimationChannel channel;
    channel.boneName = node->GetName();
    
    // 変換、回転、スケールのカーブを取得
    FbxAnimCurve* transCurveX = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);
    FbxAnimCurve* transCurveY = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y);
    FbxAnimCurve* transCurveZ = node->LclTranslation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z);
    
    FbxAnimCurve* rotCurveX = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);
    FbxAnimCurve* rotCurveY = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y);
    FbxAnimCurve* rotCurveZ = node->LclRotation.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z);
    
    FbxAnimCurve* scaleCurveX = node->LclScaling.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_X);
    FbxAnimCurve* scaleCurveY = node->LclScaling.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Y);
    FbxAnimCurve* scaleCurveZ = node->LclScaling.GetCurve(animLayer, FBXSDK_CURVENODE_COMPONENT_Z);
    
    // キーフレームの数を取得
    int keyCount = 0;
    if (transCurveX) keyCount = (std::max)(keyCount, transCurveX->KeyGetCount());
    if (transCurveY) keyCount = (std::max)(keyCount, transCurveY->KeyGetCount());
    if (transCurveZ) keyCount = (std::max)(keyCount, transCurveZ->KeyGetCount());
    if (rotCurveX) keyCount = (std::max)(keyCount, rotCurveX->KeyGetCount());
    if (rotCurveY) keyCount = (std::max)(keyCount, rotCurveY->KeyGetCount());
    if (rotCurveZ) keyCount = (std::max)(keyCount, rotCurveZ->KeyGetCount());
    if (scaleCurveX) keyCount = (std::max)(keyCount, scaleCurveX->KeyGetCount());
    if (scaleCurveY) keyCount = (std::max)(keyCount, scaleCurveY->KeyGetCount());
    if (scaleCurveZ) keyCount = (std::max)(keyCount, scaleCurveZ->KeyGetCount());
    
    // 各キーフレームの値を取得
    for (int i = 0; i < keyCount; i++) {
        FBXModel::AnimationKey key;
        
        // 時間を取得
        FbxTime time;
        if (transCurveX && i < transCurveX->KeyGetCount()) {
            time = transCurveX->KeyGetTime(i);
        } else if (rotCurveX && i < rotCurveX->KeyGetCount()) {
            time = rotCurveX->KeyGetTime(i);
        } else if (scaleCurveX && i < scaleCurveX->KeyGetCount()) {
            time = scaleCurveX->KeyGetTime(i);
        }
        
        key.time = static_cast<float>(time.GetSecondDouble());
        
        // 変換を取得
        FbxVector4 trans = node->EvaluateLocalTranslation(time);
        key.position = ConvertFbxVector3(trans);
        
        // 回転を取得（クォータニオンに変換）
        FbxAMatrix matrix = node->EvaluateLocalTransform(time);
        FbxQuaternion quat = matrix.GetQ();
        key.rotation = ConvertFbxQuaternion(quat);
        
        // スケールを取得
        FbxVector4 scale = node->EvaluateLocalScaling(time);
        key.scale = ConvertFbxVector3(scale);
        
        channel.keys.push_back(key);
    }
    
    if (!channel.keys.empty()) {
        animation.channels[channel.boneName] = channel;
    }
}

void FBXLoader::LoadMaterials(FbxScene* scene, FBXModel* model) {
    int materialCount = scene->GetMaterialCount();
    
    for (int i = 0; i < materialCount; i++) {
        FbxSurfaceMaterial* fbxMaterial = scene->GetMaterial(i);
        FBXModel::Material material;
        material.name = fbxMaterial->GetName();
        
        if (fbxMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId)) {
            FbxSurfaceLambert* lambert = static_cast<FbxSurfaceLambert*>(fbxMaterial);
            
            FbxDouble3 diffuse = lambert->Diffuse.Get();
            material.diffuseColor = Vector4(
                static_cast<float>(diffuse[0]),
                static_cast<float>(diffuse[1]),
                static_cast<float>(diffuse[2]),
                1.0f
            );
            
            FbxDouble3 ambient = lambert->Ambient.Get();
            material.ambientColor = Vector4(
                static_cast<float>(ambient[0]),
                static_cast<float>(ambient[1]),
                static_cast<float>(ambient[2]),
                1.0f
            );
            
            // テクスチャ
            FbxProperty diffuseProperty = lambert->FindProperty(FbxSurfaceMaterial::sDiffuse);
            if (diffuseProperty.IsValid()) {
                int textureCount = diffuseProperty.GetSrcObjectCount<FbxTexture>();
                if (textureCount > 0) {
                    FbxFileTexture* texture = diffuseProperty.GetSrcObject<FbxFileTexture>(0);
                    if (texture) {
                        material.diffuseTexture = texture->GetRelativeFileName();
                    }
                }
            }
        }
        
        if (fbxMaterial->GetClassId().Is(FbxSurfacePhong::ClassId)) {
            FbxSurfacePhong* phong = static_cast<FbxSurfacePhong*>(fbxMaterial);
            
            FbxDouble3 specular = phong->Specular.Get();
            material.specularColor = Vector4(
                static_cast<float>(specular[0]),
                static_cast<float>(specular[1]),
                static_cast<float>(specular[2]),
                1.0f
            );
            
            material.shininess = static_cast<float>(phong->Shininess.Get());
        }
        
        model->AddMaterial(material);
    }
}

fbxsdk::FbxAMatrix FBXLoader::GetGeometryTransformation(fbxsdk::FbxNode* node) {
    const FbxVector4 lT = node->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 lR = node->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 lS = node->GetGeometricScaling(FbxNode::eSourcePivot);
    
    return FbxAMatrix(lT, lR, lS);
}

Matrix4x4 FBXLoader::ConvertFbxMatrixToMatrix4x4(const fbxsdk::FbxAMatrix& fbxMatrix) {
    Matrix4x4 result;
    
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            result.m[row][col] = static_cast<float>(fbxMatrix.Get(row, col));
        }
    }
    
    return result;
}

Vector3 FBXLoader::ConvertFbxVector3(const fbxsdk::FbxVector4& fbxVector) {
    return Vector3(
        static_cast<float>(fbxVector[0]),
        static_cast<float>(fbxVector[1]),
        static_cast<float>(fbxVector[2])
    );
}

Vector4 FBXLoader::ConvertFbxQuaternion(const fbxsdk::FbxQuaternion& fbxQuat) {
    return Vector4(
        static_cast<float>(fbxQuat[0]),
        static_cast<float>(fbxQuat[1]),
        static_cast<float>(fbxQuat[2]),
        static_cast<float>(fbxQuat[3])
    );
}