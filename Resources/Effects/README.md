# Effekseerエフェクトファイル

このディレクトリにEffekseerで作成した`.efkefc`エフェクトファイルを配置してください。

## 使用方法

1. **Effekseer Editor**でエフェクトを作成・編集
2. エフェクトを`.efkefc`形式で保存
3. このディレクトリに配置
4. ゲーム内で以下のAPIを使用してエフェクトを制御

```cpp
// エフェクトの読み込み
engine->LoadEffekseerEffect("effect_name", "Resources/Effects/effect_file.efkefc");

// エフェクトの再生
int handle = engine->PlayEffekseerEffect("effect_name", position);

// エフェクトの制御
engine->SetEffekseerEffectPosition(handle, newPosition);
engine->SetEffekseerEffectRotation(handle, rotation);
engine->SetEffekseerEffectScale(handle, scale);

// エフェクトの停止
engine->StopEffekseerEffect(handle);
engine->StopAllEffekseerEffects();
```

## サンプルエフェクト

GamePlaySceneでは以下のエフェクトが使用されています：

- `Laser01.efkefc` - レーザーエフェクト（Lキーで再生）
- `Explosion01.efkefc` - 爆発エフェクト（Eキーで再生）

これらのファイルをEffekseerの公式サンプルからコピーするか、
Effekseer Editorで新しく作成してください。

## 注意事項

- EffekseerライブラリはプレースホルダーAPIとして実装されています
- 実際に動作させるには、Effekseerライブラリのビルドとリンクが必要です
- 現在の実装では、ログ出力のみが行われます