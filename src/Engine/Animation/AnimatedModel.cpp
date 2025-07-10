#include "AnimatedModel.h"
#include "Mymath.h"
#include "UnoEngine.h"
#include <algorithm>
#include <cctype>

// コンストラクタ
AnimatedModel::AnimatedModel() : rootNodeName_("root") {
}

// デストラクタ
AnimatedModel::~AnimatedModel() {
}

// 初期化
void AnimatedModel::Initialize(DirectXCommon* dxCommon) {
    Model::Initialize(dxCommon);
    dxCommon_ = dxCommon;
}

// モデルとアニメーションの読み込み
void AnimatedModel::LoadFromFile(const std::string& directoryPath, const std::string& filename) {
    OutputDebugStringA(("AnimatedModel: Loading from " + directoryPath + "/" + filename + "\n").c_str());
    
    // ファイル拡張子をチェック
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == "gltf") {
        // GLTFファイルの場合（assimpを使用）
        LoadFromGLTFWithAssimp(directoryPath, filename);
    } else {
        // OBJファイルの場合
        LoadFromObj(directoryPath, filename);
        
        // マテリアル情報のデバッグ出力
        const ModelData& modelData = GetModelData();
        OutputDebugStringA(("AnimatedModel: Texture path: " + modelData.material.textureFilePath + "\n").c_str());
        
        // マテリアル情報が空の場合、デフォルト値を設定
        ModelData& modelDataInternal2 = GetModelDataInternal();
        if (modelDataInternal2.material.textureFilePath.empty()) {
            OutputDebugStringA("AnimatedModel: No texture path found, setting default values\n");
            // デフォルトのマテリアル設定
            modelDataInternal2.material.textureFilePath = "Resources/uvChecker.png";
            modelDataInternal2.material.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
            OutputDebugStringA(("AnimatedModel: Set default texture: " + modelDataInternal2.material.textureFilePath + "\n").c_str());
        }
        
        // ルートノード名を設定
        rootNodeName_ = "root";
    }
    
    OutputDebugStringA(("AnimatedModel: Root node name set to: " + rootNodeName_ + "\n").c_str());
    
    // OBJファイルの場合のみダミーアニメーションを読み込み
    if (extension != "gltf") {
        LoadAnimation(directoryPath, filename);
    }
}

// assimpを使用したGLTFファイルからの読み込み
void AnimatedModel::LoadFromGLTFWithAssimp(const std::string& directoryPath, const std::string& filename) {
    OutputDebugStringA(("AnimatedModel: Loading GLTF with Assimp from " + directoryPath + "/" + filename + "\n").c_str());
    
    std::string fullPath = directoryPath + "/" + filename;
    
    // assimpでGLTFファイルを読み込み
    const aiScene* scene = assimpImporter_.ReadFile(fullPath,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenSmoothNormals
    );
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        OutputDebugStringA(("AnimatedModel: Error loading GLTF file: " + std::string(assimpImporter_.GetErrorString()) + "\n").c_str());
        return;
    }
    
    OutputDebugStringA(("AnimatedModel: Successfully loaded GLTF file\n"));
    
    // シーンの処理
    ProcessAssimpScene(scene, directoryPath);
    
    // スキニング処理を初期化
    Node rootNode = ReadNode(scene->mRootNode);
    skeleton_ = CreateSkeleton(rootNode);
    skinCluster_ = CreateSkinCluster();
    
    // 頂点バッファを作成
    CreateVertexBuffer();
}

// アニメーションの読み込み
void AnimatedModel::LoadAnimation(const std::string& directoryPath, const std::string& filename) {
    // LoadAnimationFile関数を使用してアニメーションを読み込み
    animation_ = LoadAnimationFile(directoryPath, filename);
    
    // アニメーションプレイヤーに設定
    animationPlayer_.SetAnimation(animation_);
    animationPlayer_.SetLoop(true);
}

// 更新（アニメーション時刻を進める）
void AnimatedModel::Update(float deltaTime) {
    animationPlayer_.Update(deltaTime);
}

// アニメーションのローカル変換行列を取得
Matrix4x4 AnimatedModel::GetAnimationLocalMatrix() {
    // GLTFファイルの場合はAnimatedCubeノード、それ以外はrootノードを使用
    return animationPlayer_.GetLocalMatrix(rootNodeName_);
}

// アニメーション再生制御
void AnimatedModel::PlayAnimation() {
    animationPlayer_.Play();
}

void AnimatedModel::StopAnimation() {
    animationPlayer_.Stop();
}

void AnimatedModel::PauseAnimation() {
    animationPlayer_.Pause();
}

void AnimatedModel::SetAnimationLoop(bool loop) {
    animationPlayer_.SetLoop(loop);
}


Node AnimatedModel::ReadNode(aiNode* node)
{
    Node result;
    aiVector3D scale, translate;
    aiQuaternion rotation;

    node->mTransformation.Decompose(scale, rotation, translate);
    result.transform.scale = { scale.x, scale.y, scale.z };
    result.transform.rotate = { rotation.x, -rotation.y, -rotation.z, rotation.w };  // Y,Z成分を反転（SoraEngine-Skinning方式）
    result.transform.translate = { -translate.x, translate.y, translate.z };  // X座標を反転（SoraEngine-Skinning方式）
    result.localMatrix = MakeAffineMatrix(result.transform.scale, result.transform.rotate, result.transform.translate);
    result.name = node->mName.C_Str();
    result.children.resize(node->mNumChildren);
    for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
        result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
    }
    return result;
}

Skeleton AnimatedModel::CreateSkeleton(const Node& rootNode)
{
    Skeleton skeleton;
    skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);
    for (const Joint& joint : skeleton.joints) {
        skeleton.jointMap.emplace(joint.name, joint.index);
    }
    return skeleton;
}

int32_t AnimatedModel::CreateJoint(const Node& node, std::optional<int32_t> parent, std::vector<Joint>& joints)
{
    Joint joint;
    joint.name = node.name;
    joint.localMatrix = node.localMatrix;
    joint.skeletonSpaceMatrix = MakeIdentity4x4();
    joint.transform.scale = node.transform.scale;
    joint.transform.rotate = node.transform.rotate;
    joint.transform.translate = node.transform.translate;
    joint.index = int32_t(joints.size());
    joint.parent = parent;

    joints.push_back(joint);

    for (const Node& child : node.children) {
        int32_t childIndex = CreateJoint(child, joint.index, joints);
        joints[joint.index].children.push_back(childIndex);
    }

    return joint.index;
}

SkinCluster AnimatedModel::CreateSkinCluster()
{
    SkinCluster skinCluster;
    
    // palette用のリソースを作成
    skinCluster.paletteResource = dxCommon_->CreateBufferResource(sizeof(WellForGPU) * skeleton_.joints.size());
    WellForGPU* mappedPalette = nullptr;
    skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
    skinCluster.mappedPalette = { mappedPalette, skeleton_.joints.size() };
    
    // SRVを作成
    // UnoEngineのSrvManagerを使用
    UnoEngine* engine = UnoEngine::GetInstance();
    if (engine && engine->GetSrvManager()) {
        SrvManager* srvManager = engine->GetSrvManager();
        uint32_t srvIndex = srvManager->Allocate();
        srvManager->CreateSRVForStructuredBuffer(srvIndex, skinCluster.paletteResource, 
                                                  static_cast<UINT>(skeleton_.joints.size()), sizeof(WellForGPU));
        skinCluster.paletteSrvHandle.first = srvManager->GetCPUDescriptorHandle(srvIndex);
        skinCluster.paletteSrvHandle.second = srvManager->GetGPUDescriptorHandle(srvIndex);
        
        OutputDebugStringA(("AnimatedModel: Created palette SRV at index " + std::to_string(srvIndex) + "\n").c_str());
    } else {
        OutputDebugStringA("AnimatedModel: WARNING - Could not access SrvManager\n");
        skinCluster.paletteSrvHandle.first = {};
        skinCluster.paletteSrvHandle.second = {};
    }
    
    // Influence用のリソースを作成
    const ModelData& modelData = GetModelData();
    skinCluster.influenceResource = dxCommon_->CreateBufferResource(sizeof(VertexInfluence) * modelData.vertices.size());
    VertexInfluence* mappedInfluence = nullptr;
    skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
    std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData.vertices.size());
    skinCluster.mappedInfluence = { mappedInfluence, modelData.vertices.size() };
    
    // Influence用のVBVを作成
    skinCluster.influenceBufferView.BufferLocation = skinCluster.influenceResource->GetGPUVirtualAddress();
    skinCluster.influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * modelData.vertices.size());
    skinCluster.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);
    
    // InverseBindPoseMatrixを格納する場所を作成して、単位行列で埋める
    skinCluster.inverseBindPoseMatrices.resize(skeleton_.joints.size());
    std::generate(skinCluster.inverseBindPoseMatrices.begin(), skinCluster.inverseBindPoseMatrices.end(), [] { return MakeIdentity4x4(); });
    
    // ボーンウェイト情報をインフルエンスデータに設定
    for (const auto& jointWeight : modelData.skinClusterData) {
        auto it = skeleton_.jointMap.find(jointWeight.first);
        if (it == skeleton_.jointMap.end()) {
            continue; // このジョイントがスケルトンに存在しない場合はスキップ
        }
        
        // InverseBindPoseMatrixを設定
        skinCluster.inverseBindPoseMatrices[it->second] = jointWeight.second.inverseBindPoseMatrix;
        
        // 各頂点のウェイト情報を設定
        for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
            if (vertexWeight.vectorIndex >= modelData.vertices.size()) {
                continue; // 無効なインデックスはスキップ
            }
            
            auto& currentInfluence = skinCluster.mappedInfluence[vertexWeight.vectorIndex];
            
            // 空いているスロットにウェイトとジョイントインデックスを設定
            for (uint32_t index = 0; index < kNumMaxInfluence; ++index) {
                if (currentInfluence.weights[index] == 0.0f) {
                    currentInfluence.weights[index] = vertexWeight.weight;
                    currentInfluence.jointIndices[index] = it->second;
                    break;
                }
            }
        }
    }
    
    // ウェイトの正規化（合計が1になるように）
    for (size_t i = 0; i < modelData.vertices.size(); ++i) {
        auto& influence = skinCluster.mappedInfluence[i];
        float totalWeight = 0.0f;
        
        // 合計ウェイトを計算
        for (uint32_t j = 0; j < kNumMaxInfluence; ++j) {
            totalWeight += influence.weights[j];
        }
        
        // 正規化
        if (totalWeight > 0.0f) {
            for (uint32_t j = 0; j < kNumMaxInfluence; ++j) {
                influence.weights[j] /= totalWeight;
            }
        } else {
            // ウェイトが設定されていない頂点はルートジョイントに100%バインド
            influence.weights[0] = 1.0f;
            influence.jointIndices[0] = 0;
        }
    }
    
    OutputDebugStringA(("AnimatedModel: Created SkinCluster with " + std::to_string(skeleton_.joints.size()) + " joints\n").c_str());
    
    return skinCluster;
}

// assimpシーンからモデルデータを作成
void AnimatedModel::ProcessAssimpScene(const aiScene* scene, const std::string& directoryPath) {
    ModelData& modelData = GetModelDataInternal();
    
    // メッシュの処理（最初のメッシュのみ処理）
    if (scene->mNumMeshes > 0) {
        ProcessAssimpMesh(scene->mMeshes[0], scene);
    }
    
    // マテリアルの処理
    if (scene->mNumMaterials > 0) {
        ProcessAssimpMaterial(scene->mMaterials[0], directoryPath);
    }
    
    // ルートノードの設定
    if (scene->mRootNode) {
        rootNodeName_ = scene->mRootNode->mName.C_Str();
        OutputDebugStringA(("AnimatedModel: Root node name: " + rootNodeName_ + "\n").c_str());
    }
    
    // アニメーションの処理
    ProcessAssimpAnimation(scene);
}

// assimpメッシュからジオメトリデータを作成
void AnimatedModel::ProcessAssimpMesh(const aiMesh* mesh, const aiScene* scene) {
    ModelData& modelData = GetModelDataInternal();
    modelData.vertices.clear();
    
    OutputDebugStringA(("AnimatedModel: Processing mesh with " + std::to_string(mesh->mNumVertices) + " vertices and " + std::to_string(mesh->mNumFaces) + " faces\n").c_str());
    
    // まず頂点データを頂点インデックス順に格納（ボーンウェイトの参照用）
    std::vector<VertexData> indexedVertices(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        VertexData& vertex = indexedVertices[i];
        
        // 位置（右手座標系→左手座標系：X座標を反転）
        vertex.position = {
            -mesh->mVertices[i].x,  // X座標を反転（SoraEngine-Skinning方式）
            mesh->mVertices[i].y,
            mesh->mVertices[i].z,
            1.0f
        };
        
        // 法線（右手座標系→左手座標系：X成分を反転）
        if (mesh->HasNormals()) {
            vertex.normal = {
                -mesh->mNormals[i].x,  // X成分を反転（SoraEngine-Skinning方式）
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            };
        } else {
            vertex.normal = {0.0f, 1.0f, 0.0f};
        }
        
        // テクスチャ座標（最初のセットのみ）
        if (mesh->mTextureCoords[0]) {
            vertex.texcoord = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };
        } else {
            vertex.texcoord = {0.0f, 0.0f};
        }
    }
    
    // インデックスを使用して三角形ごとに頂点を作成（DirectX用に座標変換も適用）
    for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++) {
        const aiFace& face = mesh->mFaces[faceIndex];
        
        if (face.mNumIndices != 3) {
            OutputDebugStringA(("AnimatedModel: Warning - Face " + std::to_string(faceIndex) + " has " + std::to_string(face.mNumIndices) + " indices (expected 3)\n").c_str());
            continue;
        }
        
        // X軸反転によりワインディングオーダーはそのまま（0, 1, 2の順序）
        unsigned int indices[3] = { face.mIndices[0], face.mIndices[1], face.mIndices[2] };
        
        for (int i = 0; i < 3; i++) {
            unsigned int vertexIndex = indices[i];
            modelData.vertices.push_back(indexedVertices[vertexIndex]);
        }
    }
    
    // ボーン情報の処理
    OutputDebugStringA(("AnimatedModel: Processing " + std::to_string(mesh->mNumBones) + " bones\n").c_str());
    
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {
        aiBone* bone = mesh->mBones[boneIndex];
        std::string jointName = bone->mName.C_Str();
        JointWeightData& jointWeightData = modelData.skinClusterData[jointName];
        
        OutputDebugStringA(("AnimatedModel: Processing bone: " + jointName + " with " + std::to_string(bone->mNumWeights) + " weights\n").c_str());
        
        // InverseBindPose行列を取得（assimpのOffsetMatrixがInverseBindPose）
        aiMatrix4x4 offsetMatrix = bone->mOffsetMatrix;
        
        // SoraEngine-Skinning方式の座標系変換を使用
        // InverseBindPose行列を分解して、各成分を変換してから再構築
        aiMatrix4x4 bindPoseMatrix = offsetMatrix.Inverse();
        aiVector3D scale, translate;
        aiQuaternion rotation;
        bindPoseMatrix.Decompose(scale, rotation, translate);
        
        // 右手座標系から左手座標系への変換（SoraEngine-Skinning方式）
        Matrix4x4 bindPoseMatrixConverted = MakeAffineMatrix(
            { scale.x, scale.y, scale.z },                              // スケール
            { rotation.x, -rotation.y, -rotation.z, rotation.w },       // 回転（Y,Z成分を反転）
            { -translate.x, translate.y, translate.z }                  // 位置（X座標を反転）
        );
        
        // 逆バインドポーズ行列を格納
        jointWeightData.inverseBindPoseMatrix = Inverse(bindPoseMatrixConverted);
        
        // 頂点ウェイト情報を格納
        for (unsigned int weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++) {
            const aiVertexWeight& weight = bone->mWeights[weightIndex];
            
            // 元の頂点インデックスから実際の頂点配列でのインデックスを探す
            // 各面で頂点が複製されているため、元のインデックスを持つすべての頂点を探す
            for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++) {
                const aiFace& face = mesh->mFaces[faceIndex];
                unsigned int indices[3] = { face.mIndices[0], face.mIndices[2], face.mIndices[1] };
                
                for (int i = 0; i < 3; i++) {
                    if (indices[i] == weight.mVertexId) {
                        // この頂点は modelData.vertices の (faceIndex * 3 + i) 番目に格納されている
                        VertexWeightData vwd;
                        vwd.weight = weight.mWeight;
                        vwd.vectorIndex = faceIndex * 3 + i;
                        jointWeightData.vertexWeights.push_back(vwd);
                    }
                }
            }
        }
    }
    
    OutputDebugStringA(("AnimatedModel: Created " + std::to_string(modelData.vertices.size()) + " vertices from faces\n").c_str());
}

// assimpマテリアルからマテリアルデータを作成
void AnimatedModel::ProcessAssimpMaterial(const aiMaterial* material, const std::string& directoryPath) {
    ModelData& modelData = GetModelDataInternal();
    
    // デフォルト値を設定
    modelData.material.diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
    modelData.material.ambient = {0.2f, 0.2f, 0.2f, 1.0f};
    modelData.material.specular = {0.5f, 0.5f, 0.5f, 1.0f};
    modelData.material.alpha = 1.0f;
    
    // ディフューズテクスチャを取得
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString texturePath;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
            std::string textureFileName = texturePath.C_Str();
            modelData.material.textureFilePath = directoryPath + "/" + textureFileName;
            OutputDebugStringA(("AnimatedModel: Found texture: " + modelData.material.textureFilePath + "\n").c_str());
        }
    } else {
        // デフォルトテクスチャを設定
        modelData.material.textureFilePath = directoryPath + "/" + "AnimatedCube_BaseColor.png";
        OutputDebugStringA(("AnimatedModel: Using default texture: " + modelData.material.textureFilePath + "\n").c_str());
    }
    
    // マテリアルプロパティの取得
    aiColor3D color;
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        modelData.material.diffuse = {color.r, color.g, color.b, 1.0f};
    }
    
    if (material->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
        modelData.material.ambient = {color.r, color.g, color.b, 1.0f};
    }
    
    if (material->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
        modelData.material.specular = {color.r, color.g, color.b, 1.0f};
    }
    
    float opacity;
    if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
        modelData.material.alpha = opacity;
    }
}

// assimpノードからアニメーションデータを作成
void AnimatedModel::ProcessAssimpAnimation(const aiScene* scene) {
    if (scene->mNumAnimations == 0) {
        OutputDebugStringA("AnimatedModel: No animations found in scene\n");
        return;
    }
    
    const aiAnimation* assimpAnimation = scene->mAnimations[0];
    
    OutputDebugStringA(("AnimatedModel: Processing animation with " + std::to_string(assimpAnimation->mNumChannels) + " channels\n").c_str());
    
    // アニメーション時間を設定
    animation_.duration = static_cast<float>(assimpAnimation->mDuration / assimpAnimation->mTicksPerSecond);
    animation_.nodeAnimations.clear();
    
    // 各ノードアニメーションチャンネルを処理
    for (unsigned int i = 0; i < assimpAnimation->mNumChannels; i++) {
        const aiNodeAnim* nodeAnim = assimpAnimation->mChannels[i];
        std::string nodeName = nodeAnim->mNodeName.C_Str();
        
        NodeAnimation& nodeAnimation = animation_.nodeAnimations[nodeName];
        
        // 位置キーフレーム（右手座標系→左手座標系：X座標を反転）
        for (unsigned int j = 0; j < nodeAnim->mNumPositionKeys; j++) {
            const aiVectorKey& key = nodeAnim->mPositionKeys[j];
            KeyframeVector3 keyframe;
            keyframe.time = static_cast<float>(key.mTime / assimpAnimation->mTicksPerSecond);
            keyframe.value = {-key.mValue.x, key.mValue.y, key.mValue.z};  // X座標を反転（SoraEngine-Skinning方式）
            nodeAnimation.translate.push_back(keyframe);
        }
        
        // 回転キーフレーム（右手座標系→左手座標系）
        for (unsigned int j = 0; j < nodeAnim->mNumRotationKeys; j++) {
            const aiQuatKey& key = nodeAnim->mRotationKeys[j];
            KeyframeQuaternion keyframe;
            keyframe.time = static_cast<float>(key.mTime / assimpAnimation->mTicksPerSecond);
            
            // 右手座標系から左手座標系への変換（SoraEngine-Skinning方式）
            // Y,Z成分を反転
            keyframe.value = {key.mValue.x, -key.mValue.y, -key.mValue.z, key.mValue.w};
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
        
        OutputDebugStringA(("AnimatedModel: Node " + nodeName + " - Position keys: " + std::to_string(nodeAnimation.translate.size()) + 
                          ", Rotation keys: " + std::to_string(nodeAnimation.rotate.size()) + 
                          ", Scale keys: " + std::to_string(nodeAnimation.scale.size()) + "\n").c_str());
    }
    
    // アニメーションプレイヤーに設定
    animationPlayer_.SetAnimation(animation_);
    animationPlayer_.SetLoop(true);
    
    OutputDebugStringA(("AnimatedModel: Animation duration: " + std::to_string(animation_.duration) + " seconds\n").c_str());
}