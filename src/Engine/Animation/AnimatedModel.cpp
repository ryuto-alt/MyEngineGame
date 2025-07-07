#include "AnimatedModel.h"
#include "Mymath.h"
#include "TextureManager.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

// コンストラクタ
AnimatedModel::AnimatedModel() : rootNodeName_("root"), hasSkeleton_(false) {
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
    
    OutputDebugStringA(("AnimatedModel: Loading file: " + fullPath + "\n").c_str());
    
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
    
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        const aiMaterial* material = scene->mMaterials[i];
        if (material) {
            aiString materialName;
            if (material->Get(AI_MATKEY_NAME, materialName) == AI_SUCCESS) {
                OutputDebugStringA(("AnimatedModel: Material " + std::to_string(i) + " name: " + materialName.C_Str() + "\n").c_str());
            }
            
            OutputDebugStringA(("AnimatedModel: Material " + std::to_string(i) + " has " + 
                               std::to_string(material->GetTextureCount(aiTextureType_DIFFUSE)) + " diffuse textures\n").c_str());
            
            for (unsigned int j = 0; j < material->GetTextureCount(aiTextureType_DIFFUSE); j++) {
                aiString texturePath;
                if (material->GetTexture(aiTextureType_DIFFUSE, j, &texturePath) == AI_SUCCESS) {
                    OutputDebugStringA(("AnimatedModel: Diffuse texture " + std::to_string(j) + ": " + texturePath.C_Str() + "\n").c_str());
                }
            }
        }
    }
    
    // シーンの処理
    ProcessAssimpScene(scene, directoryPath, filename);
    
    // 頂点バッファを作成
    CreateVertexBuffer();
}


// 更新（アニメーション時刻を進める）
void AnimatedModel::Update(float deltaTime) {
    animationPlayer_.Update(deltaTime);
    
    // スケルトンがある場合は更新する
    if (hasSkeleton_) {
        UpdateSkeleton(skeleton_);
    }
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
void AnimatedModel::ProcessAssimpScene(const aiScene* scene, const std::string& directoryPath, const std::string& objFileName) {
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
        ProcessAssimpMaterial(scene->mMaterials[0], directoryPath, objFileName);
    } else {
        ProcessAssimpMaterial(nullptr, directoryPath, objFileName);
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
    
    // スケルトンを作成
    if (scene->mRootNode) {
        OutputDebugStringA("AnimatedModel: Creating skeleton from node hierarchy\n");
        
        // assimpのノード階層をEngineのNode階層に変換
        Node rootNode;
        ProcessAssimpNode(scene->mRootNode, rootNode);
        
        // Skeletonを作成
        skeleton_ = CreateSkeleton(rootNode);
        hasSkeleton_ = true;
        
        OutputDebugStringA(("AnimatedModel: Created skeleton with " + std::to_string(skeleton_.joints.size()) + " joints\n").c_str());
        
        // 各ジョイントの名前をデバッグ出力
        for (const Joint& joint : skeleton_.joints) {
            OutputDebugStringA(("AnimatedModel: Joint " + std::to_string(joint.index) + ": " + joint.name + "\n").c_str());
        }
    }
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
void AnimatedModel::ProcessAssimpMaterial(const aiMaterial* material, const std::string& directoryPath, const std::string& objFileName) {
    ModelData& modelData = GetModelDataInternal();
    
    modelData.material.diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
    modelData.material.ambient = {0.2f, 0.2f, 0.2f, 1.0f};
    modelData.material.specular = {0.5f, 0.5f, 0.5f, 1.0f};
    modelData.material.alpha = 1.0f;
    modelData.material.textureFilePath = "";
    
    if (!material) {
        OutputDebugStringA("AnimatedModel: No material data, using defaults\n");
        modelData.material.textureFilePath = "Resources/uvChecker.png";
        return;
    }
    
    OutputDebugStringA(("AnimatedModel: Processing material with " + std::to_string(material->GetTextureCount(aiTextureType_DIFFUSE)) + " diffuse textures\n").c_str());
    
    aiTextureType textureTypes[] = {aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR, aiTextureType_UNKNOWN};
    std::string textureFileName;
    
    for (const auto& textureType : textureTypes) {
        if (material->GetTextureCount(textureType) > 0) {
            aiString texturePath;
            if (material->GetTexture(textureType, 0, &texturePath) == AI_SUCCESS) {
                textureFileName = texturePath.C_Str();
                if (!textureFileName.empty()) {
                    OutputDebugStringA(("AnimatedModel: Found texture in material: " + textureFileName + "\n").c_str());
                    break;
                }
            }
        }
    }
    
    if (!textureFileName.empty()) {
        std::string originalFileName = textureFileName;
        
        size_t lastSlash = textureFileName.find_last_of("/\\");
        std::string filenameOnly = (lastSlash != std::string::npos) ? textureFileName.substr(lastSlash + 1) : textureFileName;
        
        std::vector<std::string> possiblePaths = {
            directoryPath + "/" + textureFileName,
            directoryPath + "/" + filenameOnly,
            "Resources/" + textureFileName,
            "Resources/" + filenameOnly,
            "Resources/textures/" + filenameOnly,
            "Resources/models/" + filenameOnly,
            "Resources/Models/" + filenameOnly,
            "Resources/CG2/Resources/" + filenameOnly,
            "GitHub/CG2/Resources/" + filenameOnly,
            "CG2/Resources/" + filenameOnly
        };
        
        bool found = false;
        for (const auto& path : possiblePaths) {
            OutputDebugStringA(("AnimatedModel: Checking texture path: " + path + "\n").c_str());
            if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
                modelData.material.textureFilePath = path;
                OutputDebugStringA(("AnimatedModel: Texture found at: " + path + "\n").c_str());
                found = true;
                break;
            }
        }
        
        if (!found) {
            OutputDebugStringA(("AnimatedModel: Texture not found - Original: " + originalFileName + ", Filename: " + filenameOnly + "\n").c_str());
            OutputDebugStringA("AnimatedModel: Checked paths:\n");
            for (const auto& path : possiblePaths) {
                OutputDebugStringA(("  - " + path + "\n").c_str());
            }
            OutputDebugStringA("AnimatedModel: Using default texture\n");
            modelData.material.textureFilePath = "Resources/uvChecker.png";
        }
    } else {
        OutputDebugStringA("AnimatedModel: No texture found in material via assimp, searching manually\n");
        std::string mtlTexture = ParseMTLFile(directoryPath, objFileName);
        
        if (!mtlTexture.empty()) {
            size_t lastSlash = mtlTexture.find_last_of("/\\");
            std::string filenameOnly = (lastSlash != std::string::npos) ? mtlTexture.substr(lastSlash + 1) : mtlTexture;
            
            std::vector<std::string> possiblePaths = {
                directoryPath + "/" + mtlTexture,
                directoryPath + "/" + filenameOnly,
                "Resources/" + filenameOnly,
                "Resources/Models/" + filenameOnly
            };
            
            bool found = false;
            for (const auto& path : possiblePaths) {
                if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
                    modelData.material.textureFilePath = path;
                    OutputDebugStringA(("AnimatedModel: MTL texture found at: " + path + "\n").c_str());
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                OutputDebugStringA("AnimatedModel: MTL texture not found, searching directory for PNG files\n");
                std::string autoTexture = FindTextureInDirectory(directoryPath);
                if (!autoTexture.empty()) {
                    modelData.material.textureFilePath = autoTexture;
                    OutputDebugStringA(("AnimatedModel: Auto-detected texture: " + autoTexture + "\n").c_str());
                } else {
                    OutputDebugStringA("AnimatedModel: No PNG files found, using default\n");
                    modelData.material.textureFilePath = "Resources/uvChecker.png";
                }
            }
        } else {
            OutputDebugStringA("AnimatedModel: No MTL texture info, searching directory for PNG files\n");
            std::string autoTexture = FindTextureInDirectory(directoryPath);
            if (!autoTexture.empty()) {
                modelData.material.textureFilePath = autoTexture;
                OutputDebugStringA(("AnimatedModel: Auto-detected texture: " + autoTexture + "\n").c_str());
            } else {
                OutputDebugStringA("AnimatedModel: No PNG files found, using default\n");
                modelData.material.textureFilePath = "Resources/uvChecker.png";
            }
        }
    }
    
    aiColor3D color;
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        modelData.material.diffuse = {color.r, color.g, color.b, 1.0f};
        OutputDebugStringA(("AnimatedModel: Diffuse color: " + std::to_string(color.r) + ", " + std::to_string(color.g) + ", " + std::to_string(color.b) + "\n").c_str());
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
    
    if (!modelData.material.textureFilePath.empty()) {
        TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
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

std::string AnimatedModel::FindTextureInDirectory(const std::string& directoryPath) {
    WIN32_FIND_DATAA findData;
    std::string searchPath = directoryPath + "/*.png";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string foundTexture = directoryPath + "/" + findData.cFileName;
                OutputDebugStringA(("AnimatedModel: Found PNG in directory: " + foundTexture + "\n").c_str());
                FindClose(hFind);
                return foundTexture;
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
    
    return "";
}

std::string AnimatedModel::ParseMTLFile(const std::string& directoryPath, const std::string& objFileName) {
    std::string mtlFileName = objFileName;
    size_t dotPos = mtlFileName.find_last_of('.');
    if (dotPos != std::string::npos) {
        mtlFileName = mtlFileName.substr(0, dotPos) + ".mtl";
    } else {
        mtlFileName += ".mtl";
    }
    
    std::string mtlPath = directoryPath + "/" + mtlFileName;
    std::ifstream mtlFile(mtlPath);
    
    if (!mtlFile.is_open()) {
        OutputDebugStringA(("AnimatedModel: Cannot open MTL file: " + mtlPath + "\n").c_str());
        return "";
    }
    
    std::string line;
    while (std::getline(mtlFile, line)) {
        if (line.find("map_Kd") == 0) {
            size_t spacePos = line.find(' ');
            if (spacePos != std::string::npos) {
                std::string texturePath = line.substr(spacePos + 1);
                while (!texturePath.empty() && (texturePath.back() == '\r' || texturePath.back() == '\n' || texturePath.back() == ' ')) {
                    texturePath.pop_back();
                }
                OutputDebugStringA(("AnimatedModel: Found map_Kd in MTL: " + texturePath + "\n").c_str());
                return texturePath;
            }
        }
    }
    
    return "";
}

// AssimpのノードをEngineのNodeに変換する関数
void AnimatedModel::ProcessAssimpNode(const aiNode* assimpNode, Node& engineNode) {
    if (!assimpNode) {
        return;
    }
    
    // ノード名を設定
    engineNode.name = assimpNode->mName.C_Str();
    if (engineNode.name.empty()) {
        engineNode.name = "UnnamedNode";
    }
    
    // 変換行列を設定
    aiMatrix4x4 assimpMatrix = assimpNode->mTransformation;
    
    // assimpの行列をEngineの行列に変換
    Matrix4x4 transformMatrix;
    transformMatrix.m[0][0] = assimpMatrix.a1; transformMatrix.m[0][1] = assimpMatrix.a2; transformMatrix.m[0][2] = assimpMatrix.a3; transformMatrix.m[0][3] = assimpMatrix.a4;
    transformMatrix.m[1][0] = assimpMatrix.b1; transformMatrix.m[1][1] = assimpMatrix.b2; transformMatrix.m[1][2] = assimpMatrix.b3; transformMatrix.m[1][3] = assimpMatrix.b4;
    transformMatrix.m[2][0] = assimpMatrix.c1; transformMatrix.m[2][1] = assimpMatrix.c2; transformMatrix.m[2][2] = assimpMatrix.c3; transformMatrix.m[2][3] = assimpMatrix.c4;
    transformMatrix.m[3][0] = assimpMatrix.d1; transformMatrix.m[3][1] = assimpMatrix.d2; transformMatrix.m[3][2] = assimpMatrix.d3; transformMatrix.m[3][3] = assimpMatrix.d4;
    
    // 座標系変換（右手座標系から左手座標系へ）
    Matrix4x4 coordinateConversion = MakeIdentity4x4();
    coordinateConversion.m[2][2] = -1.0f;
    
    engineNode.localMatrix = Multiply(coordinateConversion, transformMatrix);
    
    // assimpの行列分解機能を使ってSRTを抽出
    aiVector3D scale, translation;
    aiQuaternion rotation;
    assimpMatrix.Decompose(scale, rotation, translation);
    
    // QuaternionTransformに設定
    engineNode.transform.scale = {scale.x, scale.y, scale.z};
    engineNode.transform.rotate = {-rotation.x, -rotation.y, -rotation.z, rotation.w};  // 座標系変換
    engineNode.transform.translate = {translation.x, translation.y, -translation.z};  // Z軸反転
    
    // 子ノードを処理
    engineNode.children.resize(assimpNode->mNumChildren);
    for (unsigned int i = 0; i < assimpNode->mNumChildren; i++) {
        ProcessAssimpNode(assimpNode->mChildren[i], engineNode.children[i]);
    }
}

// スケルトンのデバッグ描画用ライン生成
std::vector<std::pair<Vector3, Vector3>> AnimatedModel::GenerateSkeletonLines(const Matrix4x4& worldMatrix) const {
    std::vector<std::pair<Vector3, Vector3>> lines;
    
    if (!hasSkeleton_) {
        return lines;
    }
    
    // 各ジョイントからラインを生成
    for (const Joint& joint : skeleton_.joints) {
        // ジョイントの位置を取得（ワールド座標変換）
        Matrix4x4 jointWorldMatrix = Multiply(worldMatrix, joint.skeletonSpaceMatrix);
        Vector3 jointPos = {jointWorldMatrix.m[3][0], jointWorldMatrix.m[3][1], jointWorldMatrix.m[3][2]};
        
        // 親ジョイントとの間にラインを生成
        if (joint.parent) {
            const Joint& parentJoint = skeleton_.joints[*joint.parent];
            Matrix4x4 parentWorldMatrix = Multiply(worldMatrix, parentJoint.skeletonSpaceMatrix);
            Vector3 parentPos = {parentWorldMatrix.m[3][0], parentWorldMatrix.m[3][1], parentWorldMatrix.m[3][2]};
            
            // 親子ジョイント間のラインを登録
            lines.push_back({parentPos, jointPos});
        }
        
        // ジョイント位置に小さな十字を生成
        float crossSize = 0.05f;
        Vector3 right = {jointPos.x + crossSize, jointPos.y, jointPos.z};
        Vector3 left = {jointPos.x - crossSize, jointPos.y, jointPos.z};
        Vector3 up = {jointPos.x, jointPos.y + crossSize, jointPos.z};
        Vector3 down = {jointPos.x, jointPos.y - crossSize, jointPos.z};
        Vector3 forward = {jointPos.x, jointPos.y, jointPos.z + crossSize};
        Vector3 back = {jointPos.x, jointPos.y, jointPos.z - crossSize};
        
        // ジョイントを表す十字のラインを登録
        lines.push_back({left, right});
        lines.push_back({down, up});
        lines.push_back({back, forward});
    }
    
    return lines;
}