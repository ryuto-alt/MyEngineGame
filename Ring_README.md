# Ring プリミティブ実装

## 概要
スライドで説明されたRingプリミティブをMyEngineGameプロジェクトに実装しました。
Ringはエフェクトでよく利用される円を真ん中でくりぬいたような形状のプリミティブです。

## 実装ファイル

### 基本クラス
- **Ring.h/cpp**: 基本的なRingプリミティブ
- **AdvancedRing.h/cpp**: 拡張機能付きRingプリミティブ
- **RingManager.h/cpp**: Ring管理クラス（シングルトン）
- **RingSample.h/cpp**: 使用例とサンプル実装

## 機能

### 基本Ring (Ring.h)
- 外径・内径・分割数の指定
- 時計回りの三角形生成
- DirectX12対応の頂点バッファ生成

```cpp
// 基本的な使用例
Ring ring;
ring.Initialize(dxCommon);
ring.Generate(1.0f, 0.2f, 32); // 外径1.0, 内径0.2, 32分割
```

### 拡張Ring (AdvancedRing.h)
- 部分円の作成（開始角度・終了角度指定）
- UV方向の選択（Horizontal/Vertical）
- 頂点カラー対応（外側・内側それぞれ）
- UVScrollに対応した設計

```cpp
// 拡張Ringの使用例
AdvancedRing ring;
ring.Initialize(dxCommon);
ring.Generate(
    1.0f, 0.2f, 32,                    // 基本パラメータ
    0.0f, PI,                          // 半円（0度から180度）
    AdvancedRing::UVDirection::Vertical, // v方向スクロール
    {1.0f, 0.0f, 0.0f, 1.0f},         // 外側：赤
    {0.0f, 0.0f, 1.0f, 1.0f}          // 内側：青
);
```

### Ring Manager
複数のRingを効率的に管理し、よく使用されるプリセットを提供

```cpp
// RingManagerの使用例
RingManager::GetInstance()->Initialize(dxCommon);
RingManager::GetInstance()->CreateCommonPresets();

// プリセットの使用
Ring* basicRing = RingManager::GetInstance()->GetRing("effect_basic");
AdvancedRing* gradientRing = RingManager::GetInstance()->GetAdvancedRing("effect_gradient");
```

## スライドで説明された機能の実装

### 1. 基本的なリング形状
- 分割数による円の精度調整
- 外径・内径による太さ調整
- 時計回りの三角形配置

### 2. UVScrollエフェクト
- gradationLine.pngテクスチャとの組み合わせ
- UV方向の切り替え（u方向/v方向）
- AddressV設定による表示制御

### 3. ParticleSystemとの組み合わせ
- Emitterタイミングに合わせた設定
- 点の動きを制御するシステムとの連携
- ModelParticle等への応用

### 4. 石のようなエフェクト
- PlaneとRingの組み合わせ
- ビルボード有り/無しの使い分け
- 半透明とカラーグラデーション

## プリセット一覧

### 基本Ring
- `effect_basic`: 標準的なエフェクト用（外径1.0, 内径0.2）
- `effect_thin`: 細いリング（境界線効果）
- `effect_thick`: 太いリング（大きなエフェクト）
- `effect_smooth`: 高解像度リング（128分割）

### 拡張Ring
- `effect_semicircle`: 半円リング（爆発エフェクト用）
- `effect_vertical_scroll`: 縦スクロール用リング
- `effect_gradient`: グラデーション効果リング
- `effect_slice`: 部分円リング（90度スライス）

## 使用方法

### 1. プロジェクトに組み込む
```cpp
#include "RingManager.h"

// 初期化
RingManager::GetInstance()->Initialize(dxCommon);
RingManager::GetInstance()->CreateCommonPresets();
```

### 2. エフェクトで使用
```cpp
// 基本的なリングエフェクト
Ring* ring = RingManager::GetInstance()->GetRing("effect_basic");

// Object3Dなどでレンダリング
object3d->SetModel(ring); // 頂点データを設定
object3d->Draw();
```

### 3. カスタムリングの作成
```cpp
// カスタムリングの作成
AdvancedRing* customRing = RingManager::GetInstance()->CreateAdvancedRing(
    "my_custom_ring",
    2.0f,    // 外径
    0.5f,    // 内径  
    64,      // 分割数
    0.0f, 2.0f * PI,                    // 全円
    AdvancedRing::UVDirection::Horizontal, // 水平スクロール
    {1.0f, 1.0f, 1.0f, 1.0f},          // 外側カラー
    {1.0f, 1.0f, 1.0f, 1.0f}           // 内側カラー
);
```

## 技術的な特徴

### パフォーマンス最適化
- 頂点バッファの効率的な作成
- 重複頂点の削除
- 分割数に応じた適応的な処理

### エフェクト対応
- UVScrollに最適化されたUV座標
- 市販ゲームレベルの品質
- Shaderとの連携を考慮した設計

### 拡張性
- 新しいRingタイプの追加が容易
- カラー、アニメーション対応
- ゲーム制作での応用を想定

## 注意事項

### テクスチャ設定
```hlsl
// シェーダーでのUVScroll設定例
staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
// または
staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
```

### ビルボード対応
- カメラに常に向くエフェクト
- 固定位置のエフェクト
- 用途に応じた使い分け

## 今後の拡張

### 追加予定機能
- アニメーション機能の統合
- より高度なパーティクルシステム連携
- テクスチャアトラス対応
- インスタンシング対応

スライドの内容を完全に再現し、さらにゲーム制作で実際に使いやすいように設計されています。
ぜひエフェクト制作にご活用ください！