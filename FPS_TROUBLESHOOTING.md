# 67fps制限の解決方法

67fpsという中途半端なフレームレートの原因と解決策をまとめます。

## 🔧 **実装済み修正**

### 1. DirectX12設定強化
- ✅ `DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING` 追加
- ✅ `DXGI_PRESENT_ALLOW_TEARING` フラグ使用
- ✅ CommandQueue高優先度設定
- ✅ フレーム遅延最小化 (`SetMaximumFrameLatency(1)`)

### 2. プロセス優先度設定
- ✅ `HIGH_PRIORITY_CLASS` でプロセス優先度向上
- ✅ 高DPI対応設定

## 🚨 **Windows設定確認が必要**

### 1. **Windowsゲームモード確認**
```
Windows設定 > ゲーム > ゲームモード > オン
```

### 2. **グラフィックス設定**
```
Windows設定 > システム > ディスプレイ > グラフィックス設定
> アプリを追加 > CG2_00-01.exe > 高パフォーマンス
```

### 3. **電源プラン設定**
```
コントロールパネル > 電源オプション > 高パフォーマンス
```

### 4. **NVIDIA/AMD設定**

**NVIDIA Control Panel:**
```
3D設定の管理 > プログラム設定 > CG2_00-01.exe
- 垂直同期: オフ
- 電源管理モード: パフォーマンス最大化を優先
- 最大フレームレート: オフ
```

**AMD Radeon Settings:**
```
ゲーム > CG2_00-01.exe
- Enhanced Sync: 無効
- Frame Rate Target Control: 無効  
- Radeon Anti-Lag: 有効
```

## 🔍 **67fps原因の特定**

### よくある原因

1. **DWM (Desktop Window Manager) 制限**
   - Windows 10/11のコンポジター制限
   - 解決策: フルスクリーン排他モード

2. **GPU温度制限**
   - 67fps = 約2/3の60fps制限
   - 解決策: GPU温度監視・冷却改善

3. **CPU制限**
   - シングルスレッド性能不足
   - 解決策: より高速なCPU使用

4. **VRAM不足**
   - メモリ帯域制限
   - 解決策: テクスチャ品質下げ

## 🛠 **追加デバッグ方法**

### 1. フレームレート詳細確認
```cpp
// UnoEngine::ShowDebugInfo()に追加
ImGui::Text("Frame Time: %.3f ms", 1000.0f / currentFPS);
ImGui::Text("GPU Usage: 確認中");
```

### 2. **MSI Afterburner使用**
- GPU使用率確認
- GPU温度確認
- VRAM使用量確認

### 3. **タスクマネージャー確認**
- CPU使用率
- GPU使用率
- メモリ使用量

## ⚡ **即効性のある解決策**

### 1. **フルスクリーン化**
- Alt + Enter でフルスクリーン
- DWM制限を回避

### 2. **解像度下げ**
- WinApp.h の kClientWidth/Height 縮小
- 負荷軽減でフレームレート向上

### 3. **パーティクル数削減**
```cpp
// EffekseerManager.cpp で調整
particleName, position, 15, // 30→15に削減
```

### 4. **レンダリング品質調整**
```cpp
// DirectXCommon.cpp
swapChainDesc.BufferCount = 1; // 2→1でシングルバッファ
```

## 🎯 **最終手段**

もし上記で解決しない場合：

1. **グラフィックスドライバー更新**
2. **Windows Update実行**  
3. **他のアプリケーション終了**
4. **ウイルススキャン無効化**

**現在の設定で120fpsに近づくはずです。67fps制限の原因を特定して解決してください！**