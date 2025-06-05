# FBXアニメーション実装

このプロジェクトはFBXファイルからアニメーションを読み込んで再生する機能を実装しています。

## 機能

- FBXファイルの読み込み（メッシュ、マテリアル、ボーン、アニメーション）
- スケルタルアニメーションの再生
- キーフレーム補間（線形補間）
- ボーン階層の変換計算
- スキンウェイトによる頂点変形

## 使用方法

### 基本的な使い方

```cpp
// FBXモデルの作成と初期化
auto fbxModel = std::make_unique<FBXModel>();
fbxModel->Initialize(dxCommon);

// FBXファイルの読み込み
if (fbxModel->LoadFromFile("Resources/Models/spider/Spider.fbx")) {
    // アニメーションの設定
    fbxModel->SetAnimation("Walk");  // アニメーション名を指定
    fbxModel->PlayAnimation(true);   // true = ループ再生
}

// 毎フレームの更新
void Update(float deltaTime) {
    fbxModel->Update(deltaTime);
}

// 描画時にボーン行列を取得
const auto& boneMatrices = fbxModel->GetBoneMatrices();
// シェーダーにボーン行列を渡す
```

### AnimatedObject3dクラスの使用

```cpp
// アニメーション付き3Dオブジェクトの作成
auto animatedObject = std::make_unique<AnimatedObject3d>();
animatedObject->Initialize();

// FBXモデルの設定
animatedObject->SetFBXModel(fbxModel.get());

// 描画
animatedObject->Draw(viewProjectionMatrix);
```

## 実装詳細

### FBXLoader（FBX SDK使用時）
- Autodesk FBX SDKを使用した本格的なローダー
- 複雑なアニメーションやボーン階層に対応
- FBX SDKのインストールが必要（詳細は`FBX_SDK_SETUP.md`参照）

### SimpleFBXLoader（FBX SDKなし）
- FBX SDKが利用できない場合の簡易実装
- テスト用のアニメーション付きキューブモデルを生成
- 基本的なアニメーション機能の検証用

### アニメーションデータ構造

```cpp
struct AnimationKey {
    float time;         // キーフレーム時刻
    Vector3 position;   // 位置
    Vector4 rotation;   // 回転（クォータニオン）
    Vector3 scale;      // スケール
};

struct AnimationChannel {
    std::string boneName;           // ボーン名
    std::vector<AnimationKey> keys; // キーフレーム配列
};

struct Animation {
    std::string name;               // アニメーション名
    float duration;                 // 継続時間
    float ticksPerSecond;           // 再生速度
    std::unordered_map<std::string, AnimationChannel> channels;
};
```

### シェーダー対応

頂点シェーダー（AnimatedObject3d.VS.hlsl）:
```hlsl
// ボーン変換
float4x4 skinMatrix = 
    input.boneWeights.x * boneMatrices[input.boneIndices.x] +
    input.boneWeights.y * boneMatrices[input.boneIndices.y] +
    input.boneWeights.z * boneMatrices[input.boneIndices.z] +
    input.boneWeights.w * boneMatrices[input.boneIndices.w];

// 頂点変換
float4 skinnedPos = mul(float4(input.position, 1.0f), skinMatrix);
```

## トラブルシューティング

### FBXファイルが読み込めない
- ファイルパスを確認
- FBXファイルのバージョン（2020.x推奨）
- FBX SDKが正しくインストールされているか確認

### アニメーションが再生されない
- アニメーション名が正しいか確認
- `PlayAnimation()`を呼んでいるか確認
- `Update()`を毎フレーム呼んでいるか確認

### 変形がおかしい
- ボーンウェイトの合計が1になっているか確認
- ボーンインデックスが正しい範囲内か確認
- ボーン行列が正しく計算されているか確認

## 今後の拡張

- ブレンドシェイプアニメーション対応
- 複数アニメーションのブレンド
- アニメーションイベントシステム
- LODシステムの実装