#include "MazeRenderer.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "SpriteCommon.h"
#include "Mymath.h"
#include <algorithm>
#include <cmath>
#include <iostream>

MazeRenderer::MazeRenderer()
    : dxCommon_(nullptr)
    , spriteCommon_(nullptr)
    , currentGrid_(nullptr)
    , fogEnabled_(true)
    , fogColor_(0.1f, 0.1f, 0.15f, 1.0f)
    , fogDensity_(0.05f)
    , fogStart_(5.0f)
    , fogEnd_(30.0f)
    , ambientLight_(0.1f, 0.1f, 0.15f)
    , flickerLightEnabled_(true)
    , flickerTimer_(0.0f)
    , flickerIntensity_(1.0f)
    , viewDistance_(50.0f)
    , frustumCullingEnabled_(true)
    , debugDrawEnabled_(false) {
    
    statistics_ = {};
}

MazeRenderer::~MazeRenderer() {
}

void MazeRenderer::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;
    
    // パーツモデルを読み込み
    LoadMazePieces();
}

void MazeRenderer::LoadMazePieces() {
    // 各種迷路パーツのモデルを読み込み
    std::unordered_map<MazePieceType, std::string> pieceFiles = {
        {MazePieceType::Floor, "Resources/Models/maze/floor_basic.glb"},
        {MazePieceType::WallStraight, "Resources/Models/maze/wall_straight.glb"},
        {MazePieceType::WallCornerInner, "Resources/Models/maze/wall_corner_inner.glb"},
        {MazePieceType::WallTJunction, "Resources/Models/maze/wall_t_junction.glb"},
        {MazePieceType::WallCross, "Resources/Models/maze/wall_cross.glb"},
        {MazePieceType::Light, "Resources/Models/maze/light_flickering.glb"}
    };
    
    for (const auto& [type, filepath] : pieceFiles) {
        auto model = std::make_unique<Model>();
        
        // ファイルが存在しない場合は仮のキューブモデルを使用
        if (!model->LoadFromGLB(filepath)) {
            // フォールバック: デフォルトのキューブモデルを使用
            model->LoadFromGLB("Resources/Models/cube/obje.glb");
        }
        
        pieceModels_[type] = std::move(model);
    }
}

void MazeRenderer::BuildMaze(MazeGrid* grid) {
    if (!grid || !dxCommon_ || !spriteCommon_) return;
    
    currentGrid_ = grid;
    pieceInstances_.clear();
    
    // グリッドの各セルに対してパーツを配置
    grid->ForEachCell([this, grid](int x, int y, MazeCell* cell) {
        if (!cell) return;
        
        Vector3 worldPos = grid->GridToWorld(x, y);
        
        // 床の配置
        if (cell->GetType() != MazeCellType::Wall && cell->GetType() != MazeCellType::Empty) {
            CreatePieceInstance(
                MazePieceType::Floor,
                worldPos,
                0.0f,
                cell->GetDarkness(),
                cell->GetFogDensity()
            );
            
            // ライトの配置（一部の通路にランダムに）
            if (x % 5 == 0 && y % 5 == 0) {
                Vector3 lightPos = worldPos;
                lightPos.y += 2.8f; // 天井付近に配置
                CreatePieceInstance(
                    MazePieceType::Light,
                    lightPos,
                    0.0f,
                    0.0f,
                    0.0f
                );
            }
        }
        
        // 壁の配置
        if (cell->GetType() == MazeCellType::Wall || cell->GetType() == MazeCellType::Empty) {
            MazePieceType pieceType = DeterminePieceType(grid, x, y);
            float rotation = DeterminePieceRotation(grid, x, y, pieceType);
            
            if (pieceType != MazePieceType::Floor) {
                CreatePieceInstance(
                    pieceType,
                    worldPos,
                    rotation,
                    cell->GetDarkness(),
                    cell->GetFogDensity()
                );
            }
        }
    });
    
    // 統計情報を更新
    statistics_.totalPieces = static_cast<int>(pieceInstances_.size());
}

void MazeRenderer::Update(Camera* camera) {
    if (!camera) return;
    
    statistics_.visiblePieces = 0;
    statistics_.culledPieces = 0;
    
    Vector3 cameraPos = camera->GetTranslate();
    
    // 各パーツインスタンスを更新
    for (auto& instance : pieceInstances_) {
        // 視界距離チェック
        if (frustumCullingEnabled_) {
            instance.isVisible = IsInViewDistance(instance.position, camera);
            
            if (instance.isVisible) {
                instance.isVisible = IsInFrustum(instance.position, camera);
            }
        } else {
            instance.isVisible = true;
        }
        
        if (instance.isVisible) {
            // Object3dを更新
            instance.object->Update();
            
            // セルごとのエフェクトを適用
            ApplyCellEffects(instance);
            
            statistics_.visiblePieces++;
        } else {
            statistics_.culledPieces++;
        }
    }
}

void MazeRenderer::Draw() {
    // 可視パーツのみ描画（インスタンシングで効率化）
    std::unordered_map<MazePieceType, std::vector<PieceInstance*>> instancesByType;
    
    // タイプごとにインスタンスをグループ化
    for (auto& instance : pieceInstances_) {
        if (instance.isVisible) {
            instancesByType[instance.type].push_back(&instance);
        }
    }
    
    // タイプごとにまとめて描画
    statistics_.instancedDrawCalls = 0;
    for (const auto& [type, instances] : instancesByType) {
        for (auto* instance : instances) {
            instance->object->Draw();
        }
        statistics_.instancedDrawCalls++;
    }
}

void MazeRenderer::UpdateDarkDeceptionEffects(float deltaTime) {
    // ライトのちらつき効果
    if (flickerLightEnabled_) {
        UpdateFlickerLights(deltaTime);
    }
    
    // フォグの動的変化（オプション）
    if (fogEnabled_) {
        // 時間経過でフォグ濃度を微妙に変化させる
        float fogVariation = std::sin(flickerTimer_ * 0.5f) * 0.01f;
        fogDensity_ = std::max(0.01f, fogDensity_ + fogVariation);
    }
}

MazePieceType MazeRenderer::DeterminePieceType(MazeGrid* grid, int x, int y) {
    MazeCell* cell = grid->GetCell(x, y);
    if (!cell) return MazePieceType::Floor;
    
    // 通路の場合は床
    if (cell->GetType() != MazeCellType::Wall && cell->GetType() != MazeCellType::Empty) {
        return MazePieceType::Floor;
    }
    
    // 壁の場合は周囲の接続状態から判定
    int connections = CountConnections(grid, x, y);
    
    switch (connections) {
        case 0:
            return MazePieceType::WallCross; // 独立した柱
        case 1:
            return MazePieceType::WallDeadEnd; // 行き止まり
        case 2:
            // 直線かコーナーか判定
            if ((HasNorthConnection(grid, x, y) && HasSouthConnection(grid, x, y)) ||
                (HasEastConnection(grid, x, y) && HasWestConnection(grid, x, y))) {
                return MazePieceType::WallStraight;
            } else {
                return MazePieceType::WallCornerInner;
            }
        case 3:
            return MazePieceType::WallTJunction; // T字路
        case 4:
            return MazePieceType::WallCross; // 十字路
        default:
            return MazePieceType::WallStraight;
    }
}

float MazeRenderer::DeterminePieceRotation(MazeGrid* grid, int x, int y, MazePieceType type) {
    float rotation = 0.0f;
    
    switch (type) {
        case MazePieceType::WallStraight:
            // 南北に接続があれば0度、東西なら90度
            if (HasEastConnection(grid, x, y) && HasWestConnection(grid, x, y)) {
                rotation = 90.0f;
            }
            break;
            
        case MazePieceType::WallCornerInner:
            // コーナーの向きを判定
            if (HasNorthConnection(grid, x, y) && HasEastConnection(grid, x, y)) {
                rotation = 0.0f;
            } else if (HasEastConnection(grid, x, y) && HasSouthConnection(grid, x, y)) {
                rotation = 90.0f;
            } else if (HasSouthConnection(grid, x, y) && HasWestConnection(grid, x, y)) {
                rotation = 180.0f;
            } else if (HasWestConnection(grid, x, y) && HasNorthConnection(grid, x, y)) {
                rotation = 270.0f;
            }
            break;
            
        case MazePieceType::WallTJunction:
            // T字の向きを判定
            if (!HasNorthConnection(grid, x, y)) {
                rotation = 0.0f;
            } else if (!HasEastConnection(grid, x, y)) {
                rotation = 90.0f;
            } else if (!HasSouthConnection(grid, x, y)) {
                rotation = 180.0f;
            } else if (!HasWestConnection(grid, x, y)) {
                rotation = 270.0f;
            }
            break;
            
        default:
            break;
    }
    
    return rotation * (3.14159265f / 180.0f); // ラジアンに変換
}

void MazeRenderer::CreatePieceInstance(MazePieceType type, const Vector3& position, 
                                       float rotation, float darkness, float fogDensity) {
    if (pieceModels_.find(type) == pieceModels_.end()) {
        return; // モデルが読み込まれていない
    }
    
    PieceInstance instance;
    instance.type = type;
    instance.position = position;
    instance.rotation = rotation;
    instance.darkness = darkness;
    instance.fogDensity = fogDensity;
    instance.isVisible = true;
    
    // Object3dを作成
    instance.object = std::make_unique<Object3d>();
    instance.object->Initialize(dxCommon_, spriteCommon_);
    instance.object->SetModel(pieceModels_[type].get());
    instance.object->SetPosition(position);
    instance.object->SetRotation(Vector3(0, rotation, 0));
    
    // Dark Deception風の暗い色調を設定
    Vector4 baseColor(0.3f - darkness * 0.2f, 
                     0.3f - darkness * 0.2f, 
                     0.35f - darkness * 0.15f, 
                     1.0f);
    instance.object->SetColor(baseColor);
    
    // ライティング設定
    instance.object->SetEnableLighting(true);
    
    DirectionalLight dirLight;
    dirLight.color = Vector4(0.2f, 0.2f, 0.25f, 1.0f);
    dirLight.direction = Vector3Normalize(Vector3(0.0f, -1.0f, 0.3f));
    dirLight.intensity = 0.5f;
    instance.object->SetDirectionalLight(dirLight);
    
    // スポットライト（懐中電灯風）
    if (type == MazePieceType::Light) {
        SpotLight spotLight;
        spotLight.color = Vector4(1.0f, 0.9f, 0.7f, 1.0f);
        spotLight.position = position;
        spotLight.direction = Vector3(0, -1, 0);
        spotLight.intensity = flickerIntensity_;
        spotLight.innerCone = std::cos(35.0f * 3.14159265f / 180.0f);
        spotLight.outerCone = std::cos(45.0f * 3.14159265f / 180.0f);
        spotLight.attenuation = Vector3(1.0f, 0.09f, 0.032f);
        instance.object->SetSpotLight(spotLight);
    }
    
    pieceInstances_.push_back(std::move(instance));
}

bool MazeRenderer::IsInViewDistance(const Vector3& position, Camera* camera) {
    Vector3 cameraPos = camera->GetTranslate();
    float distance = Vector3Length(position - cameraPos);
    return distance <= viewDistance_;
}

bool MazeRenderer::IsInFrustum(const Vector3& position, Camera* camera) {
    // 簡易的なフラスタムカリング
    // TODO: 適切なフラスタムカリングの実装
    return IsInViewDistance(position, camera);
}

void MazeRenderer::UpdateFlickerLights(float deltaTime) {
    flickerTimer_ += deltaTime;
    
    // ライトの明るさをランダムに変化させる
    float flicker = std::sin(flickerTimer_ * 15.0f) * 0.1f +
                   std::sin(flickerTimer_ * 23.0f) * 0.05f +
                   std::sin(flickerTimer_ * 37.0f) * 0.03f;
    
    flickerIntensity_ = (std::max)(0.3f, (std::min)(1.0f, 0.8f + flicker));
    
    // ライトインスタンスの更新
    for (auto& instance : pieceInstances_) {
        if (instance.type == MazePieceType::Light && instance.isVisible) {
            SpotLight spotLight = instance.object->GetSpotLight();
            spotLight.intensity = flickerIntensity_;
            instance.object->SetSpotLight(spotLight);
        }
    }
}

void MazeRenderer::ApplyCellEffects(PieceInstance& instance) {
    // セル固有の暗さを適用
    if (instance.darkness > 0.0f) {
        Vector4 color = instance.object->GetColor();
        float darknessFactor = 1.0f - instance.darkness;
        color.x *= darknessFactor;
        color.y *= darknessFactor;
        color.z *= darknessFactor;
        instance.object->SetColor(color);
    }
    
    // セル固有のフォグ濃度を適用（将来的な拡張用）
    // TODO: フォグエフェクトの実装
}

int MazeRenderer::CountConnections(MazeGrid* grid, int x, int y) {
    int count = 0;
    if (HasNorthConnection(grid, x, y)) count++;
    if (HasEastConnection(grid, x, y)) count++;
    if (HasSouthConnection(grid, x, y)) count++;
    if (HasWestConnection(grid, x, y)) count++;
    return count;
}

bool MazeRenderer::HasNorthConnection(MazeGrid* grid, int x, int y) {
    MazeCell* neighbor = grid->GetNeighbor(x, y, Direction::North);
    return neighbor && (neighbor->GetType() == MazeCellType::Floor || 
                        neighbor->GetType() == MazeCellType::Start ||
                        neighbor->GetType() == MazeCellType::Goal);
}

bool MazeRenderer::HasEastConnection(MazeGrid* grid, int x, int y) {
    MazeCell* neighbor = grid->GetNeighbor(x, y, Direction::East);
    return neighbor && (neighbor->GetType() == MazeCellType::Floor || 
                        neighbor->GetType() == MazeCellType::Start ||
                        neighbor->GetType() == MazeCellType::Goal);
}

bool MazeRenderer::HasSouthConnection(MazeGrid* grid, int x, int y) {
    MazeCell* neighbor = grid->GetNeighbor(x, y, Direction::South);
    return neighbor && (neighbor->GetType() == MazeCellType::Floor || 
                        neighbor->GetType() == MazeCellType::Start ||
                        neighbor->GetType() == MazeCellType::Goal);
}

bool MazeRenderer::HasWestConnection(MazeGrid* grid, int x, int y) {
    MazeCell* neighbor = grid->GetNeighbor(x, y, Direction::West);
    return neighbor && (neighbor->GetType() == MazeCellType::Floor || 
                        neighbor->GetType() == MazeCellType::Start ||
                        neighbor->GetType() == MazeCellType::Goal);
}