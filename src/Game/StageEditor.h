#pragma once
#include "Model.h"
#include "Object3d.h"
#include "Camera.h"
#include "Input.h"
#include <vector>
#include <memory>
#include <stack>
#include <string>

struct GameObject {
    std::unique_ptr<Object3d> object;
    std::string name;
    bool hasCollision = true;
    bool hasGravity = true;
    Vector3 position = {0.0f, 0.0f, 0.0f};
    Vector3 rotation = {0.0f, 0.0f, 0.0f};
    Vector3 scale = {1.0f, 1.0f, 1.0f};
    
    // デフォルトコンストラクタ
    GameObject() = default;
    
    // ムーブコンストラクタ
    GameObject(GameObject&& other) noexcept = default;
    
    // ムーブ代入演算子
    GameObject& operator=(GameObject&& other) noexcept = default;
    
    // コピーコンストラクタ（削除）
    GameObject(const GameObject&) = delete;
    
    // コピー代入演算子（削除）
    GameObject& operator=(const GameObject&) = delete;
};

enum class EditorAction {
    Add,
    Delete,
    Transform
};

struct EditorCommand {
    EditorAction action{EditorAction::Add};
    size_t index{0};
    
    // Transform用の状態保存
    struct TransformState {
        Vector3 position{0.0f, 0.0f, 0.0f};
        Vector3 rotation{0.0f, 0.0f, 0.0f};
        Vector3 scale{1.0f, 1.0f, 1.0f};
        std::string name{};
        bool hasCollision{true};
        bool hasGravity{true};
        
        // デフォルトコンストラクタ
        TransformState() = default;
    };
    
    // アクションに応じて使用するデータ
    std::unique_ptr<GameObject> gameObject{};  // Delete用
    TransformState previousState{};            // Transform用
    TransformState newState{};                 // Transform用
    
    // デフォルトコンストラクタ
    EditorCommand() = default;
    
    // ムーブコンストラクタ
    EditorCommand(EditorCommand&& other) noexcept = default;
    
    // ムーブ代入演算子
    EditorCommand& operator=(EditorCommand&& other) noexcept = default;
    
    // コピーコンストラクタ（削除）
    EditorCommand(const EditorCommand&) = delete;
    
    // コピー代入演算子（削除）
    EditorCommand& operator=(const EditorCommand&) = delete;
};

class DirectXCommon;
class SpriteCommon;

class StageEditor {
public:
    void Initialize(Camera* camera, Input* input, DirectXCommon* dxCommon, SpriteCommon* spriteCommon);
    void Update();
    void DrawImGui();
    void DrawObjects();
    
    bool IsEnabled() const { return isEnabled_; }
    void SetEnabled(bool enabled) { isEnabled_ = enabled; }
    
    const std::vector<std::unique_ptr<GameObject>>& GetGameObjects() const { return gameObjects_; }
    
private:
    void HandleInput();
    void AddObject(const std::string& modelPath);
    void DeleteObject(size_t index);
    void TransformObject(size_t index, const Vector3& position, const Vector3& rotation, const Vector3& scale);
    
    void Undo();
    void Redo();
    void PushCommand(std::unique_ptr<EditorCommand> command);
    
    Vector3 GetMouseWorldPosition();
    int GetObjectUnderMouse();
    
private:
    Camera* camera_ = nullptr;
    Input* input_ = nullptr;
    DirectXCommon* dxCommon_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;
    
    bool isEnabled_ = false;
    std::vector<std::unique_ptr<GameObject>> gameObjects_;
    
    std::stack<std::unique_ptr<EditorCommand>> undoStack_;
    std::stack<std::unique_ptr<EditorCommand>> redoStack_;
    
    int selectedObjectIndex_ = -1;
    bool isDragging_ = false;
    Vector3 dragOffset_;
    
    std::vector<std::string> availableModels_ = {
        "Resources/Models/cube/cube.obj",
        "Resources/Models/sphere.obj",
        "Resources/Models/Cylinder/Cylinder.obj",
        "Resources/06_02/plane.obj"
    };
    
    int selectedModelIndex_ = 0;
    bool showGrid_ = true;
    float gridSize_ = 1.0f;
    
    enum class TransformMode {
        Translate,
        Rotate,
        Scale
    };
    TransformMode transformMode_ = TransformMode::Translate;
    
    bool newObjectHasCollision_ = true;
    bool newObjectHasGravity_ = true;
};