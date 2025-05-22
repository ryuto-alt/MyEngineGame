# 円形エフェクト実装ガイド

このプロジェクトでは、`gradationLine.png`テクスチャと`Ring`プリミティブを組み合わせて、美しい円形エフェクトを実現しています。

## 実装された修正内容

### 1. SamplerのAddressV設定の修正
**ファイル**: `src/Engine/Graphics/SpriteCommon.cpp`

```cpp
// 変更前
staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

// 変更後  
staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
```

**効果**: 中心に白い線が見えるアーティファクトを回避

### 2. RingクラスのUV座標改善
**ファイル**: `src/Engine/Graphics/Ring.cpp`

- gradationLine.pngテクスチャに最適化されたUV座標設定
- 外側の頂点: `v=0.0` (テクスチャの外端)
- 内側の頂点: `v=1.0` (テクスチャの内端)
- 円周方向に`u`座標を展開

### 3. 円形エフェクト専用クラスの実装
**新規ファイル**: 
- `src/Engine/Graphics/CircleEffect.h`
- `src/Engine/Graphics/CircleEffect.cpp`

**機能**:
- gradationLine.pngテクスチャの自動読み込み
- UVスクロールアニメーション
- Transform制御（位置、回転、スケール）
- 色変更とライティング制御

### 4. Object3dクラスの拡張
**ファイル**: `src/Engine/Graphics/Object3d.h/.cpp`

**追加メソッド**:
```cpp
void SetCircleEffectTexture();    // gradationLine.png設定
void SetUVScroll(float u, float v); // UVスクロール制御
```

## 使用方法

### 基本的な円形エフェクトの作成

```cpp
// 1. CircleEffectインスタンスの作成
auto circleEffect = std::make_unique<CircleEffect>();
circleEffect->Initialize(dxCommon, spriteCommon);
circleEffect->SetCamera(camera);

// 2. エフェクトの設定
circleEffect->CreateCircleEffect(
    1.0f,  // 外径
    0.2f,  // 内径
    32     // 分割数
);

// 3. 位置と色の設定
circleEffect->SetPosition({0.0f, 0.0f, 0.0f});
circleEffect->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

// 4. UVスクロールアニメーション開始
circleEffect->StartUVAnimation(0.02f, 0.0f); // U方向スクロール

// 5. 毎フレーム更新と描画
circleEffect->Update();
circleEffect->Draw();
```

### 様々なエフェクトバリエーション

```cpp
// 細いリング（境界線効果）
effect->CreateCircleEffect(1.0f, 0.9f, 64);

// 太いリング（大きなエフェクト）
effect->CreateCircleEffect(2.0f, 0.1f, 32);

// 縦方向スクロール
effect->StartUVAnimation(0.0f, 0.03f);

// 回転アニメーション
Vector3 rotation = effect->GetRotation();
rotation.z += rotationSpeed * deltaTime;
effect->SetRotation(rotation);
```

## テクニカルノート

### UVScrollの原理
- `AddressV = CLAMP`により、v座標が0.0-1.0の範囲外でクランプされる
- UV変換行列で平行移動を適用してスクロール効果を実現
- エフェクトには欠かせない技術で、市販ゲームでも頻繁に使用

### Ringプリミティブの利点
- 平面に円テクスチャを適用するより細かい解像度のテクスチャに見せかけられる
- UVを円方向にScaleすることで様々な応用が可能
- ParticleSystemとの組み合わせで複雑なエフェクトを実現

### パフォーマンス最適化
- 分割数（divisions）を調整してポリゴン数を制御
- 遠距離のエフェクトは低分割、近距離は高分割を使用
- 不要なエフェクトは`SetVisible(false)`で非表示に

## デモシーンの実行

**ファイル**: `src/Game/scene/CircleEffectDemoScene.h/.cpp`

デモシーンでは以下のエフェクトを確認できます：

1. **基本的な円形エフェクト** - 静的な円形表示
2. **横方向UVスクロール** - 水平方向のアニメーション
3. **縦方向UVスクロール** - 垂直方向のアニメーション  
4. **細いリング** - 境界線効果
5. **回転アニメーション** - 回転 + UVスクロール
6. **グラデーション効果** - 色変化
7. **大きな背景エフェクト** - 脈動アニメーション
8. **高速スクロール** - 高速移動エフェクト
9. **複合エフェクト** - 色変化 + 複合アニメーション

## プロジェクト構成

```
MyEngineGame/
├── src/Engine/Graphics/
│   ├── Ring.h/.cpp              # Ringプリミティブ
│   ├── AdvancedRing.h/.cpp      # 拡張Ring機能
│   ├── RingManager.h/.cpp       # Ring管理システム
│   ├── CircleEffect.h/.cpp      # 円形エフェクトクラス（新規）
│   ├── Object3d.h/.cpp          # 拡張されたObject3d
│   ├── SpriteCommon.cpp         # Sampler設定修正
│   └── Model.h                  # テクスチャパス設定追加
├── src/Game/scene/
│   └── CircleEffectDemoScene.h/.cpp # デモシーン（新規）
└── Resources/particle/
    └── gradationLine.png        # エフェクト用テクスチャ
```

## 今後の拡張案

1. **Billboardエフェクト** - 常にカメラを向く効果
2. **Particle System連携** - 複数エフェクトの統合管理
3. **シェーダーカスタマイズ** - より高度な視覚効果
4. **アニメーションカーブ** - より自然なアニメーション
5. **エフェクトプリセット** - よく使用される設定の保存

## 参考情報

この実装は以下の概念に基づいています：

- **UVスクロール**: エフェクトに欠かせない基本技術
- **テクスチャサンプリング**: CLAMP vs WRAP の使い分け  
- **プリミティブ最適化**: Ringによる効率的なエフェクト表現
- **ゲーム業界標準**: 市販ゲームでも多用される手法

ゲーム制作において、これらの技術をうまく応用してクオリティの高いエフェクトを実現してください。
