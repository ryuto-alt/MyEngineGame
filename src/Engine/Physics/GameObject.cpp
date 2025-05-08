#include "GameObject.h"

GameObject::GameObject() {
    // 初期化は別メソッドで行う
}

GameObject::~GameObject() {
    // ここではリソースを解放しない（GamePlaySceneで一括管理）
}

void GameObject::Initialize(Model* model, btCollisionShape* shape, btRigidBody* body) {
    model_ = model;
    shape_ = shape;
    body_ = body;
}

void GameObject::Update() {
    // Bulletの物理演算結果をモデルの位置・回転に反映
    UpdateModelTransform();
}

void GameObject::Draw() {
    // モデルの描画
    if (model_) {
        // 位置情報のデバッグ出力
        OutputDebugStringA("GameObject::Draw - ");
        char buffer[256];
        sprintf_s(buffer, "位置: (%.2f, %.2f, %.2f)\n", position_.x, position_.y, position_.z);
        OutputDebugStringA(buffer);
        
        // モデルの描画処理
        model_->Draw();   // モデルの描画
    } else {
        OutputDebugStringA("GameObject::Draw - モデルが設定されていません\n");
    }
}

void GameObject::SetPosition(const Vector3& position) {
    position_ = position;

    // 物理ボディがある場合は位置を更新
    if (body_) {
        UpdateBulletTransform();
    }

    // モデルがある場合は位置を更新
    if (model_) {
        model_->SetPosition(position_);
    }
}

void GameObject::SetRotation(const Vector3& rotation) {
    rotation_ = rotation;

    // 物理ボディがある場合は回転を更新
    if (body_) {
        UpdateBulletTransform();
    }

    // モデルがある場合は回転を更新
    if (model_) {
        model_->SetRotation(rotation_);
    }
}

void GameObject::SetScale(const Vector3& scale) {
    scale_ = scale;

    // スケールはBullet側には直接反映されない

    // モデルがある場合はスケールを更新
    if (model_) {
        model_->SetScale(scale_);
    }
}

Matrix4x4 GameObject::GetWorldMatrix() const {
    // 位置・回転・スケールからワールド行列を計算
    Matrix4x4 matScale = MakeScaleMatrix(scale_);
    Matrix4x4 matRotX = MakeRotateXMatrix(rotation_.x);
    Matrix4x4 matRotY = MakeRotateYMatrix(rotation_.y);
    Matrix4x4 matRotZ = MakeRotateZMatrix(rotation_.z);
    Matrix4x4 matTrans = MakeTranslateMatrix(position_);

    // 回転合成
    Matrix4x4 matRot = Multiply(matRotZ, Multiply(matRotX, matRotY));

    // ワールド行列を合成
    return Multiply(matScale, Multiply(matRot, matTrans));
}

void GameObject::UpdateBulletTransform() {
    if (body_) {
        // 現在の変換行列を取得
        btTransform transform;
        body_->getMotionState()->getWorldTransform(transform);

        // 位置を更新
        transform.setOrigin(btVector3(position_.x, position_.y, position_.z));

        // 回転を更新（オイラー角からクォータニオンに変換）
        btQuaternion rotation;
        // YXZの順で適用（DirectXの一般的な順序）
        rotation.setEulerZYX(rotation_.z, rotation_.y, rotation_.x);
        transform.setRotation(rotation);

        // 変換行列を設定
        body_->getMotionState()->setWorldTransform(transform);
        body_->setWorldTransform(transform);

        // 速度をリセット
        body_->setLinearVelocity(btVector3(0, 0, 0));
        body_->setAngularVelocity(btVector3(0, 0, 0));

        // 物理計算をアクティブ化
        body_->activate(true);
    }
}

void GameObject::UpdateModelTransform() {
    if (body_ && body_->getMotionState()) {
        // Bulletの変換行列を取得
        btTransform transform;
        body_->getMotionState()->getWorldTransform(transform);

        // 位置を更新
        btVector3 pos = transform.getOrigin();
        position_ = Vector3(pos.x(), pos.y(), pos.z());

        // 回転を更新（クォータニオンからオイラー角に変換）
        btMatrix3x3 rotMat = transform.getBasis();
        rotMat.getEulerYPR(rotation_.y, rotation_.x, rotation_.z);

        // モデルの位置と回転を更新
        if (model_) {
            model_->SetPosition(position_);
            model_->SetRotation(rotation_);
            
            // デバッグ情報出力
            char buffer[256];
            sprintf_s(buffer, "GameObject::UpdateModelTransform - 位置: (%.2f, %.2f, %.2f)\n", 
                     position_.x, position_.y, position_.z);
            OutputDebugStringA(buffer);
            
            // モデルの更新を確実に実行
            model_->Update();
        }
    }
}