#include "StagePart.h"
#include "../Graphics/Model.h"
#include "../Graphics/Object3d.h"
#include "../Graphics/DirectXCommon.h"
#include "../Graphics/SpriteCommon.h"

StagePart::StagePart(StagePartType type) 
    : type_(type), gridX_(0), gridY_(0), rotation_(0), worldPosition_{0, 0, 0} {
    // デフォルトですべての方向に接続可能
    connectableDirections_ = { true, true, true, true };
}

StagePart::~StagePart() = default;

void StagePart::Initialize(const std::string& modelPath) {
    // モデルをロード
    model_ = std::make_shared<Model>();
    
    // 3Dオブジェクトを作成（DirectXCommonとSpriteCommonは後で設定）
    object3d_ = std::make_unique<Object3d>();
    
    // モデルのパスを保存（InitializeGraphicsで使用）
    modelPath_ = modelPath;
    
    // パーツタイプに応じた接続方向を設定
    UpdateConnectionDirections();
}

void StagePart::InitializeGraphics(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    if (object3d_ && dxCommon && spriteCommon) {
        // Object3dを初期化
        object3d_->Initialize(dxCommon, spriteCommon);
        
        // モデルを初期化してロード
        if (model_ && !modelPath_.empty()) {
            model_->Initialize(dxCommon);
            
            // パスを分解（ディレクトリとファイル名に分離）
            size_t lastSlash = modelPath_.find_last_of("/\\");
            std::string directory = modelPath_.substr(0, lastSlash);
            std::string filename = modelPath_.substr(lastSlash + 1);
            
            model_->LoadFromObj(directory, filename);
            object3d_->SetModel(model_.get());
        }
        
        // 位置と回転を再設定
        object3d_->SetPosition(worldPosition_);
        float radians = rotation_ * (3.14159265f / 180.0f);
        object3d_->SetRotation({ 0, radians, 0 });
    }
}

void StagePart::SetGridPosition(int x, int y) {
    gridX_ = x;
    gridY_ = y;
}

void StagePart::SetWorldPosition(const Vector3& position) {
    worldPosition_ = position;
    if (object3d_) {
        object3d_->SetPosition(position);
    }
}

void StagePart::SetRotation(int rotation) {
    rotation_ = rotation % 360;
    if (rotation_ < 0) rotation_ += 360;
    
    if (object3d_) {
        float radians = rotation_ * (3.14159265f / 180.0f);
        object3d_->SetRotation({ 0, radians, 0 });
    }
}

void StagePart::Update() {
    if (object3d_) {
        object3d_->Update();
    }
}

void StagePart::Draw() {
    if (object3d_) {
        object3d_->Draw();
    }
}

bool StagePart::CanConnect(Direction dir) const {
    Direction rotatedDir = GetRotatedDirection(dir);
    return connectableDirections_[static_cast<int>(rotatedDir)];
}

bool StagePart::IsCompatibleWith(const StagePart* other, Direction dir) const {
    if (!other) return true;  // 空のスペースとは常に互換性あり
    
    // 自分がその方向に接続可能かチェック
    if (!CanConnect(dir)) return false;
    
    // 相手が逆方向に接続可能かチェック
    Direction oppositeDir = static_cast<Direction>((static_cast<int>(dir) + 2) % 4);
    return other->CanConnect(oppositeDir);
}

Direction StagePart::GetRotatedDirection(Direction original) const {
    int rotationSteps = rotation_ / 90;
    int newDir = (static_cast<int>(original) + rotationSteps) % 4;
    return static_cast<Direction>(newDir);
}

void StagePart::UpdateConnectionDirections() {
    // パーツタイプに応じて接続可能な方向を設定
    switch (type_) {
    case StagePartType::FLOOR_BASIC:
    case StagePartType::FLOOR_DAMAGED:
    case StagePartType::FLOOR_SPECIAL:
        // 床は全方向に接続可能
        connectableDirections_ = { true, true, true, true };
        break;
        
    case StagePartType::WALL_STRAIGHT:
        // 直線壁は北と南に接続
        connectableDirections_ = { true, false, true, false };
        break;
        
    case StagePartType::WALL_CORNER:
        // コーナー壁は北と東に接続
        connectableDirections_ = { true, true, false, false };
        break;
        
    case StagePartType::WALL_T_JUNCTION:
        // T字路は北、東、西に接続
        connectableDirections_ = { true, true, false, true };
        break;
        
    case StagePartType::WALL_CROSS:
        // 十字路は全方向に接続
        connectableDirections_ = { true, true, true, true };
        break;
        
    case StagePartType::WALL_DOOR:
        // ドアは北と南に接続
        connectableDirections_ = { true, false, true, false };
        break;
        
    default:
        // その他は全方向に接続可能
        connectableDirections_ = { true, true, true, true };
        break;
    }
}

std::string StagePart::GetModelPath(StagePartType type) {
    switch (type) {
    case StagePartType::FLOOR_BASIC:
        return "Resources/Models/Stage/Floor/floor_basic.obj";
    case StagePartType::FLOOR_DAMAGED:
        // floor_damagedが存在しないため、floor_basicを使用
        return "Resources/Models/Stage/Floor/floor_basic.obj";
    case StagePartType::FLOOR_SPECIAL:
        // floor_specialが存在しないため、floor_basicを使用
        return "Resources/Models/Stage/Floor/floor_basic.obj";
    case StagePartType::WALL_STRAIGHT:
        return "Resources/Models/Stage/Wall/wall_straight.obj";
    case StagePartType::WALL_CORNER:
        return "Resources/Models/Stage/Wall/wall_corner.obj";
    case StagePartType::WALL_T_JUNCTION:
        return "Resources/Models/Stage/Junction/wall_t_junction.obj";
    case StagePartType::WALL_CROSS:
        return "Resources/Models/Stage/Junction/wall_cross_junction.obj";
    case StagePartType::WALL_DOOR:
        return "Resources/Models/Stage/Wall/wall_door.obj";
    case StagePartType::CEILING_BASIC:
        return "Resources/Models/Stage/Ceiling/ceiling_basic.obj";
    case StagePartType::CEILING_LIGHT:
        return "Resources/Models/Stage/Ceiling/ceiling_light.obj";
    case StagePartType::STAIRS_UP:
        return "Resources/Models/Stage/Stairs/stairs_up.obj";
    case StagePartType::STAIRS_DOWN:
        // stairs_downが存在しないため、stairs_upを使用
        return "Resources/Models/Stage/Stairs/stairs_up.obj";
    case StagePartType::PILLAR:
        // pillarが存在しないため、wall_straightを使用
        return "Resources/Models/Stage/Wall/wall_straight.obj";
    case StagePartType::DECORATION:
        // decorationが存在しないため、floor_basicを使用
        return "Resources/Models/Stage/Floor/floor_basic.obj";
    default:
        return "Resources/Models/Stage/Floor/floor_basic.obj";
    }
}

std::string StagePart::GetTypeName(StagePartType type) {
    switch (type) {
    case StagePartType::FLOOR_BASIC: return "Floor Basic";
    case StagePartType::FLOOR_DAMAGED: return "Floor Damaged";
    case StagePartType::FLOOR_SPECIAL: return "Floor Special";
    case StagePartType::WALL_STRAIGHT: return "Wall Straight";
    case StagePartType::WALL_CORNER: return "Wall Corner";
    case StagePartType::WALL_T_JUNCTION: return "Wall T-Junction";
    case StagePartType::WALL_CROSS: return "Wall Cross";
    case StagePartType::WALL_DOOR: return "Wall Door";
    case StagePartType::CEILING_BASIC: return "Ceiling Basic";
    case StagePartType::CEILING_LIGHT: return "Ceiling Light";
    case StagePartType::STAIRS_UP: return "Stairs Up";
    case StagePartType::STAIRS_DOWN: return "Stairs Down";
    case StagePartType::PILLAR: return "Pillar";
    case StagePartType::DECORATION: return "Decoration";
    default: return "Unknown";
    }
}

// FloorPart実装
FloorPart::FloorPart(StagePartType type) : StagePart(type) {
    // 床は全方向に接続可能
    connectableDirections_ = { true, true, true, true };
}

// WallPart実装
WallPart::WallPart(StagePartType type) : StagePart(type) {
    // 壁タイプに応じて接続方向を設定
    UpdateConnectionDirections();
}