#include "CameraManager.h"
#include <cassert>
#include <Logger.h>

CameraManager* CameraManager::instance = nullptr;

CameraManager* CameraManager::GetInstance()
{
    if (!instance) {
        instance = new CameraManager();
    }


    return instance;

}

void CameraManager::Finalize()
{

    delete instance;
    instance = nullptr;
}

void CameraManager::Initialize()
{

    // デフォルトカメラの作成
    defaultCamera = new Camera();
    defaultCamera->SetTranslate({ 0, 0, -5 });
    AddCamera("default", defaultCamera);
    SetActiveCamera("default"); // デフォルトカメラをアクティブカメラとして設定



}

void CameraManager::AddCamera(const std::string& name, const Camera* camera)
{

   // assert(cameras.find(name) == cameras.end() && "Camera with the same name already exists!");

    cameras[name] = *camera; // Dereference the pointer to store the Camera object
    // 最初のカメラをアクティブに設定
    if (activeCameraName.empty()) {
        activeCameraName = name;
    }
}

void CameraManager::RemoveCamera(const std::string& name) {
    if (cameras.erase(name) > 0 && activeCameraName == name) {
        // アクティブカメラが削除された場合、他のカメラをアクティブに設定
        if (!cameras.empty()) {
            activeCameraName = cameras.begin()->first;
        }
        else {
            activeCameraName.clear();
        }
    }
}

Camera* CameraManager::GetCamera(const std::string& name) {
    auto it = cameras.find(name);
    if (it != cameras.end()) {
        return &(it->second);
    }
    return nullptr;
}


Camera* CameraManager::GetActiveCamera() {
    if (activeCameraName.empty() || cameras.find(activeCameraName) == cameras.end()) {
        // アクティブカメラが無効な場合、デフォルトカメラを使用
        SetActiveCamera("default"); // デフォルトカメラをアクティブカメラとして設定
        return defaultCamera;
    }
    return &cameras[activeCameraName];
}

void CameraManager::SetActiveCamera(const std::string& name) {
    if (cameras.find(name) != cameras.end()) {
        activeCameraName = name;
    } else {
        Logger::Log("Warning: Attempted to set an invalid active camera. Using default camera.");
        activeCameraName.clear(); // 無効なカメラを選択した場合、リセット
    }
}

