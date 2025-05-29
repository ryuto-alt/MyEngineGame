# 3Dパーティクルシステム実装完了

## 概要

effect.objファイルを使用した3Dモデルベースのヒットエフェクトシステムを実装しました。

## 実装されたファイル

### コアシステム
- `HitEffect3D.h/cpp` - 3Dヒットエフェクトクラス
- `EffectManager3D.h/cpp` - 3Dエフェクトマネージャー（シングルトン）
- `Particle3DDemo.h/cpp` - デモ・テスト用クラス

### 既存システムとの統合
- `UnoEngine.h/cpp` - エンジンに3Dパーティクルシステムを統合

## エフェクトの種類

### 1. Normal（通常ヒット）
- **色**: 黄色 → オレンジ
- **パーティクル数**: 8個
- **寿命**: 0.3～0.8秒
- **速度範囲**: ±2.0f

### 2. Critical（クリティカルヒット）
- **色**: 赤色 → 深い赤
- **パーティクル数**: 15個
- **寿命**: 0.5～1.2秒
- **速度範囲**: ±4.0f

### 3. Impact（衝撃）
- **色**: 青色 → 深い青
- **パーティクル数**: 12個
- **寿命**: 0.4～1.0秒
- **速度範囲**: ±6.0f

### 4. Explosion（爆発）
- **色**: オレンジ → 暗い赤
- **パーティクル数**: 25個
- **寿命**: 0.8～1.8秒
- **速度範囲**: ±8.0f

### 5. Lightning（雷撃）
- **色**: 薄い青白 → 青
- **パーティクル数**: 20個
- **寿命**: 0.2～0.6秒
- **速度範囲**: ±10.0f
- **特徴**: 縦長のスケール（電撃のような見た目）

## 使用方法

### 基本的な使用例

```cpp
#include "EffectManager3D.h"

// 初期化（エンジンで自動的に実行）
EffectManager3D::GetInstance()->Initialize();

// エフェクトの発生
Vector3 hitPosition = {0.0f, 0.0f, 0.0f};

// 通常ヒット
EffectManager3D::GetInstance()->PlayNormalHit(hitPosition);

// クリティカルヒット
EffectManager3D::GetInstance()->PlayCriticalHit(hitPosition);

// 衝撃エフェクト
EffectManager3D::GetInstance()->PlayImpactHit(hitPosition);

// 爆発エフェクト
EffectManager3D::GetInstance()->PlayExplosion(hitPosition);

// 雷撃エフェクト
EffectManager3D::GetInstance()->PlayLightningHit(hitPosition);

// カスタムエフェクト
EffectManager3D::GetInstance()->TriggerHitEffect(hitPosition, HitEffect3D::EffectType::Critical);
```

### ゲームシーンでの実装例

```cpp
class GameScene : public IScene {
private:
    Particle3DDemo particleDemo_;  // デモクラスを使用

public:
    void Initialize() override {
        // デモクラスの初期化
        particleDemo_.Initialize();
    }

    void Update() override {
        // デモクラスの更新
        particleDemo_.Update();
        
        // 敵にヒットした場合のエフェクト例
        if (bulletHitEnemy) {
            EffectManager3D::GetInstance()->PlayNormalHit(enemyPosition);
        }
        
        // ボスにクリティカルヒットした場合
        if (criticalHitBoss) {
            EffectManager3D::GetInstance()->PlayCriticalHit(bossPosition);
        }
    }

    void Draw() override {
        // 3Dパーティクルの描画は UnoEngine で自動的に実行される
    }

    void DrawImGui() override {
        // デモのImGuiを表示
        particleDemo_.DrawImGui();
    }
};
```

## キーボード操作（デモモード）

- **1キー**: Normal Hit エフェクト
- **2キー**: Critical Hit エフェクト  
- **3キー**: Impact Hit エフェクト
- **4キー**: Explosion エフェクト
- **5キー**: Lightning Hit エフェクト
- **スペースキー**: ランダムエフェクトを発生
- **Escキー**: 全エフェクト停止

## ImGuiコントロール

### エフェクト設定
- **Position**: エフェクト発生位置の調整
- **Type**: エフェクトタイプの選択
- **Trigger Selected Effect**: 選択したエフェクトを発生
- **Stop All Effects**: 全エフェクト停止

### 自動発生設定
- **Auto Trigger**: 自動でランダムエフェクトを発生
- **Interval**: 発生間隔の調整（0.5～5.0秒）

### デバッグ情報
- **Active Effects**: 現在アクティブなエフェクト数
- **Any Playing**: エフェクトが再生中かどうか
- **Detailed Debug**: 詳細なデバッグ情報

## 特徴

### パフォーマンス最適化
- **オブジェクトプール**: 10個のエフェクトをプールして再利用
- **バーストモード**: 高い発生頻度（100.0f以上）で一度に全パーティクルを発生
- **自動ライフサイクル管理**: パーティクルの寿命を自動管理

### エフェクトカスタマイゼーション
- **色の線形補間**: 開始色から終了色への滑らかな変化
- **スケール変化**: 開始サイズから終了サイズへの変化
- **回転アニメーション**: ランダムな回転と回転速度
- **物理シミュレーション**: 重力加速度の適用

## リソース要件

### 必要ファイル
- `Resources/particle/effect.obj` - 3Dモデルファイル
- `Resources/particle/effect.mtl` - マテリアルファイル（オプション）

### システム要件
- DirectX 12対応GPU
- 充分なVRAM（パーティクル描画用）

## 拡張可能性

### 新しいエフェクトタイプの追加
1. `HitEffect3D::EffectType` 列挙型に新しいタイプを追加
2. `InitializeEffectSettings()` メソッドに設定を追加
3. `EffectManager3D` に対応するメソッドを追加

### カスタムパーティクルモデル
異なる3Dモデルを使用したエフェクトも作成可能：
```cpp
Particle3DManager::GetInstance()->CreateParticle3DGroup("customEffect", "custom.obj");
```

## トラブルシューティング

### よくある問題
1. **エフェクトが表示されない**
   - effect.objファイルが正しい場所にあるか確認
   - Particle3DManagerが初期化されているか確認

2. **パフォーマンスが低下する**
   - エフェクトの同時発生数を制限
   - パーティクル数を調整

3. **色が正しく表示されない**
   - マテリアル設定を確認
   - カラー値の範囲（0.0f～1.0f）を確認

## 今後の改善点

- テクスチャアニメーション対応
- より複雑な物理シミュレーション
- サウンドエフェクトとの連携
- GPU パーティクルシステムへの移行検討

---

**実装完了日**: 2025年5月22日
**作成者**: 3Dパーティクルシステム開発チーム
