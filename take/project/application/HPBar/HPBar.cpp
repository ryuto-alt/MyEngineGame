#include "HPBar.h"
#undef max
#undef min
#include <algorithm>

HPBar::~HPBar() {
	// unique_ptr により自動解放されるため明示的に削除は不要
}

void HPBar::Initialize(SpriteCommon* spriteCommon, const std::string& backgroundFilePath, const std::string& foregroundFilePath, const std::string& bugeFilePath) {
	// 背景スプライトの初期化
	backgroundSprite_ = std::make_unique<Sprite>();
	backgroundSprite_->Initialize(spriteCommon, backgroundFilePath);

	// フォアグラウンドスプライトの初期化
	foregroundSprite_ = std::make_unique<Sprite>();
	foregroundSprite_->Initialize(spriteCommon, foregroundFilePath);

	bugeSprite_ = std::make_unique<Sprite>();
	bugeSprite_->Initialize(spriteCommon, bugeFilePath);
}

void HPBar::Update(float currentHP, float maxHP) {
	// HPの割合を計算

	currentHP = std::max(0.0f, std::min(currentHP, maxHP));
	float hpRatio = currentHP / maxHP;

	// フォアグラウンドスプライトの幅をHP割合に応じて更新
	Vector2 size = foregroundSprite_->GetSize();
	size.x = backgroundSprite_->GetSize().x * hpRatio; // 横幅を調整
	foregroundSprite_->SetSize(size);

	// フォアグラウンドスプライトの位置を背景に揃える
	foregroundSprite_->SetPosition(backgroundSprite_->GetTranslate());
	backgroundSprite_->Update();
	foregroundSprite_->Update();
	bugeSprite_->Update();
}

void HPBar::Draw() {
	// 背景スプライトを描画
	if (backgroundSprite_) {
		backgroundSprite_->Draw();
	}

	// フォアグラウンドスプライトを描画
	if (foregroundSprite_) {
		foregroundSprite_->Draw();
	}
	if (bugeSprite_) {

		//bugeSprite_->Draw();
	}
}

#ifdef _DEBUG

#endif // DEBUG

void HPBar::SetPosition(const Vector2& position) {
	position_ = position;

	// 背景スプライトの位置を設定
	backgroundSprite_->SetPosition(position);

	// フォアグラウンドスプライトの位置も背景に揃える
	foregroundSprite_->SetPosition(position);
}

void HPBar::SetbugePosition(const Vector2& position) { bugeSprite_->SetPosition(position); }

void HPBar::SetSize(const Vector2& size) {
	if (backgroundSprite_) {
		backgroundSprite_->SetSize(size);
	}
	if (foregroundSprite_) {
		Vector2 foregroundSize = size;
		foregroundSize.x *= foregroundSprite_->GetSize().x / backgroundSprite_->GetSize().x; // 比率を保持
		foregroundSprite_->SetSize(foregroundSize);
	}
}

void HPBar::SetbugeSize(const Vector2& size) { bugeSprite_->SetSize(size); }

Vector2 HPBar::GetTranslate() const { return position_; }
