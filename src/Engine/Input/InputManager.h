#pragma once
#include <Windows.h>
#include <vector>
#include <array>
#include <string>
#include <unordered_map>

namespace Engine {
namespace Input {

// 入力マネージャークラス
class InputManager {
public:
    // シングルトンインスタンス取得
    static InputManager* GetInstance();

    // 初期化
    void Initialize(HWND hwnd);

    // 更新
    void Update();

    // キー入力状態取得
    bool IsKeyDown(int keyCode) const;       // 押されているか
    bool IsKeyPressed(int keyCode) const;    // 押された瞬間か
    bool IsKeyReleased(int keyCode) const;   // 離された瞬間か

    // マウス入力状態取得
    bool IsMouseButtonDown(int button) const;     // 押されているか
    bool IsMouseButtonPressed(int button) const;  // 押された瞬間か
    bool IsMouseButtonReleased(int button) const; // 離された瞬間か
    
    // マウス座標取得
    int GetMouseX() const { return mouseX_; }
    int GetMouseY() const { return mouseY_; }
    
    // マウスの移動量取得
    int GetMouseMoveX() const { return mouseMoveX_; }
    int GetMouseMoveY() const { return mouseMoveY_; }

private:
    // コンストラクタ・デストラクタをprivateに
    InputManager() = default;
    ~InputManager() = default;
    
    // コピー禁止
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    // ウィンドウハンドル
    HWND hwnd_ = nullptr;
    
    // キーの状態管理
    static constexpr size_t kKeyCount = 256;
    std::array<bool, kKeyCount> currentKeys_ = {};
    std::array<bool, kKeyCount> prevKeys_ = {};
    
    // マウスの状態管理
    static constexpr size_t kMouseButtonCount = 3;
    std::array<bool, kMouseButtonCount> currentMouseButtons_ = {};
    std::array<bool, kMouseButtonCount> prevMouseButtons_ = {};
    
    // マウス座標
    int mouseX_ = 0;
    int mouseY_ = 0;
    int prevMouseX_ = 0;
    int prevMouseY_ = 0;
    int mouseMoveX_ = 0;
    int mouseMoveY_ = 0;
};

} // namespace Input
} // namespace Engine