#pragma once
#include "DirectXCommon.h"
#include "GraphicsPipeline.h"
class SpriteCommon
{
public:
	
	static SpriteCommon* GetInstance();

	

	/// <summary>
		/// 初期化
		/// </summary>
	void Initialize(DirectXCommon* dxCommon);

	void Finalize();

	//共通描画設定
	void CommonDraw();
	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }


private:

	static SpriteCommon* instance_;
	DirectXCommon* dxCommon_;//dxcommnをポインタ参照
	
	//グラフィックスパイプライン
	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;


};

