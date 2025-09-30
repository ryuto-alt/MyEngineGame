# Dark Deception風迷路パーツ仕様書

## パーツサイズ
- 基本グリッドサイズ: 2.0 x 2.0 メートル
- 壁の高さ: 3.0 メートル
- 壁の厚さ: 0.2 メートル

## 必須パーツリスト

### 1. 床パーツ (Floor)
- **floor_basic.glb** - 基本的な床タイル
- **floor_damaged.glb** - ダメージのある床（ホラー演出用）
- **floor_grate.glb** - 格子状の床

### 2. 壁パーツ (Wall)
- **wall_straight.glb** - 直線壁
- **wall_corner_inner.glb** - 内側コーナー（L字型）
- **wall_corner_outer.glb** - 外側コーナー
- **wall_t_junction.glb** - T字路
- **wall_cross.glb** - 十字路
- **wall_dead_end.glb** - 行き止まり

### 3. 特殊パーツ (Special)
- **door_normal.glb** - 通常のドア
- **door_locked.glb** - 鍵付きドア
- **pillar.glb** - 柱（大部屋用）
- **portal.glb** - ポータル/テレポート地点

### 4. 装飾パーツ (Decoration)
- **pipe_ceiling.glb** - 天井のパイプ
- **light_flickering.glb** - ちらつくライト
- **fog_emitter.glb** - 霧エミッター
- **blood_stain.glb** - 血痕（ホラー演出）

## パーツ命名規則
```
[カテゴリ]_[タイプ]_[バリエーション].glb
例: wall_corner_damaged.glb
```

## マテリアル設定
- **BaseColor**: 暗めの色調（RGB: 0.2-0.4）
- **Metallic**: 0.0-0.2（金属感は控えめ）
- **Roughness**: 0.7-0.9（粗い表面）
- **Normal Map**: 必須（壁の質感表現）
- **Emissive**: ライトパーツのみ使用

## エクスポート設定
- 形式: glTF 2.0 (.glb)
- スケール: 1 Blender Unit = 1 メートル
- 原点: パーツの中心底面
- 向き: +Y が上方向、+Z が前方向

## 最適化要件
- 頂点数: 500以下（単純パーツ）、2000以下（複雑パーツ）
- テクスチャ: 最大1024x1024
- 同一パーツは必ずインスタンシング可能にする