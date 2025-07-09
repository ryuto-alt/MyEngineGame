#include "CameraManager.h"

#pragma region imgui
#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif 
#pragma endregion

CameraManager* CameraManager::instance_ = nullptr;

CameraManager* CameraManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new CameraManager();
	}
	return instance_;
}

void CameraManager::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
}

void CameraManager::Update() {
	if (activeCamera_) {
		activeCamera_->Update();
	}
}

void CameraManager::UpdateImGui() {

#ifdef _DEBUG
	ImGui::Begin("CameraManager");
	ImGui::Text("Camera Count : %d", cameras_.size());
	ImGui::Separator();
	for (const auto& [name, camera] : cameras_) {
		// ボタンを表示し、押されたらアクティブカメラを設定
		if (ImGui::Button(name.c_str())) {
			SetActiveCamera(name);
		}
	}
	activeCamera_->UpdateImGui();
	ImGui::End();
#endif // _DEBUG
}

void CameraManager::Finalize() {

	cameras_.clear();
	delete instance_;
	instance_ = nullptr;
}

void CameraManager::AddCamera(std::string name, const Camera& camera) {
	//生成済みのカメラの検索
	if (cameras_.contains(name)) {
		return;
	}

	// 新しいカメラを追加
	cameras_.insert(std::make_pair(name, std::make_unique<Camera>(camera)));

	// もしこれが最初のカメラなら、それをアクティブに設定
	if (cameras_.size() == 1) {
		activeCamera_ = cameras_.find(name)->second.get();
	}
}

void CameraManager::ResetCameras() {
	cameras_.clear();
	activeCamera_ = nullptr;
}

void CameraManager::SetActiveCamera(std::string name) {

	auto it = cameras_.find(name);
	// 指定された名前のカメラが見つかった場合は、それをアクティブに設定
	if (it != cameras_.end()) {
		activeCamera_ = it->second.get();
	} else {
		// 見つからなかった場合は、最初のカメラをアクティブに設定
		it = cameras_.begin();
	}
}

Camera* CameraManager::GetActiveCamera() {
	return activeCamera_;
}

int CameraManager::GetCameraCount() const {
	return static_cast<int>(cameras_.size());
}