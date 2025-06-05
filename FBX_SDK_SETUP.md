# FBX SDK セットアップガイド

このプロジェクトでFBXアニメーションを使用するには、Autodesk FBX SDKをインストールする必要があります。

**注意**: FBX SDKがインストールされていない場合でも、SimpleFBXLoaderを使用してプロジェクトは動作します。

## 1. FBX SDKのダウンロード

1. [Autodesk FBX SDK](https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-3)にアクセス
2. Windows版をダウンロード（VS2022対応版）
3. ダウンロードしたインストーラーを実行

## 2. インストール

1. デフォルトのインストール先: `C:\Program Files\Autodesk\FBX\FBX SDK\2020.3.4`
2. インストール完了後、以下のフォルダ構造をプロジェクトに作成:

```
MyEngineGame/
├── externals/
│   └── fbxsdk/
│       ├── include/     (FBX SDKのincludeフォルダの内容をコピー)
│       └── lib/
│           └── x64/
│               ├── debug/   (libfbxsdk-md.libをコピー)
│               └── release/ (libfbxsdk-mt.libをコピー)
```

## 3. ファイルのコピー

### インクルードファイル
- `C:\Program Files\Autodesk\FBX\FBX SDK\2020.3.4\include` の内容を
- `externals/fbxsdk/include/` にコピー

### ライブラリファイル (Debug)
- `C:\Program Files\Autodesk\FBX\FBX SDK\2020.3.4\lib\vs2022\x64\debug\libfbxsdk-md.lib` を
- `externals/fbxsdk/lib/x64/debug/` にコピー

### ライブラリファイル (Release)
- `C:\Program Files\Autodesk\FBX\FBX SDK\2020.3.4\lib\vs2022\x64\release\libfbxsdk-mt.lib` を
- `externals/fbxsdk/lib/x64/release/` にコピー

## 4. プロジェクト設定の確認

プロジェクトファイル（CG2_00-01.vcxproj）には既に以下の設定が追加されています：

- インクルードパス: `$(ProjectDir)externals\fbxsdk\include`
- ライブラリパス (Debug): `$(ProjectDir)externals\fbxsdk\lib\x64\debug`
- ライブラリパス (Release): `$(ProjectDir)externals\fbxsdk\lib\x64\release`
- プリプロセッサ定義: `FBXSDK_SHARED`（手動で追加が必要）
- リンクライブラリ (Debug): `libfbxsdk-md.lib`
- リンクライブラリ (Release): `libfbxsdk-mt.lib`

## 5. FBX SDKを有効化

FBX SDKをインストールした後、以下の手順で有効化します：

1. `src/Engine/Graphics/FBXModel.cpp` を開く
2. 8行目の `//#define USE_FBX_SDK` のコメントを外す：
   ```cpp
   #define USE_FBX_SDK
   ```
3. プロジェクトファイル（CG2_00-01.vcxproj）を開く
4. `PreprocessorDefinitions` に `FBXSDK_SHARED` を追加：
   - Debug: `_DEBUG;_WINDOWS;FBXSDK_SHARED;%(PreprocessorDefinitions)`
   - Release: `NDEBUG;_WINDOWS;FBXSDK_SHARED;%(PreprocessorDefinitions)`

## 6. ビルドとテスト

1. Visual Studio 2022でソリューションを開く
2. ビルド構成を「Debug」または「Release」に設定
3. ソリューションをビルド

## トラブルシューティング

### ビルドエラーが発生する場合

1. FBX SDKのバージョンを確認（2020.3.x推奨）
2. ライブラリファイルのパスが正しいか確認
3. ランタイムライブラリの設定を確認:
   - Debug: マルチスレッド デバッグ DLL (/MDd)
   - Release: マルチスレッド DLL (/MD)

### FBX SDKが見つからない場合

プロジェクトはFBX SDKなしでも動作するように設計されています。
FBX SDKが利用できない場合は、SimpleFBXLoaderが使用されます。

## 使用方法

```cpp
// FBXモデルの読み込み
auto fbxModel = std::make_unique<FBXModel>();
fbxModel->Initialize(dxCommon);
fbxModel->LoadFromFile("Resources/Models/animated_model.fbx");

// アニメーションの再生
fbxModel->PlayAnimation(true);

// 毎フレーム更新
fbxModel->Update(deltaTime);
```