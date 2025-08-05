#include "ProceduralTree.h"
#include <cmath>
#define NOMINMAX  // Windows.hのmin/maxマクロを無効化
#include <algorithm>

// ベクトル関数のヘルパー
namespace {
    float VectorLength(const Vector3& v) {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }
    
    Vector3 VectorNormalize(const Vector3& v) {
        float length = VectorLength(v);
        if (length > 0.0f) {
            return v / length;
        }
        return v;
    }
    
    Vector3 VectorCross(const Vector3& a, const Vector3& b) {
        return Vector3{
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }
    
    float VectorDot(const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
}

ProceduralTree::ProceduralTree() 
    : dxCommon_(nullptr)
    , spriteCommon_(nullptr) 
    , camera_(nullptr)
    , randomEngine_(std::random_device{}())
    , randomDist_(-1.0f, 1.0f) {
}

ProceduralTree::~ProceduralTree() {
    Clear();
}

void ProceduralTree::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon, Camera* camera) {
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;
    camera_ = camera;
}

void ProceduralTree::Generate(const TreeParams& params, const Vector3& position) {
    Clear();
    
    treePosition_ = position;
    branches_.clear();
    
    // 統合メッシュデータをクリア
    unifiedMeshData_.vertices.clear();
    
    // 幹の生成（より自然な形状）
    AddTrunkToMesh(params);
    
    // 枝の生成開始位置（より自然な配置）
    float branchStartHeight = params.trunkHeight * 0.2f; // より低い位置から開始
    int branchLevels = 5; // より多くのレベル
    
    for (int i = 0; i < branchLevels; ++i) {
        float heightRatio = i / static_cast<float>(branchLevels - 1);
        float height = branchStartHeight + (params.trunkHeight - branchStartHeight) * heightRatio;
        
        // 高さに応じて枝の数を調整
        int branchCount = params.branchesPerNode;
        if (i == 0) branchCount = 2; // 下部は少なめ
        if (i >= branchLevels - 2) branchCount = params.branchesPerNode + 1; // 上部は多め
        
        // 螺旋状に枝を配置
        float spiralOffset = i * 0.618033f * 2.0f * 3.14159265f; // 黄金角
        
        for (int j = 0; j < branchCount; ++j) {
            float angle = (2.0f * 3.14159265f / branchCount) * j + spiralOffset;
            angle += randomDist_(randomEngine_) * 0.3f; // ランダム性を追加
            
            // より自然な枝の向き
            float horizontalStrength = 0.7f + randomDist_(randomEngine_) * 0.2f;
            float verticalStrength = params.branchUpwardBias + randomDist_(randomEngine_) * 0.2f;
            
            Vector3 direction = {
                cosf(angle) * horizontalStrength,
                verticalStrength,
                sinf(angle) * horizontalStrength
            };
            direction = VectorNormalize(direction);
            
            // 高さに応じて枝の長さと太さを調整
            float lengthMultiplier = 1.0f - heightRatio * 0.3f;
            float branchLength = params.trunkHeight * params.branchLengthRatio * lengthMultiplier;
            float branchRadius = params.trunkRadius * params.branchRadiusRatio * (1.0f - heightRatio * 0.5f);
            
            AddBranchesToMesh(params, 
                            treePosition_ + Vector3{0.0f, height, 0.0f}, 
                            direction, branchLength, branchRadius, 0);
        }
    }
    
    // 単一のモデルとオブジェクトを作成
    treeModel_ = std::make_unique<Model>();
    treeModel_->Initialize(dxCommon_);
    treeModel_->GetModelDataInternal() = unifiedMeshData_;
    treeModel_->CreateVertexBuffer();
    
    treeObject_ = std::make_unique<Object3d>();
    treeObject_->Initialize(dxCommon_, spriteCommon_);
    treeObject_->SetModel(treeModel_.get());
    treeObject_->SetCamera(camera_);
    treeObject_->SetEnableLighting(true);
    treeObject_->SetDirectionalLight(directionalLight_);
    treeObject_->SetSpotLight(spotLight_);
    
    // 重要：全体の色を設定（茶色の幹の色をベースにする）
    treeObject_->SetColor(params.trunkColor);
}

void ProceduralTree::AddTrunkToMesh(const TreeParams& params) {
    // より自然な幹の生成（わずかな曲がりを含む）
    const int segments = 5;
    std::vector<Vector3> trunkPoints;
    std::vector<float> trunkRadii;
    
    for (int i = 0; i <= segments; ++i) {
        float t = i / static_cast<float>(segments);
        float height = params.trunkHeight * t;
        
        // 幹の曲がり
        float curve = sinf(t * 3.14159265f) * params.trunkCurve;
        Vector3 offset = {
            curve * cosf(t * 2.0f),
            0.0f,
            curve * sinf(t * 2.0f)
        };
        
        trunkPoints.push_back(treePosition_ + Vector3{0.0f, height, 0.0f} + offset);
        
        // 高さに応じて太さを変える（根元が太い）
        float radiusMultiplier = 1.0f - t * 0.4f;
        trunkRadii.push_back(params.trunkRadius * radiusMultiplier);
    }
    
    // セグメント間を円柱で繋ぐ
    for (int i = 0; i < segments; ++i) {
        AddCylinderToMesh(trunkPoints[i], trunkPoints[i + 1], 
                         trunkRadii[i], trunkRadii[i + 1], 
                         params.trunkSegments, params.trunkColor);
    }
}

void ProceduralTree::AddBranchesToMesh(const TreeParams& params, const Vector3& startPos, 
                                       const Vector3& direction, float length, float radius, int depth,
                                       const Vector3& parentEnd) {
    if (depth >= params.maxBranchDepth) {
        // 最終階層では葉クラスターを生成
        AddLeafCluster(params, startPos + direction * length, direction);
        return;
    }
    
    // 枝をセグメント化してより自然な曲線を作る
    const int segments = 3;
    std::vector<Vector3> branchPoints;
    std::vector<float> branchRadii;
    
    Vector3 currentDir = direction;
    Vector3 currentPos = startPos;
    
    branchPoints.push_back(currentPos);
    branchRadii.push_back(radius);
    
    for (int i = 1; i <= segments; ++i) {
        float t = i / static_cast<float>(segments);
        
        // 重力の影響で少し下向きに
        currentDir.y -= 0.1f * t;
        currentDir = VectorNormalize(currentDir);
        
        float segmentLength = length / segments;
        currentPos = currentPos + currentDir * segmentLength;
        
        branchPoints.push_back(currentPos);
        branchRadii.push_back(radius * (1.0f - t * 0.3f));
    }
    
    // セグメント間を円柱で繋ぐ
    for (int i = 0; i < segments; ++i) {
        int segmentCount = (depth == 0) ? 5 : 4;
        AddCylinderToMesh(branchPoints[i], branchPoints[i + 1], 
                         branchRadii[i], branchRadii[i + 1], 
                         segmentCount, params.trunkColor);
    }
    
    Vector3 endPos = branchPoints.back();
    
    // 枝の途中にも葉を配置
    if (depth >= 1) {
        for (int i = 1; i < segments; ++i) {
            if (randomDist_(randomEngine_) > 0.3f) {
                AddLeafCluster(params, branchPoints[i], currentDir);
            }
        }
    }
    
    // 枝情報を保存
    branches_.push_back({startPos, endPos, radius, depth});
    
    // 子枝を生成
    int childBranches = (depth < 1) ? 2 + (randomDist_(randomEngine_) > 0.5f ? 1 : 0) : 2;
    
    for (int i = 0; i < childBranches; ++i) {
        // より自然な分岐角度
        float branchAngle = params.branchAngleBase + randomDist_(randomEngine_) * params.branchAngleVariance;
        float rotationAngle = (2.0f * 3.14159265f / childBranches) * i + randomDist_(randomEngine_) * 0.5f;
        
        // 親の方向から新しい方向を計算
        Vector3 up = {0.0f, 1.0f, 0.0f};
        Vector3 right = VectorNormalize(VectorCross(currentDir, up));
        if (VectorLength(right) < 0.1f) {
            right = {1.0f, 0.0f, 0.0f};
        }
        Vector3 forward = VectorCross(right, currentDir);
        
        Vector3 newDirection = currentDir * cosf(branchAngle) +
                              (right * cosf(rotationAngle) + forward * sinf(rotationAngle)) * sinf(branchAngle);
        newDirection = VectorNormalize(newDirection);
        
        float newLength = length * params.branchLengthRatio * (0.8f + randomDist_(randomEngine_) * 0.3f);
        float newRadius = radius * params.branchRadiusRatio;
        
        AddBranchesToMesh(params, endPos, newDirection, newLength, newRadius, depth + 1, endPos);
    }
}

void ProceduralTree::AddLeafCluster(const TreeParams& params, const Vector3& position, const Vector3& normal) {
    // 葉クラスターの生成
    for (int i = 0; i < params.leavesPerCluster; ++i) {
        // クラスター内での葉の配置
        float angle = (2.0f * 3.14159265f / params.leavesPerCluster) * i + randomDist_(randomEngine_) * 0.5f;
        float distance = params.leafSize * 0.5f * (0.5f + randomDist_(randomEngine_) * 0.5f);
        
        Vector3 offset = {
            cosf(angle) * distance,
            randomDist_(randomEngine_) * params.leafSize * 0.3f,
            sinf(angle) * distance
        };
        
        Vector3 leafPos = position + offset;
        
        // 葉の色にバリエーションを追加
        Vector4 leafColor = params.leafColor;
        leafColor.x += randomDist_(randomEngine_) * params.leafColorVariation.x;
        leafColor.y += randomDist_(randomEngine_) * params.leafColorVariation.y;
        leafColor.z += randomDist_(randomEngine_) * params.leafColorVariation.z;
        
        // 色を正規化
        leafColor.x = (std::max)(0.0f, (std::min)(1.0f, leafColor.x));
        leafColor.y = (std::max)(0.0f, (std::min)(1.0f, leafColor.y));
        leafColor.z = (std::max)(0.0f, (std::min)(1.0f, leafColor.z));
        
        AddLeafBillboardToMesh(leafPos, params.leafSize * (0.8f + randomDist_(randomEngine_) * 0.4f), leafColor);
    }
}

void ProceduralTree::AddLeavesToMesh(const TreeParams& params, const Vector3& position) {
    // 複数の葉クラスターを生成
    for (int i = 0; i < params.leafClustersPerBranch; ++i) {
        Vector3 clusterOffset = {
            randomDist_(randomEngine_) * params.leafSize * 2.0f,
            randomDist_(randomEngine_) * params.leafSize,
            randomDist_(randomEngine_) * params.leafSize * 2.0f
        };
        
        Vector3 clusterPos = position + clusterOffset;
        AddLeafCluster(params, clusterPos, Vector3{0.0f, 1.0f, 0.0f});
    }
}

void ProceduralTree::AddCylinderToMesh(const Vector3& start, const Vector3& end, 
                                      float startRadius, float endRadius, int segments, 
                                      const Vector4& color) {
    Vector3 axis = end - start;
    float height = VectorLength(axis);
    axis = VectorNormalize(axis);
    
    // 軸に垂直なベクトルを生成
    Vector3 perpendicular;
    if (std::abs(axis.y) < 0.99f) {
        perpendicular = VectorCross(axis, Vector3{0.0f, 1.0f, 0.0f});
    } else {
        perpendicular = VectorCross(axis, Vector3{1.0f, 0.0f, 0.0f});
    }
    perpendicular = VectorNormalize(perpendicular);
    Vector3 perpendicular2 = VectorCross(axis, perpendicular);
    
    // 円柱の頂点インデックスを記録
    size_t baseIndex = unifiedMeshData_.vertices.size();
    
    // 頂点生成（高さ方向は2段階のみ）
    for (int h = 0; h <= 1; ++h) {
        float t = static_cast<float>(h);
        float radius = startRadius * (1.0f - t) + endRadius * t;
        Vector3 center = start + axis * (height * t);
        
        for (int i = 0; i <= segments; ++i) {
            float angle = 2.0f * 3.14159265f * i / segments;
            float x = cosf(angle);
            float z = sinf(angle);
            
            Vector3 pos = center + perpendicular * (x * radius) + perpendicular2 * (z * radius);
            Vector3 normal = VectorNormalize(perpendicular * x + perpendicular2 * z);
            
            VertexData vertex;
            vertex.position = {pos.x, pos.y, pos.z, 1.0f};
            vertex.normal = normal;
            vertex.texcoord = {static_cast<float>(i) / segments, t};
            
            unifiedMeshData_.vertices.push_back(vertex);
        }
    }
    
    // 側面の三角形生成（各三角形ごとに独立した頂点を追加）
    for (int i = 0; i < segments; ++i) {
        // 下側の頂点
        VertexData v0 = unifiedMeshData_.vertices[baseIndex + i];
        VertexData v1 = unifiedMeshData_.vertices[baseIndex + i + 1];
        // 上側の頂点
        VertexData v2 = unifiedMeshData_.vertices[baseIndex + (segments + 1) + i];
        VertexData v3 = unifiedMeshData_.vertices[baseIndex + (segments + 1) + i + 1];
        
        // 最初の三角形（v0, v2, v3）
        unifiedMeshData_.vertices.push_back(v0);
        unifiedMeshData_.vertices.push_back(v2);
        unifiedMeshData_.vertices.push_back(v3);
        
        // 二番目の三角形（v0, v3, v1）
        unifiedMeshData_.vertices.push_back(v0);
        unifiedMeshData_.vertices.push_back(v3);
        unifiedMeshData_.vertices.push_back(v1);
    }
    
    // マテリアル設定（ベースカラーを設定）
    unifiedMeshData_.material.baseColorFactor = color;
}

void ProceduralTree::AddLeafBillboardToMesh(const Vector3& position, float size, const Vector4& color) {
    // リーフカード（ダイヤモンド形状）
    const float aspectRatio = 1.5f; // 縦長の葉
    Vector3 halfSize = {size * 0.5f, size * 0.5f * aspectRatio, 0.0f};
    
    // ランダムな3軸回転
    float angleX = randomDist_(randomEngine_) * 3.14159265f * 0.2f;
    float angleY = randomDist_(randomEngine_) * 3.14159265f * 2.0f;
    float angleZ = randomDist_(randomEngine_) * 3.14159265f * 0.3f;
    
    // 葉の形状（ダイヤモンド型）
    Vector3 positions[4] = {
        {0.0f, halfSize.y, 0.0f},         // 上
        {-halfSize.x, 0.0f, 0.0f},        // 左
        {0.0f, -halfSize.y * 0.8f, 0.0f}, // 下（少し短め）
        {halfSize.x, 0.0f, 0.0f}          // 右
    };
    
    Vector2 texcoords[4] = {
        {0.5f, 0.0f}, {0.0f, 0.5f}, {0.5f, 1.0f}, {1.0f, 0.5f}
    };
    
    // 3D回転を適用
    auto rotatePoint = [&](const Vector3& p) -> Vector3 {
        // X軸回転
        float y1 = p.y * cosf(angleX) - p.z * sinf(angleX);
        float z1 = p.y * sinf(angleX) + p.z * cosf(angleX);
        
        // Y軸回転
        float x2 = p.x * cosf(angleY) - z1 * sinf(angleY);
        float z2 = p.x * sinf(angleY) + z1 * cosf(angleY);
        
        // Z軸回転
        float x3 = x2 * cosf(angleZ) - y1 * sinf(angleZ);
        float y3 = x2 * sinf(angleZ) + y1 * cosf(angleZ);
        
        return {x3, y3, z2};
    };
    
    // 頂点データを統合メッシュに追加
    size_t baseIndex = unifiedMeshData_.vertices.size();
    
    // 法線の計算（葉の表面に垂直）
    Vector3 edge1 = positions[1] - positions[0];
    Vector3 edge2 = positions[2] - positions[0];
    Vector3 normal = VectorNormalize(VectorCross(edge1, edge2));
    normal = rotatePoint(normal);
    
    // 4頂点を追加
    for (int i = 0; i < 4; ++i) {
        Vector3 rotatedPos = rotatePoint(positions[i]);
        
        VertexData vertex;
        vertex.position = {
            position.x + rotatedPos.x,
            position.y + rotatedPos.y,
            position.z + rotatedPos.z,
            1.0f
        };
        vertex.normal = normal;
        vertex.texcoord = texcoords[i];
        
        unifiedMeshData_.vertices.push_back(vertex);
    }
    
    // 2つの三角形（両面描画のため両方向）
    // 取得した頂点データ
    VertexData v0 = unifiedMeshData_.vertices[baseIndex + 0];
    VertexData v1 = unifiedMeshData_.vertices[baseIndex + 1];
    VertexData v2 = unifiedMeshData_.vertices[baseIndex + 2];
    VertexData v3 = unifiedMeshData_.vertices[baseIndex + 3];
    
    // 表面
    unifiedMeshData_.vertices.push_back(v0);
    unifiedMeshData_.vertices.push_back(v1);
    unifiedMeshData_.vertices.push_back(v2);
    
    unifiedMeshData_.vertices.push_back(v0);
    unifiedMeshData_.vertices.push_back(v2);
    unifiedMeshData_.vertices.push_back(v3);
    
    // 裏面（逆の法線）
    VertexData v0_back = v0; 
    v0_back.normal.x = -v0.normal.x;
    v0_back.normal.y = -v0.normal.y;
    v0_back.normal.z = -v0.normal.z;
    
    VertexData v1_back = v1; 
    v1_back.normal.x = -v1.normal.x;
    v1_back.normal.y = -v1.normal.y;
    v1_back.normal.z = -v1.normal.z;
    
    VertexData v2_back = v2; 
    v2_back.normal.x = -v2.normal.x;
    v2_back.normal.y = -v2.normal.y;
    v2_back.normal.z = -v2.normal.z;
    
    VertexData v3_back = v3; 
    v3_back.normal.x = -v3.normal.x;
    v3_back.normal.y = -v3.normal.y;
    v3_back.normal.z = -v3.normal.z;
    
    unifiedMeshData_.vertices.push_back(v0_back);
    unifiedMeshData_.vertices.push_back(v2_back);
    unifiedMeshData_.vertices.push_back(v1_back);
    
    unifiedMeshData_.vertices.push_back(v0_back);
    unifiedMeshData_.vertices.push_back(v3_back);
    unifiedMeshData_.vertices.push_back(v2_back);
    
    // マテリアル設定（葉の色）
    unifiedMeshData_.material.baseColorFactor = color;
}

Vector3 ProceduralTree::GetRandomDirection(const Vector3& baseDirection, float variance) {
    Vector3 randomOffset = {
        randomDist_(randomEngine_) * variance,
        randomDist_(randomEngine_) * variance,
        randomDist_(randomEngine_) * variance
    };
    
    return VectorNormalize(baseDirection + randomOffset);
}

void ProceduralTree::Update() {
    if (treeObject_) {
        treeObject_->Update();
    }
}

void ProceduralTree::Draw() {
    if (treeObject_) {
        treeObject_->Draw();
    }
}

void ProceduralTree::SetDirectionalLight(const DirectionalLight& light) {
    directionalLight_ = light;
    if (treeObject_) {
        treeObject_->SetDirectionalLight(light);
    }
}

void ProceduralTree::SetSpotLight(const SpotLight& light) {
    spotLight_ = light;
    if (treeObject_) {
        treeObject_->SetSpotLight(light);
    }
}

void ProceduralTree::Clear() {
    treeObject_.reset();
    treeModel_.reset();
    branches_.clear();
    unifiedMeshData_.vertices.clear();
}