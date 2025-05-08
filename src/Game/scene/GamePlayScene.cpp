#include "GamePlayScene.h"
#include "../../../src/Engine/Physics/GameObject.h"

GamePlayScene::GamePlayScene() {
    // コンストラクタでは特に何もしない
}

GamePlayScene::~GamePlayScene() {
    Finalize();
}

void GamePlayScene::Initialize() {
    // 必要なリソースの取得確認
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(camera_);

    // カメラの初期設定
    camera_->SetTranslate(cameraPosition_);
    camera_->SetRotate(cameraRotation_);
    camera_->Update();

    // Bullet物理シミュレーションの初期化
    // 衝突検出方法の設定
    collisionConfiguration_ = new btDefaultCollisionConfiguration();
    // 衝突検出の振り分け処理
    dispatcher_ = new btCollisionDispatcher(collisionConfiguration_);
    // ブロードフェーズ法の設定
    broadphase_ = new btDbvtBroadphase();
    // 拘束（剛体間リンク）のソルバ設定
    solver_ = new btSequentialImpulseConstraintSolver();
    // ワールド作成
    dynamicsWorld_ = new btDiscreteDynamicsWorld(
        dispatcher_, broadphase_, solver_, collisionConfiguration_);
    // 重力設定（Y軸の負方向）
    dynamicsWorld_->setGravity(btVector3(0, -9.8f, 0));

    // 地面の作成
    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(btVector3(0, -1, 0));

    // 床形状の作成（箱型）
    groundShape_ = new btBoxShape(btVector3(50, 1, 50));

    // 剛体の情報を設定
    btRigidBody::btRigidBodyConstructionInfo rbInfo(
        0, // 質量（0で静的な物体）
        new btDefaultMotionState(groundTransform),
        groundShape_,
        btVector3(0, 0, 0) // 慣性（静的物体なので0）
    );

    // 反発係数の設定
    rbInfo.m_restitution = 0.5f;
    // 摩擦係数の設定
    rbInfo.m_friction = 0.5f;

    // 剛体を作成して世界に追加
    groundBody_ = new btRigidBody(rbInfo);
    dynamicsWorld_->addRigidBody(groundBody_);

    // 地面用ゲームオブジェクトの作成
    groundObject_ = new GameObject();
    // モデルは作成しない（床は見えなくてもOK）

    // 箱をいくつか追加
    AddBox(Vector3(0.0f, 10.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f), 1.0f);
    AddBox(Vector3(1.0f, 12.0f, 1.0f), Vector3(1.0f, 1.0f, 1.0f), 1.0f);
    AddBox(Vector3(-1.0f, 14.0f, -1.0f), Vector3(1.0f, 1.0f, 1.0f), 1.0f);

    // 球もいくつか追加
    AddSphere(Vector3(3.0f, 8.0f, 0.0f), 0.8f, 1.0f);
    AddSphere(Vector3(-3.0f, 6.0f, 0.0f), 0.8f, 1.0f);

    // 初期化完了
    initialized_ = true;
    OutputDebugStringA("GamePlayScene: 初期化完了\n");
}

void GamePlayScene::Update() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // ESCキーでタイトルシーンに戻る
    if (input_->TriggerKey(DIK_ESCAPE)) {
        sceneManager_->ChangeScene("Title");
        return;
    }

    // カメラの移動処理
    if (input_->PushKey(DIK_W)) {
        cameraPosition_.z += cameraSpeed_;
    }
    if (input_->PushKey(DIK_S)) {
        cameraPosition_.z -= cameraSpeed_;
    }
    if (input_->PushKey(DIK_A)) {
        cameraPosition_.x -= cameraSpeed_;
    }
    if (input_->PushKey(DIK_D)) {
        cameraPosition_.x += cameraSpeed_;
    }
    if (input_->PushKey(DIK_Q)) {
        cameraPosition_.y += cameraSpeed_;
    }
    if (input_->PushKey(DIK_E)) {
        cameraPosition_.y -= cameraSpeed_;
    }

    // カメラの回転処理
    if (input_->PushKey(DIK_UP)) {
        cameraRotation_.x += 0.01f;
    }
    if (input_->PushKey(DIK_DOWN)) {
        cameraRotation_.x -= 0.01f;
    }
    if (input_->PushKey(DIK_LEFT)) {
        cameraRotation_.y += 0.01f;
    }
    if (input_->PushKey(DIK_RIGHT)) {
        cameraRotation_.y -= 0.01f;
    }

    // カメラ更新
    camera_->SetTranslate(cameraPosition_);
    camera_->SetRotate(cameraRotation_);
    camera_->Update();

    // スペースキーで新しい箱を追加
    if (input_->TriggerKey(DIK_SPACE)) {
        // ランダムな位置と大きさで箱を追加
        float x = static_cast<float>(rand() % 10 - 5);
        float z = static_cast<float>(rand() % 10 - 5);
        float scale = static_cast<float>(rand() % 100 + 50) / 100.0f;
        AddBox(Vector3(x, 15.0f, z), Vector3(scale, scale, scale), scale);
    }

    // Enterキーで新しい球を追加
    if (input_->TriggerKey(DIK_RETURN)) {
        // ランダムな位置と半径で球を追加
        float x = static_cast<float>(rand() % 10 - 5);
        float z = static_cast<float>(rand() % 10 - 5);
        float radius = static_cast<float>(rand() % 100 + 30) / 100.0f;
        AddSphere(Vector3(x, 15.0f, z), radius, radius * 2.0f);
    }

    // 物理シミュレーションの更新（1/60秒固定時間ステップ）
    dynamicsWorld_->stepSimulation(1.0f / 60.0f, 10);

    // ゲームオブジェクトの更新
    for (auto* obj : gameObjects_) {
        obj->Update(); // GameObjectのUpdate内でBulletからのモデル位置更新を行う
    }
}

void GamePlayScene::Draw() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // 青い画面の描画は自動的に行われる

    // デバッグ情報の出力
    OutputDebugStringA("GamePlayScene::Draw - 描画開始\n");
    char buffer[256];
    sprintf_s(buffer, "カメラ位置: (%.2f, %.2f, %.2f)\n", cameraPosition_.x, cameraPosition_.y, cameraPosition_.z);
    OutputDebugStringA(buffer);

    // ゲームオブジェクトの描画
    for (size_t i = 0; i < gameObjects_.size(); i++) {
        OutputDebugStringA(("描画オブジェクト " + std::to_string(i) + "\n").c_str());
        // GameObjectのDrawメソッドを呼び出す
        gameObjects_[i]->Draw();
    }

    // 操作説明をImGuiで表示
    ImGui::Begin("操作説明");
    ImGui::Text("ESC - タイトルに戻る");
    ImGui::Text("WASD - カメラ移動");
    ImGui::Text("QE - カメラ上下");
    ImGui::Text("矢印キー - カメラ回転");
    ImGui::Text("スペース - 箱を追加");
    ImGui::Text("Enter - 球を追加");
    ImGui::Text("物理オブジェクト数: %zu", gameObjects_.size());
    ImGui::End();

    // デバッグ情報を表示
    ImGui::Begin("デバッグ情報");
    ImGui::Text("カメラ位置: (%.2f, %.2f, %.2f)", cameraPosition_.x, cameraPosition_.y, cameraPosition_.z);
    ImGui::Text("カメラ回転: (%.2f, %.2f, %.2f)", cameraRotation_.x, cameraRotation_.y, cameraRotation_.z);
    if (!gameObjects_.empty()) {
        const Vector3& pos = gameObjects_[0]->GetPosition();
        ImGui::Text("最初のオブジェクト位置: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
    }
    ImGui::End();
}

void GamePlayScene::Finalize() {
    // ゲームオブジェクトの解放
    for (auto* obj : gameObjects_) {
        btRigidBody* body = obj->GetBody();
        if (body) {
            dynamicsWorld_->removeRigidBody(body);
            delete body->getMotionState();
            delete body;
        }
        delete obj;
    }
    gameObjects_.clear();

    // モデルの解放
    for (auto* model : models_) {
        delete model;
    }
    models_.clear();

    // 地面オブジェクトの解放
    if (groundBody_) {
        dynamicsWorld_->removeRigidBody(groundBody_);
        delete groundBody_->getMotionState();
        delete groundBody_;
        groundBody_ = nullptr;
    }
    if (groundShape_) {
        delete groundShape_;
        groundShape_ = nullptr;
    }
    if (groundObject_) {
        delete groundObject_;
        groundObject_ = nullptr;
    }

    // Bullet物理シミュレーションの解放
    if (dynamicsWorld_) {
        delete dynamicsWorld_;
        dynamicsWorld_ = nullptr;
    }
    if (solver_) {
        delete solver_;
        solver_ = nullptr;
    }
    if (broadphase_) {
        delete broadphase_;
        broadphase_ = nullptr;
    }
    if (dispatcher_) {
        delete dispatcher_;
        dispatcher_ = nullptr;
    }
    if (collisionConfiguration_) {
        delete collisionConfiguration_;
        collisionConfiguration_ = nullptr;
    }

    OutputDebugStringA("GamePlayScene: 終了処理完了\n");
}

// 箱型の物理オブジェクトを追加する関数
GameObject* GamePlayScene::AddBox(const Vector3& position, const Vector3& size, float mass) {
    // モデルを作成（LoadFromObjでロードする代わりに、基本形状を使用）
    Model* model = new Model();
    model->Initialize(dxCommon_);

    // 立方体モデルを作成
    model->CreateCube();

    // モデルの後でのメモリ解放のために保存
    models_.push_back(model);

    // 箱型の衝突形状を作成
    btCollisionShape* shape = new btBoxShape(btVector3(size.x, size.y, size.z));

    // 慣性の計算
    btVector3 localInertia(0, 0, 0);
    if (mass != 0.0f) {
        shape->calculateLocalInertia(mass, localInertia);
    }

    // 位置と回転の設定
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(position.x, position.y, position.z));

    // モーションステートの作成
    btDefaultMotionState* motionState = new btDefaultMotionState(transform);

    // 剛体の情報を設定
    btRigidBody::btRigidBodyConstructionInfo rbInfo(
        mass, motionState, shape, localInertia);

    // 反発係数と摩擦係数の設定
    rbInfo.m_restitution = 0.5f;
    rbInfo.m_friction = 0.5f;

    // 剛体の作成と世界への追加
    btRigidBody* body = new btRigidBody(rbInfo);
    dynamicsWorld_->addRigidBody(body);

    // ゲームオブジェクトの作成と初期化
    GameObject* gameObject = new GameObject();
    gameObject->Initialize(model, shape, body);
    gameObject->SetPosition(position);
    gameObject->SetScale(size);

    // リストに追加
    gameObjects_.push_back(gameObject);

    return gameObject;
}

// 球型の物理オブジェクトを追加する関数
GameObject* GamePlayScene::AddSphere(const Vector3& position, float radius, float mass) {
    // モデルを作成（LoadFromObjでロードする代わりに、基本形状を使用）
    Model* model = new Model();
    model->Initialize(dxCommon_);

    // 球体モデルを作成（セグメント数を指定して詳細度を調整）
    model->CreateSphere(16);

    // モデルの後でのメモリ解放のために保存
    models_.push_back(model);

    // 球型の衝突形状を作成
    btCollisionShape* shape = new btSphereShape(radius);

    // 慣性の計算
    btVector3 localInertia(0, 0, 0);
    if (mass != 0.0f) {
        shape->calculateLocalInertia(mass, localInertia);
    }

    // 位置と回転の設定
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(position.x, position.y, position.z));

    // モーションステートの作成
    btDefaultMotionState* motionState = new btDefaultMotionState(transform);

    // 剛体の情報を設定
    btRigidBody::btRigidBodyConstructionInfo rbInfo(
        mass, motionState, shape, localInertia);

    // 反発係数と摩擦係数の設定
    rbInfo.m_restitution = 0.7f;
    rbInfo.m_friction = 0.3f;

    // 剛体の作成と世界への追加
    btRigidBody* body = new btRigidBody(rbInfo);
    dynamicsWorld_->addRigidBody(body);

    // ゲームオブジェクトの作成と初期化
    GameObject* gameObject = new GameObject();
    gameObject->Initialize(model, shape, body);
    gameObject->SetPosition(position);
    gameObject->SetScale(Vector3(radius, radius, radius));

    // リストに追加
    gameObjects_.push_back(gameObject);

    return gameObject;
}