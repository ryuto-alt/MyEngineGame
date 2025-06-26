#include "TitleScene.h"

TitleScene::TitleScene() {
    // コンストラクタでは特に何もしない
}

TitleScene::~TitleScene() {
    // デストラクタでも特に何もしない（ResourceはuniquePtr）
}

void TitleScene::Initialize() {
    // 必要なリソースの取得確認
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(camera_);

    // カメラの初期位置設定
    camera_->SetTranslate({ 0.0f, 0.0f, -10.0f });

    // 3Dオブジェクトとスプライトの初期化を削除

    // 初期化完了
    initialized_ = true;
}

void TitleScene::Update() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // カメラの更新
    camera_->Update();

    // オブジェクトの更新処理を削除

    // スペースキーでゲームプレイシーンへ
    if (input_->TriggerKey(DIK_SPACE)) {
        sceneManager_->ChangeScene("GamePlay");
    }
}

void TitleScene::Draw() {
    // 初期化されていない場合は何もしない
    if (!initialized_) return;

    // 描画処理を削除
}

void TitleScene::Finalize() {
    // 特に追加のリソース解放が必要なければ何もしない
}