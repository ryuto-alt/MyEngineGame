#include "StageEditor.h"
#include "TextureManager.h"
#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "UnoEngine.h"
#include <imgui.h>
#include <Windows.h>

void StageEditor::Initialize(Camera* camera, Input* input, DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    camera_ = camera;
    input_ = input;
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;
    
    // アニメーションパイプラインの初期化
    animatedPipeline_ = std::make_unique<AnimatedRenderingPipeline>();
    animatedPipeline_->Initialize(dxCommon_);
    
    // デフォルトテクスチャを事前にロード
    TextureManager::GetInstance()->LoadTexture("Resources/white.png");
}

void StageEditor::Update() {
    if (!isEnabled_) return;
    
    HandleInput();
}

void StageEditor::HandleInput() {
    if (!isEnabled_) return;
    
    if (input_->PushKey(DIK_LCONTROL) && input_->TriggerKey(DIK_Z)) {
        Undo();
    }
    
    if (input_->PushKey(DIK_LCONTROL) && input_->TriggerKey(DIK_Y)) {
        Redo();
    }
    
    // マウスの状態を取得
    DIMOUSESTATE mouseState;
    if (input_->GetMouseState(&mouseState) == S_OK && mouseState.rgbButtons[0] & 0x80) {
        if (!isDragging_) {
            int index = GetObjectUnderMouse();
            if (index >= 0) {
                selectedObjectIndex_ = index;
                isDragging_ = true;
                Vector3 mousePos = GetMouseWorldPosition();
                dragOffset_.x = gameObjects_[index]->position.x - mousePos.x;
                dragOffset_.y = gameObjects_[index]->position.y - mousePos.y;
                dragOffset_.z = gameObjects_[index]->position.z - mousePos.z;
            }
        } else if (selectedObjectIndex_ >= 0) {
            Vector3 mousePos = GetMouseWorldPosition();
            Vector3 newPos;
            newPos.x = mousePos.x + dragOffset_.x;
            newPos.y = mousePos.y + dragOffset_.y;
            newPos.z = mousePos.z + dragOffset_.z;
            
            if (showGrid_) {
                newPos.x = std::round(newPos.x / gridSize_) * gridSize_;
                newPos.y = std::round(newPos.y / gridSize_) * gridSize_;
                newPos.z = std::round(newPos.z / gridSize_) * gridSize_;
            }
            
            GameObject& obj = *gameObjects_[selectedObjectIndex_];
            // prevStateの保存はコマンド登録時に行う
            obj.position = newPos;
            
            if (obj.isAnimated && obj.animatedObject) {
                obj.animatedObject->SetPosition(newPos);
            } else if (obj.object) {
                obj.object->SetPosition(newPos);
            }
        }
    } else {
        if (isDragging_ && selectedObjectIndex_ >= 0) {
            isDragging_ = false;
        }
    }
    
    if (input_->TriggerKey(DIK_DELETE) && selectedObjectIndex_ >= 0) {
        DeleteObject(selectedObjectIndex_);
        selectedObjectIndex_ = -1;
    }
}

void StageEditor::AddObject(const std::string& modelPath) {
    auto newObj = std::make_unique<GameObject>();
    
    // ファイル名から拡張子を除いたものを名前に使用
    size_t lastSlash = modelPath.find_last_of("/\\");
    size_t lastDot = modelPath.find_last_of(".");
    std::string baseName = modelPath.substr(lastSlash + 1, lastDot - lastSlash - 1);
    newObj->name = baseName + "_" + std::to_string(gameObjects_.size());
    newObj->modelPath = modelPath;
    
    // FBXファイルかOBJファイルかチェック
    if (modelPath.find(".fbx") != std::string::npos || modelPath.find(".FBX") != std::string::npos) {
        // FBXファイルの場合
        newObj->isAnimated = true;
        newObj->animatedObject = std::make_unique<AnimatedObject3d>();
        newObj->animatedObject->Initialize(dxCommon_, spriteCommon_);
        
        // FBXモデルの読み込み
        newObj->fbxModel = std::make_shared<FBXModel>();
        newObj->fbxModel->Initialize(dxCommon_);
        
        if (newObj->fbxModel->LoadFromFile(modelPath)) {
            OutputDebugStringA(("StageEditor: FBX model loaded successfully: " + modelPath + "\n").c_str());
            newObj->animatedObject->SetFBXModel(newObj->fbxModel);
            
            // テクスチャを事前にロード
            const auto& materials = newObj->fbxModel->GetMaterials();
            OutputDebugStringA(("StageEditor: Material count: " + std::to_string(materials.size()) + "\n").c_str());
            for (const auto& material : materials) {
                if (!material.diffuseTexture.empty()) {
                    OutputDebugStringA(("StageEditor: Loading texture: " + material.diffuseTexture + "\n").c_str());
                    TextureManager::GetInstance()->LoadTexture(material.diffuseTexture);
                }
            }
            
            // アニメーションがあれば自動再生
            newObj->animatedObject->PlayAnimation("Walk", true);
        } else {
            OutputDebugStringA(("StageEditor: Failed to load FBX model: " + modelPath + "\n").c_str());
        }
        
        newObj->animatedObject->SetPosition(newObj->position);
        // Spiderモデルはデフォルトサイズのまま
        newObj->animatedObject->SetScale(newObj->scale);
        newObj->animatedObject->SetRotation(newObj->rotation);
        newObj->animatedObject->SetCamera(camera_);
        
    } else {
        // OBJファイルの場合
        newObj->isAnimated = false;
        newObj->object = std::make_unique<Object3d>();
        newObj->object->Initialize(dxCommon_, spriteCommon_);
        
        // Modelの作成と読み込み
        Model* model = new Model();
        model->Initialize(dxCommon_);
        
        // ファイルパスからディレクトリとファイル名を分離
        size_t lastSlash = modelPath.find_last_of("/");
        std::string directory = modelPath.substr(0, lastSlash + 1);
        std::string filename = modelPath.substr(lastSlash + 1);
        
        model->LoadFromObj(directory, filename);
        newObj->object->SetModel(model);
        
        newObj->object->SetPosition(newObj->position);
        newObj->object->SetScale(newObj->scale);
        newObj->object->SetRotation(newObj->rotation);
        newObj->object->SetCamera(camera_);
    }
    
    newObj->hasCollision = newObjectHasCollision_;
    newObj->hasGravity = newObjectHasGravity_;
    
    size_t index = gameObjects_.size();
    gameObjects_.push_back(std::move(newObj));
    
    auto cmd = std::make_unique<EditorCommand>();
    cmd->action = EditorAction::Add;
    cmd->index = index;
    PushCommand(std::move(cmd));
}

void StageEditor::DeleteObject(size_t index) {
    if (index >= gameObjects_.size()) return;
    
    auto cmd = std::make_unique<EditorCommand>();
    cmd->action = EditorAction::Delete;
    cmd->index = index;
    // GameObjectをunique_ptrに移動
    cmd->gameObject = std::move(gameObjects_[index]);
    
    gameObjects_.erase(gameObjects_.begin() + index);
    PushCommand(std::move(cmd));
}

void StageEditor::TransformObject(size_t index, const Vector3& position, const Vector3& rotation, const Vector3& scale) {
    if (index >= gameObjects_.size()) return;
    
    GameObject& obj = *gameObjects_[index];
    
    auto cmd = std::make_unique<EditorCommand>();
    cmd->action = EditorAction::Transform;
    cmd->index = index;
    
    // 変更前の状態を保存
    cmd->previousState.position = obj.position;
    cmd->previousState.rotation = obj.rotation;
    cmd->previousState.scale = obj.scale;
    cmd->previousState.name = obj.name;
    cmd->previousState.hasCollision = obj.hasCollision;
    cmd->previousState.hasGravity = obj.hasGravity;
    
    // 変更後の状態を保存
    cmd->newState.position = position;
    cmd->newState.rotation = rotation;
    cmd->newState.scale = scale;
    cmd->newState.name = obj.name;
    cmd->newState.hasCollision = obj.hasCollision;
    cmd->newState.hasGravity = obj.hasGravity;
    
    // 実際に変更
    obj.position = position;
    obj.rotation = rotation;
    obj.scale = scale;
    
    if (obj.isAnimated && obj.animatedObject) {
        obj.animatedObject->SetPosition(position);
        obj.animatedObject->SetRotation(rotation);
        obj.animatedObject->SetScale(scale);
    } else if (obj.object) {
        obj.object->SetPosition(position);
        obj.object->SetRotation(rotation);
        obj.object->SetScale(scale);
    }
    
    PushCommand(std::move(cmd));
}

void StageEditor::Undo() {
    if (undoStack_.empty()) return;
    
    auto cmd = std::move(undoStack_.top());
    undoStack_.pop();
    
    switch (cmd->action) {
        case EditorAction::Add:
            gameObjects_.erase(gameObjects_.begin() + cmd->index);
            break;
        case EditorAction::Delete:
            if (cmd->gameObject) {
                gameObjects_.insert(gameObjects_.begin() + cmd->index, std::move(cmd->gameObject));
            }
            break;
        case EditorAction::Transform:
            if (cmd->index < gameObjects_.size()) {
                GameObject& obj = *gameObjects_[cmd->index];
                obj.position = cmd->previousState.position;
                obj.rotation = cmd->previousState.rotation;
                obj.scale = cmd->previousState.scale;
                if (obj.isAnimated && obj.animatedObject) {
                    obj.animatedObject->SetPosition(cmd->previousState.position);
                    obj.animatedObject->SetRotation(cmd->previousState.rotation);
                    obj.animatedObject->SetScale(cmd->previousState.scale);
                } else if (obj.object) {
                    obj.object->SetPosition(cmd->previousState.position);
                    obj.object->SetRotation(cmd->previousState.rotation);
                    obj.object->SetScale(cmd->previousState.scale);
                }
            }
            break;
    }
    
    redoStack_.push(std::move(cmd));
}

void StageEditor::Redo() {
    if (redoStack_.empty()) return;
    
    auto cmd = std::move(redoStack_.top());
    redoStack_.pop();
    
    switch (cmd->action) {
        case EditorAction::Add:
            // AddのRedoは追加操作を再実行
            // TODO: 実装
            break;
        case EditorAction::Delete:
            if (cmd->index < gameObjects_.size()) {
                cmd->gameObject = std::move(gameObjects_[cmd->index]);
                gameObjects_.erase(gameObjects_.begin() + cmd->index);
            }
            break;
        case EditorAction::Transform:
            if (cmd->index < gameObjects_.size()) {
                GameObject& obj = *gameObjects_[cmd->index];
                obj.position = cmd->newState.position;
                obj.rotation = cmd->newState.rotation;
                obj.scale = cmd->newState.scale;
                if (obj.isAnimated && obj.animatedObject) {
                    obj.animatedObject->SetPosition(cmd->newState.position);
                    obj.animatedObject->SetRotation(cmd->newState.rotation);
                    obj.animatedObject->SetScale(cmd->newState.scale);
                } else if (obj.object) {
                    obj.object->SetPosition(cmd->newState.position);
                    obj.object->SetRotation(cmd->newState.rotation);
                    obj.object->SetScale(cmd->newState.scale);
                }
            }

            break;
    }
    
    undoStack_.push(std::move(cmd));
}

void StageEditor::PushCommand(std::unique_ptr<EditorCommand> command) {
    undoStack_.push(std::move(command));
    while (!redoStack_.empty()) {
        redoStack_.pop();
    }
}

Vector3 StageEditor::GetMouseWorldPosition() {
    POINT mousePos;
    GetCursorPos(&mousePos);
    ScreenToClient(FindWindowA(nullptr, "DirectX"), &mousePos);
    
    float x = (float)mousePos.x / 640.0f * 2.0f - 1.0f;
    float y = 1.0f - (float)mousePos.y / 480.0f * 2.0f;
    
    Vector3 worldPos;
    worldPos.x = x * 10.0f;
    worldPos.y = 0.0f;
    worldPos.z = y * 10.0f;
    return worldPos;
}

int StageEditor::GetObjectUnderMouse() {
    Vector3 mousePos = GetMouseWorldPosition();
    
    for (size_t i = 0; i < gameObjects_.size(); ++i) {
        Vector3 objPos = gameObjects_[i]->position;
        Vector3 diff;
        diff.x = objPos.x - mousePos.x;
        diff.y = objPos.y - mousePos.y;
        diff.z = objPos.z - mousePos.z;
        float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
        
        if (distance < 2.0f) {
            return static_cast<int>(i);
        }
    }
    
    return -1;
}

void StageEditor::DrawImGui() {
    if (!isEnabled_) return;
    
    ImGui::Begin("Stage Editor");
    
    ImGui::Text("Stage Editor is ACTIVE");
    
    ImGui::Text("F1: Toggle Editor");
    ImGui::Text("Ctrl+Z: Undo, Ctrl+Y: Redo");
    ImGui::Text("Delete: Delete selected object");
    ImGui::Separator();
    
    ImGui::Checkbox("Show Grid", &showGrid_);
    ImGui::DragFloat("Grid Size", &gridSize_, 0.1f, 0.1f, 10.0f);
    
    ImGui::Separator();
    ImGui::Text("New Object Settings:");
    ImGui::Checkbox("Has Collision", &newObjectHasCollision_);
    ImGui::Checkbox("Has Gravity", &newObjectHasGravity_);
    
    ImGui::Combo("Model", &selectedModelIndex_, 
        [](void* data, int idx, const char** out_text) {
            auto models = (std::vector<std::string>*)data;
            *out_text = (*models)[idx].c_str();
            return true;
        }, &availableModels_, static_cast<int>(availableModels_.size()));
    
    if (ImGui::Button("Add Object")) {
        AddObject(availableModels_[selectedModelIndex_]);
    }
    
    ImGui::Separator();
    ImGui::Text("Objects:");
    
    for (size_t i = 0; i < gameObjects_.size(); ++i) {
        GameObject& obj = *gameObjects_[i];
        
        if (ImGui::TreeNode((obj.name + "##" + std::to_string(i)).c_str())) {
            bool changed = false;
            
            changed |= ImGui::DragFloat3("Position", &obj.position.x, 0.1f);
            changed |= ImGui::DragFloat3("Rotation", &obj.rotation.x, 0.1f);
            changed |= ImGui::DragFloat3("Scale", &obj.scale.x, 0.1f);
            
            ImGui::Checkbox("Collision", &obj.hasCollision);
            ImGui::Checkbox("Gravity", &obj.hasGravity);
            
            if (obj.isAnimated) {
                ImGui::Text("Type: Animated (FBX)");
                if (obj.animatedObject && obj.animatedObject->IsAnimationPlaying()) {
                    ImGui::Text("Animation: Playing");
                } else {
                    ImGui::Text("Animation: Stopped");
                }
            } else {
                ImGui::Text("Type: Static (OBJ)");
            }
            
            if (changed) {
                if (obj.isAnimated && obj.animatedObject) {
                    obj.animatedObject->SetPosition(obj.position);
                    obj.animatedObject->SetRotation(obj.rotation);
                    obj.animatedObject->SetScale(obj.scale);
                } else if (obj.object) {
                    obj.object->SetPosition(obj.position);
                    obj.object->SetRotation(obj.rotation);
                    obj.object->SetScale(obj.scale);
                }
            }
            
            if (ImGui::Button(("Delete##" + std::to_string(i)).c_str())) {
                DeleteObject(i);
                if (selectedObjectIndex_ == static_cast<int>(i)) {
                    selectedObjectIndex_ = -1;
                }
            }
            
            ImGui::TreePop();
        }
    }
    
    ImGui::End();
}

void StageEditor::DrawObjects() {
    // まず通常のオブジェクトを描画
    for (auto& obj : gameObjects_) {
        if (!obj->isAnimated && obj->object) {
            // カメラを設定
            obj->object->SetCamera(camera_);
            // Update()を呼んで変換行列を更新
            obj->object->Update();
            // 描画
            obj->object->Draw();
        }
    }
    
    // アニメーションオブジェクトは専用パイプラインで描画
    bool hasAnimatedObjects = false;
    for (auto& obj : gameObjects_) {
        if (obj->isAnimated) {
            hasAnimatedObjects = true;
            break;
        }
    }
    
    if (hasAnimatedObjects && animatedPipeline_) {
        OutputDebugStringA("StageEditor: Drawing animated objects\n");
        animatedPipeline_->Bind();
        
        for (auto& obj : gameObjects_) {
            if (obj->isAnimated && obj->animatedObject) {
                OutputDebugStringA(("StageEditor: Drawing animated object: " + obj->name + "\n").c_str());
                // カメラを設定
                obj->animatedObject->SetCamera(camera_);
                // Update()を呼んで変換行列を更新
                obj->animatedObject->Update();
                // 描画
                obj->animatedObject->Draw();
            }
        }
    }
}