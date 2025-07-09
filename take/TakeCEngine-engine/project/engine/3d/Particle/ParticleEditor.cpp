#include "ParticleEditor.h"
#include "base/ImGuiManager.h"
#include "base/TakeCFrameWork.h"
#include "base/TextureManager.h"
#include <cassert>

//======================================================================
//			初期化
//======================================================================

void ParticleEditor::Initialize(ParticleManager* particleManager,ParticleCommon* particleCommon) {

	// ParticleManager初期化
	particleManager_ = particleManager;
	particleCommon_ = particleCommon;
	//デフォルトのパーティクルグループを作成
	currentGroupName_ = "DefaultGroup";
	currentPreset_.textureFilePath = "white1x1.png";
	currentPreset_.presetName = currentGroupName_;
	currentPreset_.primitiveType = PRIMITIVE_PLANE;
	particleManager_->CreateParticleGroup(particleCommon_,currentGroupName_, "white1x1.png", currentPreset_.primitiveType);

	// エディター専用エミッターの初期化
	previewEmitter_ = std::make_unique<ParticleEmitter>();

	// エミッターの初期設定
	emitterTransform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	previewEmitter_->Initialize("PreviewEmitter", emitterTransform_, emitCount_, emitFrequency_);
	
	//エミッターに発生させるパーティクルを設定
	previewEmitter_->SetParticleName(currentGroupName_);

	//全プリセットの読み込み
	LoadAllPresets();

	//テクスチャ名一覧の取得
	textureFileNames_ = TextureManager::GetInstance()->GetLoadedTextureFileNames();
}

//======================================================================
//			更新処理
//======================================================================

void ParticleEditor::Update() {

	if (autoApply_) {
		//属性を自動的に適用する場合、現在の属性をグループに適用
		particleManager_->SetPreset(currentGroupName_, currentPreset_);
	}

	particleManager_->Update();

	// プレビューエミッターの更新
	previewEmitter_->SetTranslate(emitterTransform_.translate);
	previewEmitter_->SetRotate(emitterTransform_.rotate);
	previewEmitter_->SetScale(emitterTransform_.scale);
	previewEmitter_->SetIsEmit(autoEmit_);
	particleManager_->GetParticleGroup(currentGroupName_)->SetEmitterPosition(emitterTransform_.translate);
	previewEmitter_->Update();

}

//======================================================================
//			ImGui更新処理
//======================================================================

void ParticleEditor::UpdateImGui() {

	// ImGuiのウィンドウを開始
	if (!ImGui::Begin("Particle Editor", nullptr, ImGuiWindowFlags_MenuBar)) {
		// ウィンドウが閉じられた場合は終了
		ImGui::End();
		return;
	}

	// メニューバーの作成
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Save Preset")) {
				// プリセットの保存
				SavePreset(selectedPresetName_);
			}
			if (ImGui::MenuItem("Load Preset")) {
				// プリセットの読み込み
				LoadPreset(selectedPresetName_);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	//タブ形式のUIの作成
	if (ImGui::BeginTabBar("ParticleEditorTabs")) {
		// パーティクル属性エディター
		if (ImGui::BeginTabItem("Particle Attributes")) {

			//属性の編集
			ImGui::SeparatorText("Attributes Setting");
			DrawParticleAttributesEditor();

			// エミッターコントロール
			ImGui::SeparatorText("Emitter Controls");
			DrawEmitterControls();

			ImGui::EndTabItem();
		}

		//プリセット管理
		if (ImGui::BeginTabItem("Preset Manager")) {

			// プレビュー設定
			DrawPreviewSettings();
			// プリセットマネージャー
			DrawPresetManager();

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

//======================================================================
//			終了処理
//======================================================================

void ParticleEditor::Finalize() {

	// プレビューエミッターの解放
	previewEmitter_.reset();
}

//======================================================================
//			描画処理
//======================================================================

void ParticleEditor::Draw() {

	particleCommon_->PreDraw();
	TakeCFrameWork::GetParticleManager()->Draw();
}

//======================================================================
//			プリセットの編集
//======================================================================

void ParticleEditor::DrawParticleAttributesEditor() {

	//テクスチャが再ロードされたかチェック
	TextureManager::GetInstance()->CheckAndReloadTextures();

	ParticleAttributes& attributes = currentPreset_.attribute;

	//attributeの編集
#pragma region coodinate attribute
	ImGui::SeparatorText("Particle Attributes");

	//Scale
	ImGui::SliderInt("Scale Setting", reinterpret_cast<int*>(&attributes.scaleSetting_), 0, 2, "None: %d, Scale Up: %d, Scale Down: %d");
	ImGui::DragFloat3("Scale", &attributes.scale.x, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat2("Scale Range", &attributes.scaleRange.min, 0.01f, 0.0f, 10.0f);

	//Rotate
	ImGui::DragFloat2("Rotate Range", &attributes.rotateRange.min, 0.01f, -3.14f, 3.14f);

	//Translate,Velocity
	ImGui::DragFloat2("Position Range", &attributes.positionRange.min, 0.01f, -10.0f, 10.0f);
	if (attributes.isTraslate_) {
		ImGui::DragFloat2("Velocity Range", &attributes.velocityRange.min, 0.01f, -10.0f, 10.0f);
	}
	
	//Color
	ImGui::Checkbox("Edit Color", &attributes.editColor);
	if (attributes.editColor) {
		ImGui::ColorEdit3("Color", &attributes.color.x);
		ImGui::DragFloat2("Color Range", &attributes.colorRange.min, 0.01f, 0.0f, 1.0f);
	}
	//lifetime
	ImGui::DragFloat2("Lifetime Range", &attributes.lifetimeRange.min, 0.01f, 0.0f, 10.0f);

	//Billboard
	ImGui::Checkbox("Is Billboard", &attributes.isBillboard);
	//Follow Emitter
	ImGui::Checkbox("Enable Follow Emitter", &attributes.enableFollowEmitter_);
	ImGui::Checkbox("TranslateUpdate", &attributes.isTraslate_);

	//設定の適用
	if (ImGui::Button("Apply Attributes")) {
		// 現在のグループに属性を適用
		particleManager_->SetPreset(currentGroupName_, currentPreset_);
	}
#pragma endregion

	//テクスチャファイルの設定
#pragma region texture setting
	ImGui::SeparatorText("Texture Setting");
	//テクスチャファイル名の選択
	static int slectedTextureIndex = 0;

	//今の設定がリストにあればインデックスを合わせる
	auto it = std::find(textureFileNames_.begin(), textureFileNames_.end(), currentPreset_.textureFilePath);
	if(it != textureFileNames_.end()) {
		slectedTextureIndex = static_cast<int>(std::distance(textureFileNames_.begin(), it));
	}

	
	if(ImGui::BeginCombo("Texture File",textureFileNames_.empty() ? "None":textureFileNames_[slectedTextureIndex].c_str())) {
		
		for (int i = 0; i < textureFileNames_.size(); ++i) {
			bool isSelected = (slectedTextureIndex == i);
			if (ImGui::Selectable(textureFileNames_[i].c_str(), isSelected)) {
				slectedTextureIndex = i;
				currentPreset_.textureFilePath = textureFileNames_[i];
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		
		

		ImGui::EndCombo();
	}
#pragma endregion

	//プリミティブタイプの選択

#pragma region primitive type setting
	ImGui::SeparatorText("Primitive Type");
	static const char* PrimitiveTypeNames[] = {
		"Ring", "Plane", "Sphere"
	};

	int currentType = static_cast<int>(currentPreset_.primitiveType);
	if (ImGui::Combo("Primitive Type", &currentType, PrimitiveTypeNames, IM_ARRAYSIZE(PrimitiveTypeNames))) {
		currentPreset_.primitiveType = static_cast<PrimitiveType>(currentType);
		// プリミティブタイプが変更された場合、グループのプリミティブを更新
		particleManager_->UpdatePrimitiveType(currentGroupName_, currentPreset_.primitiveType,currentPreset_.primitiveParameters);
	}
	ImGui::DragFloat3("Primitive Parameters", &currentPreset_.primitiveParameters.x, 0.01f, 0.0f, 10.0f);
	if(ImGui::Button("Update Primitive")) {
		// プリミティブの更新
		particleManager_->UpdatePrimitiveType(currentGroupName_, currentPreset_.primitiveType,currentPreset_.primitiveParameters);
	}


#pragma endregion

}

//======================================================================
//			エミッターコントロールの描画
//======================================================================

void ParticleEditor::DrawEmitterControls() {

	ImGui::Text("Emitter Controls");
	ImGui::Separator();

	//エミッターTrasformの表示
	ImGui::DragFloat3("Emitter Translate", &emitterTransform_.translate.x, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat3("Emitter Rotate", &emitterTransform_.rotate.x, 0.01f, -3.14f, 3.14f);
	ImGui::DragFloat3("Emitter Scale", &emitterTransform_.scale.x, 0.01f, 0.0f, 10.0f);

	//エミッターの発生設定
	ImGui::SliderInt("Emit Count", reinterpret_cast<int*>(&emitCount_), 1, 100, "Count: %d");
	ImGui::SliderFloat("Emit Frequency", &emitFrequency_, 0.0f, 1.0f, "Frequency: %.2f");

	//自動発生のチェックボックス
	ImGui::Checkbox("Auto Emit", &autoEmit_);

	//エミッターの発生ボタン
	if (ImGui::Button("Manual Emit")) {
		TakeCFrameWork::GetParticleManager()->Emit(currentGroupName_, emitterTransform_.translate, emitCount_);
	}

	ImGui::Separator();

	//エミッターのImGui表示
	if (ImGui::CollapsingHeader("Emitter Debug Info")) {

		previewEmitter_->UpdateImGui();
	}
}

//======================================================================
//			プレビュー設定の描画
//======================================================================

void ParticleEditor::DrawPreviewSettings() {

	ImGui::Text("Preview Settings");
	ImGui::Separator();

	// プレビューエミッターの状態表示
	ImGui::Text("Emitter Name: %s", previewEmitter_->GetEmitterName().c_str());
	ImGui::Text("Is Emit: %s", previewEmitter_->IsEmit() ? "True" : "False");
	ImGui::Text("Particle Count: %d", previewEmitter_->GetParticleCount());
	ImGui::Text("Translate: (%.2f, %.2f, %.2f)", emitterTransform_.translate.x, emitterTransform_.translate.y, emitterTransform_.translate.z);
	ImGui::Text("Rotate: (%.2f, %.2f, %.2f)", emitterTransform_.rotate.x, emitterTransform_.rotate.y, emitterTransform_.rotate.z);
	ImGui::Text("Scale: (%.2f, %.2f, %.2f)", emitterTransform_.scale.x, emitterTransform_.scale.y, emitterTransform_.scale.z);
	ImGui::Text("Emit Frequency: %.2f", previewEmitter_->GetFrequency());

	ImGui::Separator();

	//座標のリセット
	if (ImGui::Button("Reset Emitter Translate")) {
		emitterTransform_.translate = { 0.0f, 0.0f, 0.0f };
	}

	ImGui::SameLine();

	if (ImGui::Button("Stop Emit")) {
		//パーティクルを発生を停止
		autoEmit_ = false;
	}
}

//=======================================================================
//			プリセットマネージャーの描画
//=======================================================================

void ParticleEditor::DrawPresetManager() {

	ImGui::Text("Preset Manager");
	ImGui::Separator();

	// プリセット名の入力
	static char presetNameBuffer[64] = "";
	ImGui::InputText("Preset Name", presetNameBuffer, sizeof(presetNameBuffer));

	if (ImGui::Button("Refresh Preset List")) {
		// プリセット名のリストを更新
		RefreshPresetList();
	}

	// プリセットの保存
	if (ImGui::Button("Save current as Preset")) {

		// プリセット名が空でない場合のみ保存
		if (strlen(presetNameBuffer) > 0) {
			SavePreset(std::string(presetNameBuffer));
		}
	}

	ImGui::Separator();

	//プリセット一覧
	for (const std::string& presetName : presetNames_) {

		bool isSelected = (selectedPresetName_ == presetName);

		// プリセット名を表示し、選択可能にする
		if (ImGui::Selectable(presetName.c_str(),isSelected)) {
			selectedPresetName_ = presetName;
		}

		if (isSelected) {
			// 選択されたプリセットを読み込む
			if (ImGui::Button(("Load##" + presetName).c_str())) {
				LoadPreset(presetName);
			}
			// プリセットを削除する
			if (ImGui::Button(("Delete##" + presetName).c_str())) {
				//プリセットの削除
				DeletePreset(presetName);
			}
		}
	}
}

//========================================================================
//			プリセットの保存、読み込み、削除
//========================================================================

void ParticleEditor::SavePreset(const std::string& presetName) {

	if (presetName.empty() || presets_.find(presetName) != presets_.end()) {
		ImGui::Text("Preset already exists or name is empty: %s", presetName.c_str());
		return;
	}
	// 現在の属性をプリセットとして保存
	TakeCFrameWork::GetJsonLoader()->SaveParticlePreset(presetName, currentPreset_);
	// プリセット名を更新
	presetNames_ = TakeCFrameWork::GetJsonLoader()->GetParticlePresetList();

}

//=======================================================================
//			プリセットの読み込み、削除、デフォルトプリセットの読み込み
//========================================================================

void ParticleEditor::LoadPreset(const std::string& presetName) {

	if (presetName.empty() || presets_.find(presetName) == presets_.end()) {
		ImGui::Text("Preset not found: %s", presetName.c_str());
		return;
	}

	//JSONからプリセットを読み込む
	currentPreset_ = TakeCFrameWork::GetJsonLoader()->LoadParticlePreset(presetName);

	// 現在のグループに属性を適用
	particleManager_->SetPreset(currentGroupName_, currentPreset_);
}

//========================================================================
//			プリセットの削除
//========================================================================

void ParticleEditor::DeletePreset(const std::string& presetName) {
	// プリセットが存在するか確認
	if (presetName.empty() || presets_.find(presetName) == presets_.end()) {
		ImGui::Text("Preset not found: %s", presetName.c_str());
		return;
	}

	// プリセットを削除
	TakeCFrameWork::GetJsonLoader()->DeleteParticlePreset(presetName);

	//プリセット名更新
	RefreshPresetList();
}

//=======================================================================
//			デフォルトプリセットの読み込み
//========================================================================

void ParticleEditor::LoadDefaultPreset() {
	// デフォルトプリセットの読み込み
	currentPreset_ = TakeCFrameWork::GetJsonLoader()->LoadParticlePreset("DefaultPreset");
	// 現在のグループに属性を適用
	particleManager_->SetPreset(currentGroupName_, currentPreset_);
}

//=======================================================================
//			全てのプリセットを読み込み
//========================================================================

void ParticleEditor::LoadAllPresets() {

	//プリセットのリストを取得
	RefreshPresetList();

	// 各プリセットを読み込み
	for (const std::string& presetName : presetNames_) {
		// プリセットを読み込む
		presets_[presetName] = TakeCFrameWork::GetJsonLoader()->LoadParticlePreset(presetName);
	}

	// デバッグ出力（必要に応じて削除）
	OutputDebugStringA(("Loaded " + std::to_string(presets_.size()) + " presets from folder\n").c_str());
}

//=======================================================================
//			プリセット名のリストを更新
//========================================================================

void ParticleEditor::RefreshPresetList() {
	// プリセット名のリストを更新
	presetNames_ = TakeCFrameWork::GetJsonLoader()->GetParticlePresetList();
}
