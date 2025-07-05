#include "AnimatedModel.h"
#include "Mymath.h"
#include <algorithm>
#include <cctype>

// コンストラクタ
AnimatedModel::AnimatedModel() : rootNodeName_("root") {
}

// デストラクタ
AnimatedModel::~AnimatedModel() {
}

// 初期化
void AnimatedModel::Initialize(DirectXCommon* dxCommon) {
    Model::Initialize(dxCommon);
}

// モデルとアニメーションの読み込み
void AnimatedModel::LoadFromFile(const std::string& directoryPath, const std::string& filename) {
    OutputDebugStringA(("AnimatedModel: Loading from " + directoryPath + "/" + filename + "\n").c_str());
    
    // ファイル拡張子をチェック
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == "gltf") {
        // GLTFファイルの場合
        LoadFromGLTF(directoryPath, filename);
    } else {
        // OBJファイルの場合
        LoadFromObj(directoryPath, filename);
        
        // マテリアル情報のデバッグ出力
        const ModelData& modelData = GetModelData();
        OutputDebugStringA(("AnimatedModel: Texture path: " + modelData.material.textureFilePath + "\n").c_str());
        
        // マテリアル情報が空の場合、デフォルト値を設定
        ModelData& modelDataInternal2 = GetModelDataInternal();
        if (modelDataInternal2.material.textureFilePath.empty()) {
            OutputDebugStringA("AnimatedModel: No texture path found, setting default values\n");
            // デフォルトのマテリアル設定
            modelDataInternal2.material.textureFilePath = "Resources/uvChecker.png";
            modelDataInternal2.material.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
            OutputDebugStringA(("AnimatedModel: Set default texture: " + modelDataInternal2.material.textureFilePath + "\n").c_str());
        }
        
        // ルートノード名を設定
        ModelData& modelDataInternal = GetModelDataInternal();
        if (!modelDataInternal.rootNode.name.empty()) {
            rootNodeName_ = modelDataInternal.rootNode.name;
        } else {
            modelDataInternal.rootNode.name = "root";
            modelDataInternal.rootNode.localMatrix = MakeIdentity4x4();
            rootNodeName_ = "root";
        }
    }
    
    OutputDebugStringA(("AnimatedModel: Root node name set to: " + rootNodeName_ + "\n").c_str());
    
    // ダミーアニメーションを読み込み
    LoadAnimation(directoryPath, filename);
}

// GLTFファイルからの読み込み
void AnimatedModel::LoadFromGLTF(const std::string& directoryPath, const std::string& filename) {
    OutputDebugStringA(("AnimatedModel: Loading GLTF from " + directoryPath + "/" + filename + "\n").c_str());
    
    // 現在はGLTFローダーが実装されていないため、
    // AnimatedCube_BaseColor.pngを使用した簡易的なキューブモデルを作成
    
    // モデルデータを手動で作成（キューブの頂点データ）
    ModelData& modelData = GetModelDataInternal();
    
    // 簡易キューブの頂点データを作成（各面ごとに独立した頂点）
    modelData.vertices.clear();
    
    // 各面ごとに4頂点 × 6面 = 24頂点を作成し、2つの三角形で1つの面を構成
    
    // 前面 (+Z) - 時計回り
    modelData.vertices.push_back({{-0.5f, -0.5f,  0.5f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}});
    modelData.vertices.push_back({{ 0.5f, -0.5f,  0.5f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}});
    modelData.vertices.push_back({{ 0.5f,  0.5f,  0.5f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}});
    modelData.vertices.push_back({{ 0.5f,  0.5f,  0.5f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}});
    modelData.vertices.push_back({{-0.5f,  0.5f,  0.5f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}});
    modelData.vertices.push_back({{-0.5f, -0.5f,  0.5f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}});
    
    // 背面 (-Z) - 時計回り
    modelData.vertices.push_back({{ 0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}});
    modelData.vertices.push_back({{-0.5f, -0.5f, -0.5f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}});
    modelData.vertices.push_back({{-0.5f,  0.5f, -0.5f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}});
    modelData.vertices.push_back({{-0.5f,  0.5f, -0.5f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}});
    modelData.vertices.push_back({{ 0.5f,  0.5f, -0.5f, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}});
    modelData.vertices.push_back({{ 0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}});
    
    // 左面 (-X) - 時計回り
    modelData.vertices.push_back({{-0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}});
    modelData.vertices.push_back({{-0.5f, -0.5f,  0.5f, 1.0f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}});
    modelData.vertices.push_back({{-0.5f,  0.5f,  0.5f, 1.0f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}});
    modelData.vertices.push_back({{-0.5f,  0.5f,  0.5f, 1.0f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}});
    modelData.vertices.push_back({{-0.5f,  0.5f, -0.5f, 1.0f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}});
    modelData.vertices.push_back({{-0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}});
    
    // 右面 (+X) - 時計回り
    modelData.vertices.push_back({{ 0.5f, -0.5f,  0.5f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}});
    modelData.vertices.push_back({{ 0.5f, -0.5f, -0.5f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}});
    modelData.vertices.push_back({{ 0.5f,  0.5f, -0.5f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});
    modelData.vertices.push_back({{ 0.5f,  0.5f, -0.5f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});
    modelData.vertices.push_back({{ 0.5f,  0.5f,  0.5f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});
    modelData.vertices.push_back({{ 0.5f, -0.5f,  0.5f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}});
    
    // 上面 (+Y) - 時計回り
    modelData.vertices.push_back({{-0.5f,  0.5f,  0.5f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}});
    modelData.vertices.push_back({{ 0.5f,  0.5f,  0.5f, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}});
    modelData.vertices.push_back({{ 0.5f,  0.5f, -0.5f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}});
    modelData.vertices.push_back({{ 0.5f,  0.5f, -0.5f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}});
    modelData.vertices.push_back({{-0.5f,  0.5f, -0.5f, 1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}});
    modelData.vertices.push_back({{-0.5f,  0.5f,  0.5f, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}});
    
    // 下面 (-Y) - 時計回り
    modelData.vertices.push_back({{-0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}});
    modelData.vertices.push_back({{ 0.5f, -0.5f, -0.5f, 1.0f}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}});
    modelData.vertices.push_back({{ 0.5f, -0.5f,  0.5f, 1.0f}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}});
    modelData.vertices.push_back({{ 0.5f, -0.5f,  0.5f, 1.0f}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}});
    modelData.vertices.push_back({{-0.5f, -0.5f,  0.5f, 1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}});
    modelData.vertices.push_back({{-0.5f, -0.5f, -0.5f, 1.0f}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}});
    
    OutputDebugStringA(("AnimatedModel: Created " + std::to_string(modelData.vertices.size()) + " vertices for cube\n").c_str());
    
    // マテリアル設定（AnimatedCube_BaseColor.pngを使用）
    modelData.material.textureFilePath = directoryPath + "/" + "AnimatedCube_BaseColor.png";
    modelData.material.diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
    modelData.material.ambient = {0.2f, 0.2f, 0.2f, 1.0f};
    modelData.material.specular = {0.5f, 0.5f, 0.5f, 1.0f};
    modelData.material.alpha = 1.0f;
    
    // ルートノード設定
    modelData.rootNode.name = "AnimatedCube";
    modelData.rootNode.localMatrix = MakeIdentity4x4();
    rootNodeName_ = "AnimatedCube";
    
    OutputDebugStringA(("AnimatedModel: Created cube with texture: " + modelData.material.textureFilePath + "\n").c_str());
    
    // 頂点バッファを作成
    CreateVertexBuffer();
}

// アニメーションの読み込み
void AnimatedModel::LoadAnimation(const std::string& directoryPath, const std::string& filename) {
    // LoadAnimationFile関数を使用してアニメーションを読み込み
    animation_ = LoadAnimationFile(directoryPath, filename);
    
    // アニメーションプレイヤーに設定
    animationPlayer_.SetAnimation(animation_);
    animationPlayer_.SetLoop(true);
}

// 更新（アニメーション時刻を進める）
void AnimatedModel::Update(float deltaTime) {
    animationPlayer_.Update(deltaTime);
}

// アニメーションのローカル変換行列を取得
Matrix4x4 AnimatedModel::GetAnimationLocalMatrix() {
    // GLTFファイルの場合はAnimatedCubeノード、それ以外はrootノードを使用
    return animationPlayer_.GetLocalMatrix(rootNodeName_);
}

// アニメーション再生制御
void AnimatedModel::PlayAnimation() {
    animationPlayer_.Play();
}

void AnimatedModel::StopAnimation() {
    animationPlayer_.Stop();
}

void AnimatedModel::PauseAnimation() {
    animationPlayer_.Pause();
}

void AnimatedModel::SetAnimationLoop(bool loop) {
    animationPlayer_.SetLoop(loop);
}