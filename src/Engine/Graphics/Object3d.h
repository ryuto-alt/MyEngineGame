#pragma once
#include "Model.h"
#include "Matrix4x4.h"
#include "Vector3.h"
#include "math.h"
#include "Camera.h"

#include <d3d12.h>
#include <wrl.h>
#include <memory>

class DirectXCommon;
class SpriteCommon;
class Camera;

// 3Dオブジェクトクラス
class Object3d {
public:
    // コンストラクタ
    Object3d();
    // デストラクタ
    ~Object3d();

    // 初期化
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon);
    // モデルのセット
    void SetModel(Model* model);
    // 更新処理（従来のメソッド - 後方互換性のため残す）
    void Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix);
    // 描画処理
    void Draw();

    // カメラ関連のメソッド
    void SetCamera(Camera* camera);
    Camera* GetCamera() const;

    // カメラを使用するUpdateメソッド
    void Update();

    // 座標の設定
    void SetPosition(const Vector3& position) { transform_.translate = position; }
    const Vector3& GetPosition() const { return transform_.translate; }

    // 回転の設定
    void SetRotation(const Vector3& rotation) { transform_.rotate = rotation; }
    const Vector3& GetRotation() const { return transform_.rotate; }

    // スケールの設定
    void SetScale(const Vector3& scale) { transform_.scale = scale; }
    const Vector3& GetScale() const { return transform_.scale; }

    // カラーの設定
    void SetColor(const Vector4& color) { materialData_->color = color; }
    const Vector4& GetColor() const { return materialData_->color; }

    // ライトを有効にするか
    void SetEnableLighting(bool enable) { materialData_->enableLighting = enable ? 1 : 0; }
    bool GetEnableLighting() const { return materialData_->enableLighting != 0; }

    // ライトの設定
    void SetDirectionalLight(const DirectionalLight& light) { *directionalLightData_ = light; }
    const DirectionalLight& GetDirectionalLight() const { return *directionalLightData_; }

    /// <summary>
    /// 円形エフェクト用テクスチャの設定
    /// gradationLine.pngを使用した円形エフェクトを描画する
    /// </summary>
    void SetCircleEffectTexture();

    /// <summary>
    /// UVScroll の設定
    /// エフェクトの動的な変化に使用
    /// </summary>
    /// <param name="scrollU">U方向のスクロール量</param>
    /// <param name="scrollV">V方向のスクロール量</param>
    void SetUVScroll(float scrollU, float scrollV);
    
    // リソースアクセサー（CircleEffect用）
    ID3D12Resource* GetMaterialResource() const { return materialResource_.Get(); }
    ID3D12Resource* GetTransformationMatrixResource() const { return transformationMatrixResource_.Get(); }
    ID3D12Resource* GetDirectionalLightResource() const { return directionalLightResource_.Get(); }

private:
    // モデル
    Model* model_;

    // DirectXCommon
    DirectXCommon* dxCommon_;
    // SpriteCommon
    SpriteCommon* spriteCommon_;

    // マテリアルリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    // マテリアルデータ
    Material* materialData_;

    // 変換行列リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
    // 変換行列データ
    TransformationMatrix* transformationMatrixData_;

    // ライトリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
    // ライトデータ
    DirectionalLight* directionalLightData_;

    // トランスフォーム
    Transform transform_;

    // カメラへの参照
    Camera* camera_ = nullptr;
};