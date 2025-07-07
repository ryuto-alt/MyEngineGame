#include "AnimatedModel.h"
#include "Mymath.h"
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
}

void AnimatedModel::LoadFromFile(const std::string& directoryPath, const std::string& filename) {
    OutputDebugStringA(("AnimatedModel: Loading " + directoryPath + "/" + filename + " with Assimp\n").c_str());
    LoadWithAssimp(directoryPath, filename);
}

void AnimatedModel::LoadWithAssimp(const std::string& directoryPath, const std::string& filename) {
    
    std::string fullPath = directoryPath + "/" + filename;
    
    const aiScene* scene = assimpImporter_.ReadFile(fullPath,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenSmoothNormals |
        aiProcess_ValidateDataStructure
    );
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        OutputDebugStringA(("AnimatedModel: Error loading file: " + std::string(assimpImporter_.GetErrorString()) + "\n").c_str());
        return;
    }
    
    OutputDebugStringA(("AnimatedModel: Successfully loaded file with " + 
                       std::to_string(scene->mNumMeshes) + " meshes, " +
                       std::to_string(scene->mNumMaterials) + " materials, " +
                       std::to_string(scene->mNumAnimations) + " animations\n").c_str());
    
    // シーンの処理
    ProcessAssimpScene(scene, directoryPath);
    
    // 頂点バッファを作成
    CreateVertexBuffer();
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

// assimpシーンからモデルデータを作成
void AnimatedModel::ProcessAssimpScene(const aiScene* scene, const std::string& directoryPath) {
    if (!scene) {
        OutputDebugStringA("AnimatedModel: Invalid scene data\n");
        return;
    }
    
    ModelData& modelData = GetModelDataInternal();
    
    if (scene->mNumMeshes > 0 && scene->mMeshes[0]) {
        ProcessAssimpMesh(scene->mMeshes[0], scene);
    } else {
        OutputDebugStringA("AnimatedModel: No valid meshes found\n");
    }
    
    if (scene->mNumMaterials > 0 && scene->mMaterials[0]) {
        ProcessAssimpMaterial(scene->mMaterials[0], directoryPath);
    } else {
        ProcessAssimpMaterial(nullptr, directoryPath);
    }
    
    if (scene->mRootNode) {
        modelData.rootNode.name = scene->mRootNode->mName.C_Str();
        if (modelData.rootNode.name.empty()) {
            modelData.rootNode.name = "RootNode";
        }
        rootNodeName_ = modelData.rootNode.name;
        
        aiMatrix4x4 assimpMatrix = scene->mRootNode->mTransformation;
        
        Matrix4x4 transformMatrix;
        transformMatrix.m[0][0] = assimpMatrix.a1; transformMatrix.m[0][1] = assimpMatrix.a2; transformMatrix.m[0][2] = assimpMatrix.a3; transformMatrix.m[0][3] = assimpMatrix.a4;
        transformMatrix.m[1][0] = assimpMatrix.b1; transformMatrix.m[1][1] = assimpMatrix.b2; transformMatrix.m[1][2] = assimpMatrix.b3; transformMatrix.m[1][3] = assimpMatrix.b4;
        transformMatrix.m[2][0] = assimpMatrix.c1; transformMatrix.m[2][1] = assimpMatrix.c2; transformMatrix.m[2][2] = assimpMatrix.c3; transformMatrix.m[2][3] = assimpMatrix.c4;
        transformMatrix.m[3][0] = assimpMatrix.d1; transformMatrix.m[3][1] = assimpMatrix.d2; transformMatrix.m[3][2] = assimpMatrix.d3; transformMatrix.m[3][3] = assimpMatrix.d4;
        
        Matrix4x4 coordinateConversion = MakeIdentity4x4();
        coordinateConversion.m[2][2] = -1.0f;
        
        modelData.rootNode.localMatrix = Multiply(coordinateConversion, transformMatrix);
        
        OutputDebugStringA(("AnimatedModel: Root node: " + rootNodeName_ + "\n").c_str());
    } else {
        modelData.rootNode.name = "DefaultRoot";
        modelData.rootNode.localMatrix = MakeIdentity4x4();
        rootNodeName_ = "DefaultRoot";
        OutputDebugStringA("AnimatedModel: Using default root node\n");
    }
    
    ProcessAssimpAnimation(scene);
}

// assimpメッシュからジオメトリデータを作成
void AnimatedModel::ProcessAssimpMesh(const aiMesh* mesh, const aiScene* scene) {
    if (!mesh || mesh->mNumVertices == 0) {
        OutputDebugStringA("AnimatedModel: Invalid mesh data\n");
        return;
    }
    
    ModelData& modelData = GetModelDataInternal();
    modelData.vertices.clear();
    modelData.vertices.reserve(mesh->mNumFaces * 3);
    
    OutputDebugStringA(("AnimatedModel: Processing mesh with " + std::to_string(mesh->mNumVertices) + " vertices and " + std::to_string(mesh->mNumFaces) + " faces\n").c_str());
    
    for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++) {
        const aiFace& face = mesh->mFaces[faceIndex];
        
        if (face.mNumIndices != 3) {
            continue;
        }
        
        unsigned int indices[3] = { face.mIndices[0], face.mIndices[2], face.mIndices[1] };
        
        for (int i = 0; i < 3; i++) {
            unsigned int vertexIndex = indices[i];
            
            if (vertexIndex >= mesh->mNumVertices) {
                OutputDebugStringA(("AnimatedModel: Invalid vertex index: " + std::to_string(vertexIndex) + "\n").c_str());
                continue;
            }
            
            VertexData vertex{};
            
            vertex.position = {
                mesh->mVertices[vertexIndex].x,
                mesh->mVertices[vertexIndex].y,
                -mesh->mVertices[vertexIndex].z,
                1.0f
            };
            
            if (mesh->HasNormals()) {
                vertex.normal = {
                    mesh->mNormals[vertexIndex].x,
                    mesh->mNormals[vertexIndex].y,
                    -mesh->mNormals[vertexIndex].z
                };
            } else {
                vertex.normal = {0.0f, 1.0f, 0.0f};
            }
            
            if (mesh->mTextureCoords[0] && vertexIndex < mesh->mNumVertices) {
                vertex.texcoord = {
                    mesh->mTextureCoords[0][vertexIndex].x,
                    mesh->mTextureCoords[0][vertexIndex].y
                };
            } else {
                vertex.texcoord = {0.0f, 0.0f};
            }
            
            modelData.vertices.push_back(vertex);
        }
    }
    
    OutputDebugStringA(("AnimatedModel: Created " + std::to_string(modelData.vertices.size()) + " vertices\n").c_str());
}

// assimpマテリアルからマテリアルデータを作成
void AnimatedModel::ProcessAssimpMaterial(const aiMaterial* material, const std::string& directoryPath) {
    ModelData& modelData = GetModelDataInternal();
    
    modelData.material.diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
    modelData.material.ambient = {0.2f, 0.2f, 0.2f, 1.0f};
    modelData.material.specular = {0.5f, 0.5f, 0.5f, 1.0f};
    modelData.material.alpha = 1.0f;
    modelData.material.textureFilePath = "Resources/uvChecker.png";
    
    if (!material) {
        OutputDebugStringA("AnimatedModel: No material data, using defaults\n");
        return;
    }
    
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString texturePath;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
            std::string textureFileName = texturePath.C_Str();
            if (!textureFileName.empty()) {
                modelData.material.textureFilePath = directoryPath + "/" + textureFileName;
                OutputDebugStringA(("AnimatedModel: Found texture: " + modelData.material.textureFilePath + "\n").c_str());
            }
        }
    }
    
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
    if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS && opacity > 0.0f) {
        modelData.material.alpha = opacity;
    }
}

// assimpノードからアニメーションデータを作成
void AnimatedModel::ProcessAssimpAnimation(const aiScene* scene) {
    animation_.duration = 1.0f;
    animation_.nodeAnimations.clear();
    
    if (!scene || scene->mNumAnimations == 0) {
        OutputDebugStringA("AnimatedModel: No animations found, using default\n");
        animationPlayer_.SetAnimation(animation_);
        animationPlayer_.SetLoop(true);
        return;
    }
    
    const aiAnimation* assimpAnimation = scene->mAnimations[0];
    if (!assimpAnimation || assimpAnimation->mTicksPerSecond <= 0.0) {
        OutputDebugStringA("AnimatedModel: Invalid animation data\n");
        animationPlayer_.SetAnimation(animation_);
        animationPlayer_.SetLoop(true);
        return;
    }
    
    OutputDebugStringA(("AnimatedModel: Processing animation with " + std::to_string(assimpAnimation->mNumChannels) + " channels\n").c_str());
    
    double ticksPerSecond = assimpAnimation->mTicksPerSecond > 0.0 ? assimpAnimation->mTicksPerSecond : 25.0;
    animation_.duration = static_cast<float>(assimpAnimation->mDuration / ticksPerSecond);
    
    for (unsigned int i = 0; i < assimpAnimation->mNumChannels; i++) {
        const aiNodeAnim* nodeAnim = assimpAnimation->mChannels[i];
        if (!nodeAnim) continue;
        
        std::string nodeName = nodeAnim->mNodeName.C_Str();
        if (nodeName.empty()) continue;
        
        NodeAnimation& nodeAnimation = animation_.nodeAnimations[nodeName];
        
        for (unsigned int j = 0; j < nodeAnim->mNumPositionKeys; j++) {
            const aiVectorKey& key = nodeAnim->mPositionKeys[j];
            KeyframeVector3 keyframe;
            keyframe.time = static_cast<float>(key.mTime / ticksPerSecond);
            keyframe.value = {key.mValue.x, key.mValue.y, -key.mValue.z};
            nodeAnimation.translate.push_back(keyframe);
        }
        
        for (unsigned int j = 0; j < nodeAnim->mNumRotationKeys; j++) {
            const aiQuatKey& key = nodeAnim->mRotationKeys[j];
            KeyframeQuaternion keyframe;
            keyframe.time = static_cast<float>(key.mTime / ticksPerSecond);
            keyframe.value = {-key.mValue.x, -key.mValue.y, -key.mValue.z, key.mValue.w};
            nodeAnimation.rotate.push_back(keyframe);
        }
        
        for (unsigned int j = 0; j < nodeAnim->mNumScalingKeys; j++) {
            const aiVectorKey& key = nodeAnim->mScalingKeys[j];
            KeyframeVector3 keyframe;
            keyframe.time = static_cast<float>(key.mTime / ticksPerSecond);
            keyframe.value = {key.mValue.x, key.mValue.y, key.mValue.z};
            nodeAnimation.scale.push_back(keyframe);
        }
        
        OutputDebugStringA(("AnimatedModel: Node " + nodeName + " - Keys: " + 
                          std::to_string(nodeAnimation.translate.size()) + "/" +
                          std::to_string(nodeAnimation.rotate.size()) + "/" +
                          std::to_string(nodeAnimation.scale.size()) + "\n").c_str());
    }
    
    animationPlayer_.SetAnimation(animation_);
    animationPlayer_.SetLoop(true);
    
    OutputDebugStringA(("AnimatedModel: Animation setup complete, duration: " + std::to_string(animation_.duration) + "s\n").c_str());
}