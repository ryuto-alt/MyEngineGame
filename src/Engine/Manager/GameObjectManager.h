#pragma once
#include "../Core/GameObject.h"
#include "Camera.h"
#include "Mymath.h"
#include <memory>
#include <map>
#include <vector>
#include <string>

class UnoEngine;

class GameObjectManager {
public:
    GameObjectManager() = default;
    ~GameObjectManager() = default;

    template<typename T>
    T* Add(const std::string& name) {
        auto obj = std::make_unique<T>();
        T* ptr = obj.get();
        ptr->SetName(name);
        gameObjects_[name] = std::move(obj);
        return ptr;
    }

    void Remove(const std::string& name) {
        gameObjects_.erase(name);
    }

    GameObject* Get(const std::string& name) {
        auto it = gameObjects_.find(name);
        if (it != gameObjects_.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    void UpdateAll() {
        for (auto& [name, obj] : gameObjects_) {
            if (obj->IsActive()) {
                obj->Update();
            }
        }
    }

    void DrawAll() {
        for (auto& [name, obj] : gameObjects_) {
            if (obj->IsActive()) {
                obj->Draw();
            }
        }
    }

    void FinalizeAll() {
        for (auto& [name, obj] : gameObjects_) {
            obj->Finalize();
        }
        gameObjects_.clear();
    }

    void SetCameraForAll(Camera* camera) {
        for (auto& [name, obj] : gameObjects_) {
            obj->SetCamera(camera);
        }
    }

    void SetEngineForAll(UnoEngine* engine) {
        for (auto& [name, obj] : gameObjects_) {
            obj->SetEngine(engine);
        }
    }

    void SetDirectionalLightForAll(const DirectionalLight& light) {
        for (auto& [name, obj] : gameObjects_) {
            obj->SetDirectionalLight(light);
        }
    }

    void SetSpotLightForAll(const SpotLight& light) {
        for (auto& [name, obj] : gameObjects_) {
            obj->SetSpotLight(light);
        }
    }

    std::map<std::string, std::unique_ptr<GameObject>>& GetAll() {
        return gameObjects_;
    }

private:
    std::map<std::string, std::unique_ptr<GameObject>> gameObjects_;
};