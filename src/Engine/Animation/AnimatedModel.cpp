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
        ModelData& modelDataInternal = GetModelDataInternal();
        if (!modelDataInternal.rootNode.name.empty()) {
            rootNodeName_ = modelDataInternal.rootNode.name;
        } else {
            modelDataInternal.rootNode.name = "root";
            modelDataInternal.rootNode.localMatrix = MakeIdentity4x4();
            rootNodeName_ = "root";
        }
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
        modelData.rootNode.name = scene->mRootNode->mName.C_Str();
        rootNodeName_ = modelData.rootNode.name;
        
        // ルートノードの変換行列を設定（座標系変換を適用）
        aiMatrix4x4 assimpMatrix = scene->mRootNode->mTransformation;
        
        // assimpの行列をDirectX形式に変換（右手座標系→左手座標系）
        Matrix4x4 transformMatrix;
        transformMatrix.m[0][0] = assimpMatrix.a1; transformMatrix.m[0][1] = assimpMatrix.a2; transformMatrix.m[0][2] = assimpMatrix.a3; transformMatrix.m[0][3] = assimpMatrix.a4;
        transformMatrix.m[1][0] = assimpMatrix.b1; transformMatrix.m[1][1] = assimpMatrix.b2; transformMatrix.m[1][2] = assimpMatrix.b3; transformMatrix.m[1][3] = assimpMatrix.b4;
        transformMatrix.m[2][0] = assimpMatrix.c1; transformMatrix.m[2][1] = assimpMatrix.c2; transformMatrix.m[2][2] = assimpMatrix.c3; transformMatrix.m[2][3] = assimpMatrix.c4;
        transformMatrix.m[3][0] = assimpMatrix.d1; transformMatrix.m[3][1] = assimpMatrix.d2; transformMatrix.m[3][2] = assimpMatrix.d3; transformMatrix.m[3][3] = assimpMatrix.d4;
        
        // 座標系変換行列を作成（Z軸反転）
        Matrix4x4 coordinateConversion = MakeIdentity4x4();
        coordinateConversion.m[2][2] = -1.0f;  // Z軸を反転
        
        // 座標系変換を適用
        modelData.rootNode.localMatrix = Multiply(coordinateConversion, transformMatrix);
        
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
    
    // インデックスを使用して三角形ごとに頂点を作成（DirectX用に座標変換も適用）
    for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++) {
        const aiFace& face = mesh->mFaces[faceIndex];
        
        if (face.mNumIndices != 3) {
            OutputDebugStringA(("AnimatedModel: Warning - Face " + std::to_string(faceIndex) + " has " + std::to_string(face.mNumIndices) + " indices (expected 3)\n").c_str());
            continue;
        }
        
        // DirectX用にワインディングオーダーを反転（0, 2, 1の順序）
        unsigned int indices[3] = { face.mIndices[0], face.mIndices[2], face.mIndices[1] };
        
        for (int i = 0; i < 3; i++) {
            unsigned int vertexIndex = indices[i];
            VertexData vertex{};
            
            // 位置（右手座標系→左手座標系：Z座標を反転）
            vertex.position = {
                mesh->mVertices[vertexIndex].x,
                mesh->mVertices[vertexIndex].y,
                -mesh->mVertices[vertexIndex].z,  // Z座標を反転
                1.0f
            };
            
            // 法線（右手座標系→左手座標系：Z成分を反転）
            if (mesh->HasNormals()) {
                vertex.normal = {
                    mesh->mNormals[vertexIndex].x,
                    mesh->mNormals[vertexIndex].y,
                    -mesh->mNormals[vertexIndex].z  // Z成分を反転
                };
            } else {
                vertex.normal = {0.0f, 1.0f, 0.0f};
            }
            
            // テクスチャ座標（最初のセットのみ）
            if (mesh->mTextureCoords[0]) {
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
            // これにより回転方向が適切に変換される
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
        
        OutputDebugStringA(("AnimatedModel: Node " + nodeName + " - Position keys: " + std::to_string(nodeAnimation.translate.size()) + 
                          ", Rotation keys: " + std::to_string(nodeAnimation.rotate.size()) + 
                          ", Scale keys: " + std::to_string(nodeAnimation.scale.size()) + "\n").c_str());
    }
    
    // アニメーションプレイヤーに設定
    animationPlayer_.SetAnimation(animation_);
    animationPlayer_.SetLoop(true);
    
    OutputDebugStringA(("AnimatedModel: Animation duration: " + std::to_string(animation_.duration) + " seconds\n").c_str());
}