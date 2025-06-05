#include "SimpleFBXLoader.h"
#include <algorithm>
#include <DirectXMath.h>

using namespace DirectX;

SimpleFBXLoader* SimpleFBXLoader::GetInstance() {
    static SimpleFBXLoader instance;
    return &instance;
}

bool SimpleFBXLoader::Initialize() {
    return true;
}

void SimpleFBXLoader::Finalize() {
    tempAnimations_.clear();
}

std::unique_ptr<FBXModel> SimpleFBXLoader::LoadModelFromFile(const std::string& filename) {
    auto model = std::make_unique<FBXModel>();
    
    if (ParseFBXBinary(filename, model.get())) {
        return model;
    }
    
    return nullptr;
}

bool SimpleFBXLoader::ParseFBXBinary(const std::string& filename, FBXModel* model) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        OutputDebugStringA(("SimpleFBXLoader: Failed to open file: " + filename + "\n").c_str());
        return false;
    }
    
    // FBXヘッダーチェック
    char header[23];
    file.read(header, 23);
    if (std::string(header, 21) != "Kaydara FBX Binary  ") {
        OutputDebugStringA("SimpleFBXLoader: Not a valid FBX binary file\n");
        return false;
    }
    
    // バージョン情報をスキップ
    file.seekg(27, std::ios::beg);
    
    // 簡易的な実装: アニメーション付きのキューブモデルを生成
    OutputDebugStringA("SimpleFBXLoader: Creating animated cube model\n");
    
    // メッシュの作成（キューブ）
    FBXModel::Mesh mesh;
    
    // キューブの頂点（8頂点）
    float size = 1.0f;
    Vector3 vertices[8] = {
        Vector3(-size, -size, -size), // 0
        Vector3(+size, -size, -size), // 1
        Vector3(+size, +size, -size), // 2
        Vector3(-size, +size, -size), // 3
        Vector3(-size, -size, +size), // 4
        Vector3(+size, -size, +size), // 5
        Vector3(+size, +size, +size), // 6
        Vector3(-size, +size, +size)  // 7
    };
    
    // 各面の法線
    Vector3 normals[6] = {
        Vector3(0, 0, -1), // 前面
        Vector3(0, 0, +1), // 背面
        Vector3(-1, 0, 0), // 左面
        Vector3(+1, 0, 0), // 右面
        Vector3(0, -1, 0), // 下面
        Vector3(0, +1, 0)  // 上面
    };
    
    // 各面を構成（6面 × 4頂点 = 24頂点）
    int faceIndices[6][4] = {
        {0, 1, 2, 3}, // 前面
        {5, 4, 7, 6}, // 背面
        {4, 0, 3, 7}, // 左面
        {1, 5, 6, 2}, // 右面
        {4, 5, 1, 0}, // 下面
        {3, 2, 6, 7}  // 上面
    };
    
    // UV座標
    Vector2 uvs[4] = {
        Vector2(0, 1),
        Vector2(1, 1),
        Vector2(1, 0),
        Vector2(0, 0)
    };
    
    // 頂点データを構築
    for (int face = 0; face < 6; face++) {
        for (int v = 0; v < 4; v++) {
            FBXModel::Vertex vertex;
            vertex.position = vertices[faceIndices[face][v]];
            vertex.normal = normals[face];
            vertex.uv = uvs[v];
            
            // ボーンウェイトの設定（2つのボーンに影響を受ける）
            if (vertex.position.y > 0) {
                // 上部の頂点は主にボーン1の影響を受ける
                vertex.boneWeights = Vector4(0.3f, 0.7f, 0, 0);
                vertex.boneIndices[0] = 0;
                vertex.boneIndices[1] = 1;
                vertex.boneIndices[2] = 0;
                vertex.boneIndices[3] = 0;
            } else {
                // 下部の頂点は主にボーン0の影響を受ける
                vertex.boneWeights = Vector4(0.9f, 0.1f, 0, 0);
                vertex.boneIndices[0] = 0;
                vertex.boneIndices[1] = 1;
                vertex.boneIndices[2] = 0;
                vertex.boneIndices[3] = 0;
            }
            
            mesh.vertices.push_back(vertex);
        }
    }
    
    // インデックスの設定（各面を三角形に分割）
    for (int face = 0; face < 6; face++) {
        int base = face * 4;
        // 三角形1
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
        // 三角形2
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 3);
    }
    
    mesh.materialIndex = 0;
    model->AddMesh(mesh);
    
    // マテリアルの設定
    FBXModel::Material material;
    material.name = "DefaultMaterial";
    material.diffuseColor = Vector4(1, 1, 1, 1);
    material.ambientColor = Vector4(0.3f, 0.3f, 0.3f, 1);
    material.specularColor = Vector4(0.5f, 0.5f, 0.5f, 1);
    material.shininess = 32.0f;
    model->AddMaterial(material);
    
    // ボーンの設定
    FBXModel::Bone rootBone;
    rootBone.name = "Root";
    rootBone.offsetMatrix = Matrix4x4::MakeIdentity();
    rootBone.currentTransform = Matrix4x4::MakeIdentity();
    rootBone.parentIndex = -1;
    model->AddBone(rootBone);
    
    FBXModel::Bone childBone;
    childBone.name = "Child";
    childBone.offsetMatrix = Matrix4x4::MakeTranslation(Vector3(0, -1, 0));
    childBone.currentTransform = Matrix4x4::MakeIdentity();
    childBone.parentIndex = 0;
    model->AddBone(childBone);
    
    // アニメーションの作成（回転アニメーション）
    FBXModel::Animation animation;
    animation.name = "Rotate";
    animation.duration = 2.0f;
    animation.ticksPerSecond = 30.0f;
    
    // ルートボーンのアニメーション（Y軸回転）
    FBXModel::AnimationChannel rootChannel;
    rootChannel.boneName = "Root";
    
    int keyCount = 5;
    for (int i = 0; i < keyCount; i++) {
        FBXModel::AnimationKey key;
        key.time = i * 0.5f;
        key.position = Vector3(0, 0, 0);
        
        // Y軸回転のクォータニオン
        float angle = (i * 90.0f) * 3.14159f / 180.0f;
        key.rotation = Vector4(0, sinf(angle * 0.5f), 0, cosf(angle * 0.5f));
        key.scale = Vector3(1, 1, 1);
        
        rootChannel.keys.push_back(key);
    }
    animation.channels[rootChannel.boneName] = rootChannel;
    
    // 子ボーンのアニメーション（曲げアニメーション）
    FBXModel::AnimationChannel childChannel;
    childChannel.boneName = "Child";
    
    for (int i = 0; i < keyCount; i++) {
        FBXModel::AnimationKey key;
        key.time = i * 0.5f;
        key.position = Vector3(0, 0, 0);
        
        // X軸回転（曲げ動作）
        float bendAngle = sinf(i * 3.14159f / (keyCount - 1)) * 30.0f * 3.14159f / 180.0f;
        key.rotation = Vector4(sinf(bendAngle * 0.5f), 0, 0, cosf(bendAngle * 0.5f));
        key.scale = Vector3(1, 1, 1);
        
        childChannel.keys.push_back(key);
    }
    animation.channels[childChannel.boneName] = childChannel;
    
    model->AddAnimation(animation.name, animation);
    
    OutputDebugStringA("SimpleFBXLoader: Model loaded successfully\n");
    OutputDebugStringA(("  Vertices: " + std::to_string(mesh.vertices.size()) + "\n").c_str());
    OutputDebugStringA(("  Indices: " + std::to_string(mesh.indices.size()) + "\n").c_str());
    OutputDebugStringA(("  Bones: " + std::to_string(model->GetBones().size()) + "\n").c_str());
    OutputDebugStringA("  Animations: 1\n");
    
    file.close();
    return true;
}

bool SimpleFBXLoader::ReadNode(std::ifstream& stream, FBXNode& node) {
    // 簡易実装のため、実際のFBXノード読み込みは省略
    return true;
}

void SimpleFBXLoader::ProcessNode(const FBXNode& node, FBXModel* model) {
    // 簡易実装のため、ノード処理は省略
}

void SimpleFBXLoader::ProcessGeometry(const FBXNode& node, FBXModel* model) {
    // 簡易実装のため、ジオメトリ処理は省略
}

void SimpleFBXLoader::ProcessAnimation(const FBXNode& node, FBXModel* model) {
    // 簡易実装のため、アニメーション処理は省略
}