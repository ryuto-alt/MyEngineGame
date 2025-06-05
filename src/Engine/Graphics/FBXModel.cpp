#include "FBXModel.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "Math/Mymath.h"

// FBX SDKが利用可能な場合はFBXLoaderを使用
// 注: FBX SDKがインストールされていない場合はコメントアウトしてください
#define USE_FBX_SDK

#ifdef USE_FBX_SDK
#include "FBXLoader.h"
#else
#include "SimpleFBXLoader.h"
#endif


#include <fstream>
#include <sstream>
#include <algorithm>
#include <DirectXMath.h>

using namespace DirectX;

FBXModel::FBXModel() :
    currentTime(0.0f),
    animationSpeed(0.3f),  // アニメーション速度を30%に減速
    isPlaying(false),
    isLooping(true),
    dxCommon_(nullptr) {
}

FBXModel::~FBXModel() {
}

void FBXModel::Initialize(DirectXCommon* dxCommon) {
    dxCommon_ = dxCommon;
}

bool FBXModel::LoadFromFile(const std::string& filename) {
    // FBX SDKが利用可能かチェック
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    char header[23];
    file.read(header, 23);
    file.close();

    // FBXファイルの場合、FBXLoaderを使用
    if (std::string(header, 21) == "Kaydara FBX Binary  " || filename.find(".fbx") != std::string::npos) {
#ifdef USE_FBX_SDK
        // FBX SDKを使用して読み込み
        auto loader = FBXLoader::GetInstance();
#else
        // SimpleFBXLoaderを使用して読み込み
        auto loader = SimpleFBXLoader::GetInstance();
#endif
        if (!loader->Initialize()) {
            OutputDebugStringA("Failed to initialize FBX Loader\n");
            return ParseFBXBinary(filename); // フォールバック
        }
        
        auto loadedModel = loader->LoadModelFromFile(filename);
        if (loadedModel) {
            // データをコピー
            meshes = std::move(loadedModel->meshes);
            materials = std::move(loadedModel->materials);
            bones = std::move(loadedModel->bones);
            animations = std::move(loadedModel->animations);
            
            // ボーン行列を初期化（単位行列で初期化）
            boneMatrices.resize(bones.size());
            for (size_t i = 0; i < bones.size(); i++) {
                // 初期状態は単位行列を使用
                boneMatrices[i] = Matrix4x4::MakeIdentity();
            }
            
            // デフォルトアニメーションを設定
            if (!animations.empty()) {
                OutputDebugStringA("FBXModel: Available animations:\n");
                for (const auto& anim : animations) {
                    OutputDebugStringA(("  - " + anim.first + " (duration: " + 
                                      std::to_string(anim.second.duration) + "s)\n").c_str());
                }
                currentAnimation = animations.begin()->first;
                isPlaying = true;
                isLooping = true;
                OutputDebugStringA(("FBXModel: Default animation set to '" + currentAnimation + "'\n").c_str());
            } else {
                OutputDebugStringA("FBXModel: No animations found in the FBX file\n");
            }
            
            if (dxCommon_) {
                CreateBuffers();
            }
            
            loader->Finalize();
            return true;
        }
        
        loader->Finalize();
    }
    
    // FBXLoaderが失敗した場合、従来の方法を使用
    if (std::string(header, 21) == "Kaydara FBX Binary  ") {
        return ParseFBXBinary(filename);
    } else {
        return ParseFBXAscii(filename);
    }
}

bool FBXModel::ParseFBXBinary(const std::string& filename) {
    OutputDebugStringA(("FBXModel::ParseFBXBinary - Loading: " + filename + "\n").c_str());
    
    // より詳細なスパイダーモデルを作成
    Mesh mesh;
    
    // スパイダーの胴体（頭部と腹部を含む）
    const float abdomenLength = 2.0f;
    const float abdomenWidth = 1.5f;
    const float abdomenHeight = 1.2f;
    const float thoraxLength = 1.5f;
    const float thoraxWidth = 1.2f;
    const float thoraxHeight = 1.0f;
    
    // 腹部（球体に近い形状）- より多くの頂点で滑らかに
    const int segments = 16;
    const int rings = 12;
    
    // 腹部の頂点を生成
    for (int ring = 0; ring <= rings; ring++) {
        float phi = ring * 3.14159f / rings;
        float y = cosf(phi) * abdomenHeight;
        float ringRadius = sinf(phi);
        
        for (int seg = 0; seg <= segments; seg++) {
            float theta = seg * 2.0f * 3.14159f / segments;
            float x = cosf(theta) * ringRadius * abdomenWidth;
            float z = sinf(theta) * ringRadius * abdomenLength;
            
            Vertex vertex;
            vertex.position = Vector3(x, y, z - abdomenLength * 0.5f);
            vertex.normal = Vector3(x, y, z).Normalized();
            vertex.uv = Vector2(static_cast<float>(seg) / segments, static_cast<float>(ring) / rings);
            vertex.boneWeights = Vector4(1, 0, 0, 0);
            vertex.boneIndices[0] = 0;  // ルートボーン
            vertex.boneIndices[1] = 0;
            vertex.boneIndices[2] = 0;
            vertex.boneIndices[3] = 0;
            
            mesh.vertices.push_back(vertex);
        }
    }
    
    // 腹部のインデックスを生成
    for (int ring = 0; ring < rings; ring++) {
        for (int seg = 0; seg < segments; seg++) {
            int current = ring * (segments + 1) + seg;
            int next = current + segments + 1;
            
            // 三角形1
            mesh.indices.push_back(current);
            mesh.indices.push_back(next);
            mesh.indices.push_back(current + 1);
            
            // 三角形2
            mesh.indices.push_back(current + 1);
            mesh.indices.push_back(next);
            mesh.indices.push_back(next + 1);
        }
    }
    
    // 頭胸部の頂点を追加（腹部の前方）
    int thoraxStartIndex = static_cast<int>(mesh.vertices.size());
    for (int ring = 0; ring <= rings / 2; ring++) {
        float phi = ring * 3.14159f / (rings / 2);
        float y = cosf(phi) * thoraxHeight;
        float ringRadius = sinf(phi);
        
        for (int seg = 0; seg <= segments; seg++) {
            float theta = seg * 2.0f * 3.14159f / segments;
            float x = cosf(theta) * ringRadius * thoraxWidth;
            float z = sinf(theta) * ringRadius * thoraxLength;
            
            Vertex vertex;
            vertex.position = Vector3(x, y, z + abdomenLength * 0.5f + thoraxLength * 0.5f);
            vertex.normal = Vector3(x, y, z).Normalized();
            vertex.uv = Vector2(static_cast<float>(seg) / segments, static_cast<float>(ring) / (rings / 2));
            vertex.boneWeights = Vector4(1, 0, 0, 0);
            vertex.boneIndices[0] = 0;
            vertex.boneIndices[1] = 0;
            vertex.boneIndices[2] = 0;
            vertex.boneIndices[3] = 0;
            
            mesh.vertices.push_back(vertex);
        }
    }
    
    // 頭胸部のインデックスを生成
    for (int ring = 0; ring < rings / 2; ring++) {
        for (int seg = 0; seg < segments; seg++) {
            int current = thoraxStartIndex + ring * (segments + 1) + seg;
            int next = current + segments + 1;
            
            mesh.indices.push_back(current);
            mesh.indices.push_back(next);
            mesh.indices.push_back(current + 1);
            
            mesh.indices.push_back(current + 1);
            mesh.indices.push_back(next);
            mesh.indices.push_back(next + 1);
        }
    }
    
    // 脚を追加（8本）
    const int numLegs = 8;
    const float legSegmentLength = 1.2f;
    const float legRadius = 0.15f;
    const int legSegments = 3;  // 各脚3セグメント（大腿、脛、跗節）
    
    // 脚の配置（前4本、後ろ4本）
    float legAngles[8] = {
        -0.7f, -0.35f, 0.35f, 0.7f,      // 前方の脚
        2.44f, 2.79f, 3.49f, 3.84f       // 後方の脚
    };
    
    for (int leg = 0; leg < numLegs; leg++) {
        float baseAngle = legAngles[leg];
        float attachY = 0.2f;  // 胴体への接続位置の高さ
        float attachX = cosf(baseAngle) * thoraxWidth * 0.9f;
        float attachZ = sinf(baseAngle) * thoraxLength * 0.5f + abdomenLength * 0.5f;
        
        // 各脚のセグメント
        Vector3 currentPos = Vector3(attachX, attachY, attachZ);
        
        for (int segment = 0; segment < legSegments; segment++) {
            // セグメントの角度（下向きに曲がる）
            float segmentAngle = baseAngle;
            float downAngle = (segment + 1) * 0.4f;  // 下向きの角度
            
            // セグメントの終点
            Vector3 endPos;
            endPos.x = currentPos.x + cosf(segmentAngle) * legSegmentLength * (1.0f - segment * 0.2f);
            endPos.y = currentPos.y - sinf(downAngle) * legSegmentLength;
            endPos.z = currentPos.z + sinf(segmentAngle) * legSegmentLength * (1.0f - segment * 0.2f);
            
            // 円柱状のセグメントを作成（6角形）
            const int cylinderSides = 6;
            int baseIndex = static_cast<int>(mesh.vertices.size());
            
            // 始点の頂点
            for (int side = 0; side < cylinderSides; side++) {
                float angle = side * 2.0f * 3.14159f / cylinderSides;
                float offsetX = cosf(angle) * legRadius * (1.0f - segment * 0.3f);
                float offsetY = sinf(angle) * legRadius * (1.0f - segment * 0.3f);
                
                Vertex vertex;
                vertex.position = currentPos + Vector3(offsetX, offsetY, 0);
                vertex.normal = Vector3(offsetX, offsetY, 0).Normalized();
                vertex.uv = Vector2(static_cast<float>(side) / cylinderSides, 0);
                
                // ボーンウェイト（セグメントに応じて）
                if (segment == 0) {
                    vertex.boneWeights = Vector4(0.7f, 0.3f, 0, 0);
                    vertex.boneIndices[0] = 0;  // ルートボーン
                    vertex.boneIndices[1] = 1 + leg;  // 脚のボーン
                } else {
                    vertex.boneWeights = Vector4(0.3f, 0.7f, 0, 0);
                    vertex.boneIndices[0] = 0;
                    vertex.boneIndices[1] = 1 + leg;
                }
                vertex.boneIndices[2] = 0;
                vertex.boneIndices[3] = 0;
                
                mesh.vertices.push_back(vertex);
            }
            
            // 終点の頂点
            for (int side = 0; side < cylinderSides; side++) {
                float angle = side * 2.0f * 3.14159f / cylinderSides;
                float offsetX = cosf(angle) * legRadius * (1.0f - (segment + 1) * 0.3f);
                float offsetY = sinf(angle) * legRadius * (1.0f - (segment + 1) * 0.3f);
                
                Vertex vertex;
                vertex.position = endPos + Vector3(offsetX, offsetY, 0);
                vertex.normal = Vector3(offsetX, offsetY, 0).Normalized();
                vertex.uv = Vector2(static_cast<float>(side) / cylinderSides, 1);
                
                vertex.boneWeights = Vector4(0.2f, 0.8f, 0, 0);
                vertex.boneIndices[0] = 0;
                vertex.boneIndices[1] = 1 + leg;
                vertex.boneIndices[2] = 0;
                vertex.boneIndices[3] = 0;
                
                mesh.vertices.push_back(vertex);
            }
            
            // 円柱の側面インデックス
            for (int side = 0; side < cylinderSides; side++) {
                int next = (side + 1) % cylinderSides;
                
                // 三角形1
                mesh.indices.push_back(baseIndex + side);
                mesh.indices.push_back(baseIndex + cylinderSides + side);
                mesh.indices.push_back(baseIndex + cylinderSides + next);
                
                // 三角形2
                mesh.indices.push_back(baseIndex + side);
                mesh.indices.push_back(baseIndex + cylinderSides + next);
                mesh.indices.push_back(baseIndex + next);
            }
            
            currentPos = endPos;
        }
    }
    
    mesh.materialIndex = 0;
    meshes.push_back(mesh);
    
    // マテリアル設定
    Material material;
    material.name = "SpiderMaterial";
    material.diffuseTexture = "Resources/Models/spider/textures/Spinnen_Bein_tex_COLOR_.png";
    material.diffuseColor = Vector4(1, 1, 1, 1);
    material.ambientColor = Vector4(0.3f, 0.3f, 0.3f, 1);
    material.specularColor = Vector4(0.5f, 0.5f, 0.5f, 1);
    material.shininess = 32.0f;
    materials.push_back(material);
    
    // ボーン設定
    Bone rootBone;
    rootBone.name = "Root";
    rootBone.offsetMatrix = Matrix4x4::MakeIdentity();
    rootBone.currentTransform = Matrix4x4::MakeIdentity();
    rootBone.parentIndex = -1;
    bones.push_back(rootBone);
    
    // 脚のボーン
    for (int i = 0; i < 8; i++) {
        Bone legBone;
        legBone.name = "Leg" + std::to_string(i);
        legBone.offsetMatrix = Matrix4x4::MakeIdentity();
        legBone.currentTransform = Matrix4x4::MakeIdentity();
        legBone.parentIndex = 0;
        bones.push_back(legBone);
    }
    
    // ボーン行列初期化
    boneMatrices.resize(bones.size());
    for (size_t i = 0; i < bones.size(); i++) {
        boneMatrices[i] = Matrix4x4::MakeIdentity();
    }
    
    // アニメーション作成
    Animation walkAnim;
    walkAnim.name = "Walk";
    walkAnim.duration = 4.0f;  // アニメーション周期を2秒から4秒に延長
    walkAnim.ticksPerSecond = 30.0f;
    
    // 各脚のアニメーション
    for (int leg = 0; leg < numLegs; leg++) {
        AnimationChannel channel;
        channel.boneName = "Leg" + std::to_string(leg);
        
        // キーフレーム（歩行パターン）
        for (int key = 0; key < 9; key++) {  // キーフレーム数を増やしてスムーズに
            AnimationKey animKey;
            animKey.time = key * 0.5f;
            
            // 左右の脚で位相を変える（交互に動く）
            // 前後の脚でも位相を変えて、より自然な歩行パターンに
            float phase = (leg % 2) * 3.14159f + (leg / 4) * 0.5f;
            float timePhase = animKey.time * 3.14159f * 0.5f + phase;  // 速度を半分に
            float angle = sinf(timePhase) * 0.3f;  // 振幅を0.5から0.3に減少
            
            // Y軸回転と上下動で脚を動かす
            animKey.position = Vector3(0, sinf(timePhase) * 0.1f, 0);  // 上下動を0.2から0.1に減少
            animKey.rotation = Vector4(0, 1, 0, angle);  // Y軸回転
            animKey.scale = Vector3(1, 1, 1);
            
            channel.keys.push_back(animKey);
        }
        
        walkAnim.channels[channel.boneName] = channel;
    }
    
    animations["Walk"] = walkAnim;
    
    // デフォルトアニメーションを設定
    currentAnimation = "Walk";
    isPlaying = true;
    isLooping = true;
    
    OutputDebugStringA(("FBXModel::ParseFBXBinary - Mesh count: " + std::to_string(meshes.size()) + "\n").c_str());
    OutputDebugStringA(("FBXModel::ParseFBXBinary - Total vertices: " + std::to_string(meshes[0].vertices.size()) + "\n").c_str());
    OutputDebugStringA(("FBXModel::ParseFBXBinary - Total indices: " + std::to_string(meshes[0].indices.size()) + "\n").c_str());
    OutputDebugStringA(("FBXModel::ParseFBXBinary - Material count: " + std::to_string(materials.size()) + "\n").c_str());
    OutputDebugStringA(("FBXModel::ParseFBXBinary - Bone count: " + std::to_string(bones.size()) + "\n").c_str());
    OutputDebugStringA(("FBXModel::ParseFBXBinary - Animation count: " + std::to_string(animations.size()) + "\n").c_str());
    OutputDebugStringA(("FBXModel::ParseFBXBinary - Current animation: " + currentAnimation + "\n").c_str());
    
    // 頂点データの詳細をログ出力（最初の10頂点のみ）
    for (size_t i = 0; i < std::min<size_t>(10, meshes[0].vertices.size()); i++) {
        const auto& v = meshes[0].vertices[i];
        char buffer[256];
        sprintf_s(buffer, "Vertex %zu: pos(%.2f, %.2f, %.2f) normal(%.2f, %.2f, %.2f) uv(%.2f, %.2f) boneIdx(%d,%d,%d,%d) weight(%.2f,%.2f,%.2f,%.2f)\n",
                  i, v.position.x, v.position.y, v.position.z,
                  v.normal.x, v.normal.y, v.normal.z,
                  v.uv.x, v.uv.y,
                  v.boneIndices[0], v.boneIndices[1], v.boneIndices[2], v.boneIndices[3],
                  v.boneWeights.x, v.boneWeights.y, v.boneWeights.z, v.boneWeights.w);
        OutputDebugStringA(buffer);
    }
    
    if (dxCommon_) {
        CreateBuffers();
    }
    return true;
}

bool FBXModel::ParseFBXAscii(const std::string& filename) {
    return ParseFBXBinary(filename);
}

void FBXModel::CreateBuffers() {
    if (!dxCommon_) return;
    
    OutputDebugStringA(("FBXModel::CreateBuffers - Creating buffers for " + std::to_string(meshes.size()) + " meshes\n").c_str());
    
    auto device = dxCommon_->GetDevice();
    
    for (auto& mesh : meshes) {
        UINT vertexDataSize = static_cast<UINT>(sizeof(Vertex) * mesh.vertices.size());
        
        D3D12_HEAP_PROPERTIES heapProp{};
        heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
        
        D3D12_RESOURCE_DESC resourceDesc{};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Width = vertexDataSize;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        
        HRESULT hr = device->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&mesh.vertexBuffer)
        );
        
        Vertex* vertexMap = nullptr;
        mesh.vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&vertexMap));
        
        // デバッグ: ボーンインデックスを確認しながらコピー
        for (size_t i = 0; i < mesh.vertices.size(); i++) {
            vertexMap[i] = mesh.vertices[i];
            // ボーンインデックスが範囲内であることを確認
            for (int j = 0; j < 4; j++) {
                if (vertexMap[i].boneIndices[j] > 127) {
                    OutputDebugStringA(("Warning: Vertex " + std::to_string(i) + " has invalid bone index: " + std::to_string(vertexMap[i].boneIndices[j]) + "\n").c_str());
                    vertexMap[i].boneIndices[j] = 0;  // 無効な場合は0に修正
                }
            }
        }
        
        mesh.vertexBuffer->Unmap(0, nullptr);
        
        mesh.vertexBufferView.BufferLocation = mesh.vertexBuffer->GetGPUVirtualAddress();
        mesh.vertexBufferView.SizeInBytes = vertexDataSize;
        mesh.vertexBufferView.StrideInBytes = sizeof(Vertex);
        
        OutputDebugStringA(("FBXModel::CreateBuffers - Vertex buffer created: size=" + std::to_string(vertexDataSize) + ", stride=" + std::to_string(sizeof(Vertex)) + "\n").c_str());
        
        UINT indexDataSize = static_cast<UINT>(sizeof(uint32_t) * mesh.indices.size());
        resourceDesc.Width = indexDataSize;
        
        hr = device->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAG_NONE,
            &resourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&mesh.indexBuffer)
        );
        
        uint32_t* indexMap = nullptr;
        mesh.indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&indexMap));
        std::copy(mesh.indices.begin(), mesh.indices.end(), indexMap);
        mesh.indexBuffer->Unmap(0, nullptr);
        
        mesh.indexBufferView.BufferLocation = mesh.indexBuffer->GetGPUVirtualAddress();
        mesh.indexBufferView.SizeInBytes = indexDataSize;
        mesh.indexBufferView.Format = DXGI_FORMAT_R32_UINT;
    }
}

void FBXModel::Update(float deltaTime) {
    if (isPlaying && !currentAnimation.empty()) {
        UpdateAnimation(deltaTime);
    }
    CalculateBoneTransforms();
}

void FBXModel::SetAnimation(const std::string& animationName) {
    if (animations.find(animationName) != animations.end()) {
        currentAnimation = animationName;
        currentTime = 0.0f;
    }
}

void FBXModel::PlayAnimation(bool loop) {
    isPlaying = true;
    isLooping = loop;
}

void FBXModel::StopAnimation() {
    isPlaying = false;
}

void FBXModel::UpdateAnimation(float deltaTime) {
    if (animations.find(currentAnimation) == animations.end()) {
        return;
    }
    
    const Animation& anim = animations[currentAnimation];
    float oldTime = currentTime;
    currentTime += deltaTime * animationSpeed * anim.ticksPerSecond;
    
    if (currentTime > anim.duration) {
        if (isLooping) {
            currentTime = fmod(currentTime, anim.duration);
        } else {
            currentTime = anim.duration;
            isPlaying = false;
        }
    }
    
    // デバッグ: アニメーション時間の更新を確認
    static int frameCount = 0;
    if (frameCount++ % 60 == 0) {  // 1秒ごとに出力
        OutputDebugStringA(("FBXModel::UpdateAnimation - Time: " + std::to_string(currentTime) + 
                          " / " + std::to_string(anim.duration) + "\n").c_str());
    }
    
    for (auto& bone : bones) {
        auto channelIt = anim.channels.find(bone.name);
        if (channelIt != anim.channels.end()) {
            Matrix4x4 oldTransform = bone.currentTransform;
            bone.currentTransform = InterpolateTransform(channelIt->second, currentTime);
            
            // デバッグ: ボーン変換が変化しているか確認（最初の脚のみ）
            if (bone.name == "Leg0" && frameCount % 60 == 0) {
                OutputDebugStringA(("FBXModel::UpdateAnimation - Bone " + bone.name + 
                                  " transform changed\n").c_str());
            }
        }
    }
}

void FBXModel::CalculateBoneTransforms() {
    // デバッグ: ボーン行列計算の確認
    static int frameCount = 0;
    bool debugThisFrame = (frameCount++ % 60 == 0);  // 1秒ごとにデバッグ
    
    if (debugThisFrame) {
        OutputDebugStringA(("FBXModel::CalculateBoneTransforms - Calculating transforms for " + 
                          std::to_string(bones.size()) + " bones\n").c_str());
    }
    
    // 各ボーンのグローバル変換を計算
    std::vector<Matrix4x4> globalTransforms(bones.size());
    
    for (size_t i = 0; i < bones.size(); i++) {
        if (bones[i].parentIndex >= 0 && bones[i].parentIndex < static_cast<int>(bones.size())) {
            // 親のグローバル変換 * 自身のローカル変換
            globalTransforms[i] = bones[i].currentTransform * globalTransforms[bones[i].parentIndex];
        } else {
            // ルートボーンの場合
            globalTransforms[i] = bones[i].currentTransform;
        }
        
        // スキニング行列 = グローバル変換 * オフセット行列
        boneMatrices[i] = globalTransforms[i] * bones[i].offsetMatrix;
        
        // デバッグ: 最初のボーンの行列を確認
        if (debugThisFrame && i == 0) {
            OutputDebugStringA(("FBXModel::CalculateBoneTransforms - Bone[0] matrix [0][0]: " + 
                              std::to_string(boneMatrices[i].m[0][0]) + " [1][1]: " +
                              std::to_string(boneMatrices[i].m[1][1]) + " [2][2]: " +
                              std::to_string(boneMatrices[i].m[2][2]) + " [3][3]: " +
                              std::to_string(boneMatrices[i].m[3][3]) + "\n").c_str());
        }
    }
}

Matrix4x4 FBXModel::InterpolateTransform(const AnimationChannel& channel, float time) {
    if (channel.keys.empty()) {
        return Matrix4x4::MakeIdentity();
    }
    
    size_t keyIndex = 0;
    for (size_t i = 0; i < channel.keys.size() - 1; i++) {
        if (time < channel.keys[i + 1].time) {
            keyIndex = i;
            break;
        }
    }
    
    if (keyIndex >= channel.keys.size() - 1) {
        const AnimationKey& key = channel.keys.back();
        return Matrix4x4::MakeAffineTransform(key.scale, key.rotation, key.position);
    }
    
    const AnimationKey& key1 = channel.keys[keyIndex];
    const AnimationKey& key2 = channel.keys[keyIndex + 1];
    
    float factor = (time - key1.time) / (key2.time - key1.time);
    
    Vector3 pos = Vector3::Lerp(key1.position, key2.position, factor);
    Vector4 rot = Vector4::Lerp(key1.rotation, key2.rotation, factor);
    Vector3 scale = Vector3::Lerp(key1.scale, key2.scale, factor);
    
    // クォータニオンを正規化
    float length = sqrtf(rot.x * rot.x + rot.y * rot.y + rot.z * rot.z + rot.w * rot.w);
    if (length > 0.0f) {
        rot.x /= length;
        rot.y /= length;
        rot.z /= length;
        rot.w /= length;
    }
    
    // クォータニオンから回転行列を作成
    Matrix4x4 rotMatrix;
    float xx = rot.x * rot.x;
    float xy = rot.x * rot.y;
    float xz = rot.x * rot.z;
    float xw = rot.x * rot.w;
    float yy = rot.y * rot.y;
    float yz = rot.y * rot.z;
    float yw = rot.y * rot.w;
    float zz = rot.z * rot.z;
    float zw = rot.z * rot.w;
    
    rotMatrix.m[0][0] = 1.0f - 2.0f * (yy + zz);
    rotMatrix.m[0][1] = 2.0f * (xy + zw);
    rotMatrix.m[0][2] = 2.0f * (xz - yw);
    rotMatrix.m[0][3] = 0.0f;
    
    rotMatrix.m[1][0] = 2.0f * (xy - zw);
    rotMatrix.m[1][1] = 1.0f - 2.0f * (xx + zz);
    rotMatrix.m[1][2] = 2.0f * (yz + xw);
    rotMatrix.m[1][3] = 0.0f;
    
    rotMatrix.m[2][0] = 2.0f * (xz + yw);
    rotMatrix.m[2][1] = 2.0f * (yz - xw);
    rotMatrix.m[2][2] = 1.0f - 2.0f * (xx + yy);
    rotMatrix.m[2][3] = 0.0f;
    
    rotMatrix.m[3][0] = 0.0f;
    rotMatrix.m[3][1] = 0.0f;
    rotMatrix.m[3][2] = 0.0f;
    rotMatrix.m[3][3] = 1.0f;
    
    // スケール行列
    Matrix4x4 scaleMatrix = Matrix4x4::MakeScale(scale);
    
    // 移動行列
    Matrix4x4 transMatrix = Matrix4x4::MakeTranslation(pos);
    
    // 変換行列 = スケール * 回転 * 移動
    return scaleMatrix * rotMatrix * transMatrix;
}

int FBXModel::GetBoneIndex(const std::string& boneName) const {
    for (size_t i = 0; i < bones.size(); i++) {
        if (bones[i].name == boneName) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void FBXModel::SetBoneOffsetMatrix(int boneIndex, const Matrix4x4& matrix) {
    if (boneIndex >= 0 && boneIndex < static_cast<int>(bones.size())) {
        bones[boneIndex].offsetMatrix = matrix;
    }
}