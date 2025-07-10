// src/Engine/Graphics/Object3d.cpp
#include "Object3d.h"
#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "Mymath.h"
#include "TextureManager.h"
#include "Animation.h"
#include "AnimatedModel.h"
#include "AnimationUtility.h"
#include "UnoEngine.h"


Object3d::Object3d() : model_(nullptr), dxCommon_(nullptr), spriteCommon_(nullptr),
materialData_(nullptr), transformationMatrixData_(nullptr), directionalLightData_(nullptr),
camera_(nullptr) {
    // 初期値設定
    transform_.scale = { 1.0f, 1.0f, 1.0f };
    transform_.rotate = { 0.0f, 0.0f, 0.0f };
    transform_.translate = { 0.0f, 0.0f, 0.0f };
    
    // アニメーション行列を単位行列で初期化
    animationMatrix_ = MakeIdentity4x4();
}

Object3d::~Object3d() {
    // ComPtrリソースの解放（Unmapは不要 - これらは永続的にマップされている）
    if (materialResource_) {
        materialResource_.Reset();
    }
    if (transformationMatrixResource_) {
        transformationMatrixResource_.Reset();
    }
    if (directionalLightResource_) {
        directionalLightResource_.Reset();
    }
    
    // データポインタをnullptrに設定（安全のため）
    materialData_ = nullptr;
    transformationMatrixData_ = nullptr;
    directionalLightData_ = nullptr;
}

void Object3d::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    assert(dxCommon);
    assert(spriteCommon);
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;

    // マテリアルリソースの作成
    materialResource_ = dxCommon_->CreateBufferResource(sizeof(Material));
    // マテリアルデータの書き込み
    materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
    materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    materialData_->enableLighting = true;
    materialData_->uvTransform = MakeIdentity4x4();

    // 変換行列リソースの作成
    transformationMatrixResource_ = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));
    // 変換行列データの書き込み
    transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
    transformationMatrixData_->WVP = MakeIdentity4x4();
    transformationMatrixData_->World = MakeIdentity4x4();

    // ライトリソースの作成
    directionalLightResource_ = dxCommon_->CreateBufferResource(sizeof(DirectionalLight));
    // ライトデータの書き込み
    directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
    directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
    directionalLightData_->direction = { 0.0f, -1.0f, 0.0f };
    directionalLightData_->intensity = 1.0f;
}

void Object3d::SetModel(Model* model) {
    model_ = model;

    OutputDebugStringA("Object3d::SetModel - Called\n");
    OutputDebugStringA(("Object3d::SetModel - model_ is " + std::string(model_ ? "not null" : "null") + "\n").c_str());
    OutputDebugStringA(("Object3d::SetModel - materialData_ is " + std::string(materialData_ ? "not null" : "null") + "\n").c_str());

    // モデルのマテリアル情報をシェーダーに設定
    if (model_ && materialData_) {
        OutputDebugStringA("Object3d::SetModel - Applying material data\n");
        const MaterialData& modelMaterial = model_->GetMaterial();

        // マテリアルデータをシェーダーのMaterial構造体に反映
        // シェーダーのcolor変数にdiffuse色を設定
        materialData_->color = modelMaterial.diffuse;

        // アルファ値も設定
        materialData_->color.w = modelMaterial.alpha;

        // モデルのテクスチャパスの確認
        std::string texturePath = model_->GetTextureFilePath();
        OutputDebugStringA(("Object3d::SetModel - Model texture path: " + texturePath + "\n").c_str());

        if (!texturePath.empty()) {
            // テクスチャが未ロードなら読み込む
            if (!TextureManager::GetInstance()->IsTextureExists(texturePath)) {
                OutputDebugStringA(("Object3d::SetModel - Loading texture: " + texturePath + "\n").c_str());
                TextureManager::GetInstance()->LoadTexture(texturePath);
            }
            else {
                OutputDebugStringA(("Object3d::SetModel - Texture already loaded: " + texturePath + "\n").c_str());
            }
        }
        else {
            OutputDebugStringA("Object3d::SetModel - No texture path provided by model\n");
        }

        // デバッグ情報
        OutputDebugStringA("Object3d::SetModel - Material information:\n");
        OutputDebugStringA(("  - Diffuse (RGBA): " +
            std::to_string(materialData_->color.x) + ", " +
            std::to_string(materialData_->color.y) + ", " +
            std::to_string(materialData_->color.z) + ", " +
            std::to_string(materialData_->color.w) + "\n").c_str());
        OutputDebugStringA(("  - Texture: " + (texturePath.empty() ? "None" : texturePath) + "\n").c_str());
    } else {
        OutputDebugStringA("Object3d::SetModel - materialData_ is null, cannot apply material\n");
        if (!materialData_) {
            OutputDebugStringA("Object3d::SetModel - ERROR: materialData_ is null - Object3d may not be properly initialized\n");
        }
    }
}

// 従来のUpdateメソッド（ビュー行列とプロジェクション行列を直接指定）
void Object3d::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {
    assert(transformationMatrixData_);

    // ワールド行列の計算
    Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    
    // アニメーション行列とワールド行列を合成
    Matrix4x4 finalWorldMatrix = Multiply(animationMatrix_, worldMatrix);

    // WVP行列の計算
    Matrix4x4 worldViewProjectionMatrix = Multiply(finalWorldMatrix, Multiply(viewMatrix, projectionMatrix));

    // 行列の更新
    transformationMatrixData_->WVP = worldViewProjectionMatrix;
    transformationMatrixData_->World = finalWorldMatrix;
}

// カメラセッター
void Object3d::SetCamera(Camera* camera) {
    camera_ = camera;
}

// カメラゲッター
Camera* Object3d::GetCamera() const {
    return camera_;
}

// 新しいUpdateメソッド（カメラを使用）
void Object3d::Update() {
    assert(transformationMatrixData_);

    // アニメーション処理を段階的に有効化
    if (enableAnimation_ && animatedModel_ && animatedModel_->GetAnimationPlayer().GetAnimation().nodeAnimations.size() > 0) {
        // アニメーションが有効で、アニメーションデータが存在する場合のみ実行
        OutputDebugStringA("Object3d::Update - Animation processing enabled\n");
        Animation& animation = animatedModel_->GetAnimationPlayer().GetAnimation();
        Skeleton& skeleton = animatedModel_->GetSkeleton();
        SkinCluster& skinCluster = animatedModel_->GetSkinCluster();
        
        // デバッグ出力
        static int frameCount = 0;
        if (frameCount % 60 == 0) {
            OutputDebugStringA(("Object3d::Update - Animation time: " + std::to_string(animationTime_) + 
                               ", Duration: " + std::to_string(animation.duration) + 
                               ", Joints: " + std::to_string(skeleton.joints.size()) + 
                               ", NodeAnimations: " + std::to_string(animation.nodeAnimations.size()) + "\n").c_str());
        }
        frameCount++;
        
        ApplyAnimation(skeleton, animation, animationTime_);
        SkeletonUpdate(skeleton);
        SkinClusterUpdate(skinCluster, skeleton);  // スキニング処理を有効化
        
        // デバッグ：最初のジョイントの変換を確認
        if (skeleton.joints.size() > 0 && frameCount % 60 == 0) {
            const Joint& firstJoint = skeleton.joints[0];
            OutputDebugStringA(("Object3d::Update - First joint transform: " + firstJoint.name + 
                               " pos:(" + std::to_string(firstJoint.transform.translate.x) + "," +
                               std::to_string(firstJoint.transform.translate.y) + "," +
                               std::to_string(firstJoint.transform.translate.z) + ")\n").c_str());
        }
        
        // アニメーション行列は単位行列のままにする（スキニングで頂点変換するため）
        animationMatrix_ = MakeIdentity4x4();
        
        animationTime_ += 1.0f / 60.0f;
        animationTime_ = std::fmod(animationTime_, animation.duration);
    } else {
        // アニメーションが無効またはデータが存在しない場合
        static int noAnimCount = 0;
        if (noAnimCount % 60 == 0) {
            OutputDebugStringA(("Object3d::Update - Animation disabled for basic rendering\n"));
        }
        noAnimCount++;
    }

    // カメラが設定されている場合のみ処理
    if (!camera_) {
        // カメラが設定されていない場合は何もしない
        OutputDebugStringA("Object3d::Update - Camera is null, skipping matrix update\n");
        return;
    }

    Camera* useCamera = camera_;

    // ワールド行列の計算
    Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    
    // アニメーション行列とワールド行列を合成
    Matrix4x4 finalWorldMatrix = Multiply(animationMatrix_, worldMatrix);

    // WVP行列の計算（カメラからビュープロジェクション行列を取得）
    Matrix4x4 worldViewProjectionMatrix = Multiply(finalWorldMatrix, useCamera->GetViewProjectionMatrix());

    // 行列の更新
    transformationMatrixData_->WVP = worldViewProjectionMatrix;
    transformationMatrixData_->World = finalWorldMatrix;
}

void Object3d::Draw() {
    assert(dxCommon_);
    assert(model_);

    // SRVディスクリプタヒープの設定（重要！）
    // 現在はSpriteCommon経由で設定済みなので、追加の設定は不要
    // TODO: 将来的にはSrvManagerを直接参照できるようにする

    // パイプラインの設定
    if (enableAnimation_ && animatedModel_) {
        // スキニング用パイプラインを使用
        dxCommon_->GetCommandList()->SetGraphicsRootSignature(spriteCommon_->GetRootSignature().Get());
        dxCommon_->GetCommandList()->SetPipelineState(spriteCommon_->GetSkinningPipelineState().Get());
        dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    } else {
        // 通常の描画設定
        spriteCommon_->CommonDraw();
    }

    // アニメーションモデルの場合、頂点バッファとインフルエンスバッファを設定
    if (enableAnimation_ && animatedModel_) {
        AnimatedModel* animModel = static_cast<AnimatedModel*>(animatedModel_);
        const SkinCluster& skinCluster = animModel->GetSkinCluster();
        
        // 頂点バッファとインフルエンスバッファを設定
        D3D12_VERTEX_BUFFER_VIEW vbvs[2] = {
            model_->GetVBView(),
            skinCluster.influenceBufferView
        };
        dxCommon_->GetCommandList()->IASetVertexBuffers(0, 2, vbvs);
    } else {
        // 通常の頂点バッファのみセット
        dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &model_->GetVBView());
    }

    // マテリアルCBufferの場所を設定
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

    // 変換行列CBufferの場所を設定
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

    // テクスチャの場所を設定
    std::string texturePath = model_->GetTextureFilePath();
    OutputDebugStringA(("Object3d::Draw - Using texture path: " + texturePath + "\n").c_str());

    // テクスチャが空または存在しない場合の詳細なチェック
    if (texturePath.empty()) {
        OutputDebugStringA("Object3d::Draw - Texture path is empty, using default texture\n");
        TextureManager::GetInstance()->LoadDefaultTexture();
        texturePath = TextureManager::GetInstance()->GetDefaultTexturePath();
    }
    else if (!TextureManager::GetInstance()->IsTextureExists(texturePath)) {
        OutputDebugStringA(("Object3d::Draw - Texture does not exist in TextureManager: " + texturePath + "\n").c_str());

        // 試しにテクスチャを再ロードする
        bool loadSuccess = false;

        // ファイルの存在チェック
        DWORD fileAttributes = GetFileAttributesA(texturePath.c_str());
        if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
            // ファイルが存在する場合はロードを試みる
            OutputDebugStringA(("Object3d::Draw - File exists, trying to load texture: " + texturePath + "\n").c_str());
            TextureManager::GetInstance()->LoadTexture(texturePath);

            // 読み込みに成功したか確認
            if (TextureManager::GetInstance()->IsTextureExists(texturePath)) {
                OutputDebugStringA(("Object3d::Draw - Successfully loaded texture: " + texturePath + "\n").c_str());
                loadSuccess = true;
            }
        }

        // それでも失敗した場合はデフォルトテクスチャを使用
        if (!loadSuccess) {
            OutputDebugStringA("Object3d::Draw - Using default texture\n");
            TextureManager::GetInstance()->LoadDefaultTexture();
            texturePath = TextureManager::GetInstance()->GetDefaultTexturePath();
        }
    }
    else {
        OutputDebugStringA(("Object3d::Draw - Using valid texture: " + texturePath + "\n").c_str());
    }

    // テクスチャをセット（必ずテクスチャがセットされることを保証）
    dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2,
        TextureManager::GetInstance()->GetSrvHandleGPU(texturePath));

    // ライトCBufferの場所を設定
    dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

    // パレットSRVの設定（アニメーション用）
    if (enableAnimation_ && animatedModel_) {
        AnimatedModel* animModel = static_cast<AnimatedModel*>(animatedModel_);
        const SkinCluster& skinCluster = animModel->GetSkinCluster();
        
        if (skinCluster.paletteSrvHandle.second.ptr != 0) {
            dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(4, skinCluster.paletteSrvHandle.second);
        } else {
            // パレットSRVが無効な場合は警告
            OutputDebugStringA("Object3d::Draw - WARNING: Palette SRV is not set for animated model\n");
        }
    } else {
        // 通常のオブジェクトの場合、ダミーのSRVを設定（必要に応じて）
        // 現在のルートシグネチャではパレットSRVは必須なので、通常のオブジェクトでは問題になる
        // この問題は後で対処
    }

    // 描画
    dxCommon_->GetCommandList()->DrawInstanced(model_->GetVertexCount(), 1, 0, 0);
}

void Object3d::SkeletonUpdate(Skeleton& skeleton)
{
    // ← ここでサイズを合わせるのが絶対必要！！
    skeletonPose_.resize(skeleton.joints.size());
    //すべてのjointを更新。親が若いので通常ループで処理可能になっている
    for (Joint& joint : skeleton.joints)
    {
        joint.localMatrix = MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
        if (joint.parent)
        {
            joint.skeletonSpaceMatrix = Multiply(joint.localMatrix, skeleton.joints[*joint.parent].skeletonSpaceMatrix);

        } else
        {
            joint.skeletonSpaceMatrix = joint.localMatrix;
        }
        skeletonPose_[joint.index] = joint.skeletonSpaceMatrix;
    }
    
}

void Object3d::ApplyAnimation(Skeleton& skeleton, const Animation& animation, float animationTime)
{
    static int applyCount = 0;
    bool shouldDebug = (applyCount % 60 == 0);
    applyCount++;
    
    for (Joint& joint : skeleton.joints) {
        // 対象のJointのAnimationがあれば、値の適用を行う。
        // 下記のif文はC++17から可能になった初期化付きif文。
        if (auto it = animation.nodeAnimations.find(joint.name); it != animation.nodeAnimations.end()) {
            const NodeAnimation& nodeAnimation = it->second;

            joint.transform.translate = ::CalculateValue(nodeAnimation.translate, animationTime);
            joint.transform.rotate = ::CalculateValue(nodeAnimation.rotate, animationTime);
            joint.transform.scale = ::CalculateValue(nodeAnimation.scale, animationTime);
            
            if (shouldDebug) {
                OutputDebugStringA(("ApplyAnimation - Joint: " + joint.name + 
                                   ", translate: (" + std::to_string(joint.transform.translate.x) + 
                                   ", " + std::to_string(joint.transform.translate.y) + 
                                   ", " + std::to_string(joint.transform.translate.z) + ")\n").c_str());
            }
        }
    }
}

void Object3d::SkinClusterUpdate(SkinCluster& skinCluster, const Skeleton& skeleton)
{
    for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); ++jointIndex)
    {
        assert(jointIndex < skinCluster.inverseBindPoseMatrices.size());
        skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix =
            Multiply(skinCluster.inverseBindPoseMatrices[jointIndex], skeleton.joints[jointIndex].skeletonSpaceMatrix);
        skinCluster.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
            Transpose(Inverse(skinCluster.mappedPalette[jointIndex].skeletonSpaceMatrix));
    }
    
    // デバッグ出力
    static int debugCount = 0;
    if (debugCount % 60 == 0) {
        OutputDebugStringA(("SkinClusterUpdate - Updated " + std::to_string(skeleton.joints.size()) + " joint matrices\n").c_str());
        if (skeleton.joints.size() > 0) {
            OutputDebugStringA(("  First joint: " + skeleton.joints[0].name + 
                               ", translate: (" + std::to_string(skeleton.joints[0].transform.translate.x) + 
                               ", " + std::to_string(skeleton.joints[0].transform.translate.y) + 
                               ", " + std::to_string(skeleton.joints[0].transform.translate.z) + ")\n").c_str());
        }
    }
    debugCount++;
}

// CalculateValue関数はAnimationUtilityから使用するため、Object3dクラスからは削除

void Object3d::SetAnimatedModel(class AnimatedModel* animatedModel)
{
    animatedModel_ = animatedModel;
}

void Object3d::SetEnableAnimation(bool enable)
{
    enableAnimation_ = enable;
}

bool Object3d::GetEnableAnimation() const
{
    return enableAnimation_;
}

float Object3d::GetAnimationTime() const
{
    return animationTime_;
}

void Object3d::SetAnimationTime(float time)
{
    animationTime_ = time;
}