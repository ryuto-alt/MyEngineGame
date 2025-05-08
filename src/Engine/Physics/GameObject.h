#pragma once
#include "btBulletDynamicsCommon.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Mymath.h"
#include "Model.h"

// 物理オブジェクトとモデルを統合するゲームオブジェクトクラス
class GameObject {
public:
    // コンストラクタ
    GameObject();
    // デストラクタ
    ~GameObject();

    // 初期化
    void Initialize(Model* model, btCollisionShape* shape, btRigidBody* body);
    // 更新
    void Update();
    // 描画
    void Draw();

    // 位置の設定
    void SetPosition(const Vector3& position);
    // 回転の設定
    void SetRotation(const Vector3& rotation);
    // スケールの設定
    void SetScale(const Vector3& scale);

    // アクセサ
    const Vector3& GetPosition() const { return position_; }
    const Vector3& GetRotation() const { return rotation_; }
    const Vector3& GetScale() const { return scale_; }
    Model* GetModel() const { return model_; }
    btCollisionShape* GetShape() const { return shape_; }
    btRigidBody* GetBody() const { return body_; }

    // トランスフォーム行列の計算
    Matrix4x4 GetWorldMatrix() const;

    // Bulletのトランスフォームを更新
    void UpdateBulletTransform();
    // モデルのトランスフォームを更新（Bulletから）
    void UpdateModelTransform();

private:
    // 位置
    Vector3 position_ = {0.0f, 0.0f, 0.0f};
    // 回転（オイラー角）
    Vector3 rotation_ = {0.0f, 0.0f, 0.0f};
    // スケール
    Vector3 scale_ = {1.0f, 1.0f, 1.0f};
    
    // モデル
    Model* model_ = nullptr;
    // 衝突形状
    btCollisionShape* shape_ = nullptr;
    // 剛体
    btRigidBody* body_ = nullptr;
};