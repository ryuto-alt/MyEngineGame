# Human Walk Animation Implementation

## 概要
SoraEngine-Skininngプロジェクトを参考に、MyEngineGameプロジェクトにhumanのwalk.gltfモデルのアニメーション描画システムを実装しました。

## 実装内容

### 1. 新規作成ファイル

#### AnimatedHumanController.h
- **場所**: `src/Engine/Animation/AnimatedHumanController.h`
- **機能**: アニメーション付きヒューマンモデルを制御するクラス
- **主な機能**:
  - walk.gltfファイルの読み込み
  - アニメーションの再生・一時停止・リセット
  - 位置・回転・スケール・色の設定
  - アニメーション速度の調整

#### AnimatedHumanController.cpp
- **場所**: `src/Engine/Animation/AnimatedHumanController.cpp`
- **機能**: AnimatedHumanControllerの実装
- **主な処理**:
  - アニメーションモデルの初期化
  - 毎フレームのアニメーション更新
  - 3Dオブジェクトの描画
  - アニメーションループの管理

### 2. 既存ファイル更新

#### GamePlayScene.h
- **追加内容**: 
  - `#include "AnimatedHumanController.h"`
  - `std::unique_ptr<AnimatedHumanController> humanController_;`メンバー変数

#### GamePlayScene.cpp
- **追加内容**:
  - 初期化処理でAnimatedHumanControllerを作成
  - 更新処理でヒューマンモデルの移動とアニメーション制御
  - 描画処理でヒューマンモデルの描画
  - 終了処理でリソースの解放

### 3. プロジェクトファイル更新

#### CG2_00-01.vcxproj
- **追加内容**:
  - `<ClCompile Include="src\Engine\Animation\AnimatedHumanController.cpp" />`
  - `<ClInclude Include="src\Engine\Animation\AnimatedHumanController.h" />`

#### CG2_00-01.vcxproj.filters
- **追加内容**:
  - `src\engine\Animation`フィルターの追加
  - AnimatedHumanControllerファイルの適切な配置

## 操作方法

### カメラ操作
- **WASD**: カメラの水平移動
- **SPACE**: カメラの上昇
- **SHIFT**: カメラの下降

### ヒューマンモデル操作
- **↑↓←→**: ヒューマンモデルの移動
- **P**: アニメーションの一時停止/再開
- **R**: アニメーションのリセット

### その他
- **ESC**: アプリケーション終了

## 技術的詳細

### アニメーションシステム
- **フレームレート**: 60FPS
- **アニメーション時間**: 自動的にループ
- **スキニング**: ボーンウェイトベースのスキニング処理
- **行列計算**: アニメーション行列の取得と適用

### 使用技術
- **glTF**: 3Dモデルフォーマット
- **Assimp**: 3Dモデル読み込みライブラリ
- **DirectX 12**: 3Dグラフィックス
- **ImGui**: デバッグ用UI

## ファイル構造

```
src/
├── Engine/
│   └── Animation/
│       ├── AnimatedHumanController.h    (新規作成)
│       ├── AnimatedHumanController.cpp  (新規作成)
│       ├── AnimatedModel.h             (既存)
│       ├── AnimatedModel.cpp           (既存)
│       └── ...
└── Game/
    └── scene/
        ├── GamePlayScene.h             (更新)
        └── GamePlayScene.cpp           (更新)
```

## 使用リソース
- **3Dモデル**: `Resources/Models/human/walk.gltf`
- **テクスチャ**: `Resources/Models/human/white.png`
- **アニメーションデータ**: `Resources/Models/human/walk.bin`

## 動作確認
1. プロジェクトをビルドしてください
2. アプリケーションを実行してください
3. 画面に歩行アニメーションするヒューマンモデルが表示されることを確認してください
4. 各種操作が正常に動作することを確認してください

## 注意事項
- walk.gltfファイルが存在しない場合、初期化でエラーが発生します
- アニメーションの継続時間は自動的に取得されます
- メモリリークを避けるため、適切にリソースを解放しています

## 今後の拡張可能性
- 複数のアニメーションの切り替え
- 物理シミュレーションとの統合
- 複数のヒューマンモデルの表示
- アニメーションブレンディング
- カスタムアニメーションの読み込み