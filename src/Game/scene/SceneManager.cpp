#include "SceneManager.h"
#include "SceneFactory.h"
#include <cassert>

// 静的メンバ変数の実体化
SceneManager* SceneManager::instance_ = nullptr;

SceneManager* SceneManager::GetInstance() {
    if (!instance_) {
        instance_ = new SceneManager();
    }
    return instance_;
}

void SceneManager::Initialize(SceneFactory* sceneFactory) {
    assert(sceneFactory);
    sceneFactory_ = sceneFactory;

    // 最初のシーンをTitleに設定
    nextScene_ = "Title";

    // デバッグ出力
    OutputDebugStringA("SceneManager initialized successfully\n");
}

void SceneManager::Update() {
    // SRVヒープを毎フレーム設定（これが重要）
    if (srvManager_) {
        srvManager_->PreDraw();
    }

    // シーン切り替えチェック
    if (!nextScene_.empty()) {
        // 詳細なデバッグ出力
        OutputDebugStringA("SceneManager: *** SCENE TRANSITION START ***\n");
        std::string currentMsg = "SceneManager: Current scene: " + currentSceneName_ + "\n";
        OutputDebugStringA(currentMsg.c_str());
        std::string changeMsg = "SceneManager: Changing to scene: " + nextScene_ + "\n";
        OutputDebugStringA(changeMsg.c_str());

        // 現在のシーンの終了処理
        if (currentScene_) {
            std::string finalizeMsg = "SceneManager: Finalizing current scene: " + currentSceneName_ + "\n";
            OutputDebugStringA(finalizeMsg.c_str());
            try {
                currentScene_->Finalize();
                std::string successMsg = "SceneManager: Successfully finalized scene: " + currentSceneName_ + "\n";
                OutputDebugStringA(successMsg.c_str());
            }
            catch (const std::exception& e) {
                std::string errorMsg = "ERROR: Failed to finalize scene " + currentSceneName_ + ": " + std::string(e.what()) + "\n";
                OutputDebugStringA(errorMsg.c_str());
            }
            currentScene_.reset(); // unique_ptrをクリア
            OutputDebugStringA("SceneManager: Current scene reset\n");
        }

        // 次のシーンを生成
        std::string createMsg = "SceneManager: Creating new scene: " + nextScene_ + "\n";
        OutputDebugStringA(createMsg.c_str());
        try {
            currentScene_ = sceneFactory_->CreateScene(nextScene_);
            if (!currentScene_) {
                std::string nullMsg = "ERROR: SceneFactory returned null for scene: " + nextScene_ + "\n";
                OutputDebugStringA(nullMsg.c_str());
                nextScene_.clear();
                return;
            }
            std::string createdMsg = "SceneManager: Successfully created scene: " + nextScene_ + "\n";
            OutputDebugStringA(createdMsg.c_str());
        }
        catch (const std::exception& e) {
            std::string createErrorMsg = "ERROR: Failed to create scene " + nextScene_ + ": " + std::string(e.what()) + "\n";
            OutputDebugStringA(createErrorMsg.c_str());
            nextScene_.clear();
            return;
        }

        // シーンマネージャーのポインタをセット
        currentScene_->SetSceneManager(this);

        // 共通リソースをセット
        currentScene_->SetDirectXCommon(dxCommon_);
        currentScene_->SetInput(input_);
        currentScene_->SetSpriteCommon(spriteCommon_);
        currentScene_->SetSrvManager(srvManager_);
        currentScene_->SetCamera(camera_);
        OutputDebugStringA("SceneManager: Common resources set\n");

        try {
            // シーンの初期化（例外をキャッチ）
            std::string initMsg = "SceneManager: Initializing scene: " + nextScene_ + "\n";
            OutputDebugStringA(initMsg.c_str());
            currentScene_->Initialize();
            
            // 現在のシーン名を更新
            currentSceneName_ = nextScene_;
            
            OutputDebugStringA("SceneManager: *** SCENE TRANSITION COMPLETE ***\n");
            std::string initSuccessMsg = "SceneManager: Successfully initialized scene: " + nextScene_ + "\n";
            OutputDebugStringA(initSuccessMsg.c_str());
        }
        catch (const std::exception& e) {
            std::string initErrorMsg = "ERROR: Failed to initialize scene " + nextScene_ + ": " + std::string(e.what()) + "\n";
            OutputDebugStringA(initErrorMsg.c_str());
            currentScene_.reset();
            currentSceneName_ = "None";
        }

        // 次のシーン名をクリア
        nextScene_.clear();
    }

    // 現在のシーンの更新
    if (currentScene_) {
        try {
            currentScene_->Update();
        }
        catch (const std::exception& e) {
            OutputDebugStringA(("ERROR: Exception in scene update: " + std::string(e.what()) + "\n").c_str());
        }
    } else {
        OutputDebugStringA("WARNING: No current scene to update\n");
    }
}

void SceneManager::Draw() {
    // SRVヒープを描画前に設定
    if (srvManager_) {
        srvManager_->PreDraw();
    }

    // 現在のシーンの描画
    if (currentScene_) {
        try {
            currentScene_->Draw();
        }
        catch (const std::exception& e) {
            OutputDebugStringA(("ERROR: Exception in scene draw: " + std::string(e.what()) + "\n").c_str());
        }
    }
}

void SceneManager::Finalize() {
    // 現在のシーンの終了処理
    if (currentScene_) {
        try {
            currentScene_->Finalize();
        }
        catch (const std::exception& e) {
            OutputDebugStringA(("ERROR: Exception in scene finalize: " + std::string(e.what()) + "\n").c_str());
        }
        currentScene_.reset(); // 明示的にunique_ptrをクリア
    }

    // シングルトンインスタンスの解放
    delete instance_;
    instance_ = nullptr;

    // デバッグ出力
    OutputDebugStringA("SceneManager finalized successfully\n");
}

void SceneManager::ChangeScene(const std::string& sceneName) {
    // 次のシーン名を設定
    nextScene_ = sceneName;

    // デバッグ出力
    std::string requestMsg = "SceneManager: Scene change requested to " + sceneName + "\n";
    OutputDebugStringA(requestMsg.c_str());
}