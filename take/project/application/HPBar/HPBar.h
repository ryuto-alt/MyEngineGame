#pragma once
#include "Sprite.h"
#include <memory>

class HPBar {
public:
	HPBar() = default;
	~HPBar();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(SpriteCommon* spriteCommon, const std::string& backgroundFilePath, const std::string& foregroundFilePath, const std::string&bugeFilePath);

	/// <summary>
	/// HPの更新
	/// </summary>
	void Update(float currentHP, float maxHP);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui
	/// </summary>
	//void ImGuiDebug(int id);
	//void ImGuibugeDebug(int id);

	/// <summary>
	/// 位置を設定
	/// </summary>
	void SetPosition(const Vector2& position);
	void SetbugePosition(const Vector2& position);

	/// <summary>
/// サイズを設定
/// </summary>
	void SetSize(const Vector2& size);
	void SetbugeSize(const Vector2& size);


	/// <summary>
	/// 位置を取得
	/// </summary>
	Vector2 GetTranslate() const;

private:
	std::unique_ptr<Sprite> backgroundSprite_; // 背景スプライト
	std::unique_ptr<Sprite> foregroundSprite_; // フォアグラウンドスプライト
	std::unique_ptr<Sprite> bugeSprite_;
	Vector2 position_ = { 0.0f, 0.0f };          // HPバーの位置
};
