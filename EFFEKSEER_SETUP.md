# Effekseer セットアップガイド

実際にエフェクトを描画するためのEffekseerライブラリセットアップ手順です。

## 1. Effekseerライブラリのビルド

### 前提条件
- Visual Studio 2022
- CMake
- DirectX 12 SDK

### ビルド手順

1. **Effekseerソースの準備**
   ```bash
   cd externals/Effekseer
   ```

2. **CMakeでプロジェクト生成**
   ```bash
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64 -DEFK_BUILD_EXAMPLES=OFF
   ```

3. **Visual Studioでビルド**
   ```bash
   cmake --build . --config Debug
   cmake --build . --config Release
   ```

4. **ライブラリファイルを確認**
   - `build/src/Effekseer/Debug/Effekseer.lib`
   - `build/src/EffekseerRendererDX12/Debug/EffekseerRendererDX12.lib`

## 2. プロジェクト設定の更新

### vcxprojファイルの更新

**Debug構成:**
```xml
<AdditionalLibraryDirectories>
  $(ProjectDir)externals\Effekseer\build\src\Effekseer\Debug;
  $(ProjectDir)externals\Effekseer\build\src\EffekseerRendererDX12\Debug;
  %(AdditionalLibraryDirectories)
</AdditionalLibraryDirectories>

<AdditionalDependencies>
  Effekseer.lib;
  EffekseerRendererDX12.lib;
  %(AdditionalDependencies)
</AdditionalDependencies>
```

**Release構成:**
```xml
<AdditionalLibraryDirectories>
  $(ProjectDir)externals\Effekseer\build\src\Effekseer\Release;
  $(ProjectDir)externals\Effekseer\build\src\EffekseerRendererDX12\Release;
  %(AdditionalLibraryDirectories)
</AdditionalLibraryDirectories>

<AdditionalDependencies>
  Effekseer.lib;
  EffekseerRendererDX12.lib;
  %(AdditionalDependencies)
</AdditionalDependencies>
```

## 3. EffekseerManager.hの更新

以下のコメントアウトを解除：
```cpp
#include <Effekseer.h>
#include <EffekseerRendererDX12.h>
```

プレースホルダー定義を削除。

## 4. EffekseerManager.cppの実装完成

```cpp
bool EffekseerManager::Initialize(DirectXCommon* dxCommon) {
    assert(dxCommon);
    dxCommon_ = dxCommon;

    // Effekseerマネージャーの作成（最大8000パーティクル）
    efkManager_ = ::Effekseer::Manager::Create(8000);

    // DirectX12グラフィックスデバイスの作成
    auto graphicsDevice = ::EffekseerRendererDX12::CreateGraphicsDevice(
        dxCommon->GetDevice(), 
        dxCommon->GetCommandQueue(), 
        3
    );

    // レンダラーの作成
    auto format = DXGI_FORMAT_R8G8B8A8_UNORM;
    efkRenderer_ = ::EffekseerRendererDX12::Create(
        graphicsDevice, 
        &format, 
        1, 
        DXGI_FORMAT_UNKNOWN, 
        false, 
        8000
    );

    // レンダラーモジュールの設定
    efkManager_->SetSpriteRenderer(efkRenderer_->CreateSpriteRenderer());
    efkManager_->SetRibbonRenderer(efkRenderer_->CreateRibbonRenderer());
    efkManager_->SetRingRenderer(efkRenderer_->CreateRingRenderer());
    efkManager_->SetTrackRenderer(efkRenderer_->CreateTrackRenderer());
    efkManager_->SetModelRenderer(efkRenderer_->CreateModelRenderer());

    isInitialized_ = true;
    return true;
}
```

## 5. サンプルエフェクトファイルの配置

1. **Effekseer Editorで.efkefcファイル作成**
2. **Resources/Effectsディレクトリに配置**
   - `Laser01.efkefc`
   - `Explosion01.efkefc`

## 6. 実行とテスト

1. **プロジェクトをビルド**
2. **ゲーム実行**
3. **キー操作でエフェクトテスト**
   - L: レーザーエフェクト
   - E: 爆発エフェクト
   - Q: 全エフェクト停止

## トラブルシューティング

### よくある問題

**リンクエラー:**
- ライブラリパスが正しく設定されているか確認
- Debug/Release構成が一致しているか確認

**実行時エラー:**
- DirectX12デバイスが正しく初期化されているか確認
- エフェクトファイルパスが正しいか確認

**エフェクトが表示されない:**
- カメラの位置・向きを確認
- エフェクトの再生位置を確認
- Draw()の呼び出し順序を確認

## 現在の実装状況

- ✅ EffekseerManager クラス設計完了
- ✅ UnoEngine統合API完了
- ✅ GamePlayScene実装完了
- ⚠️ Effekseerライブラリビルド必要
- ⚠️ 実装コード完成必要

**現在はプレースホルダー実装のため、ログ出力のみ行われます。**
**実際の描画には上記セットアップが必要です。**