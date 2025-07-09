#include "MyGame.h"

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	//leakCheckerの生成
	D3DResourceLeakChecker leakCheck;
	//ゲームクラスの生成
	std::unique_ptr<TakeCFrameWork> game = std::make_unique<MyGame>();
	//ゲームの実行
	game->Run(L"TakeCEngine");
	
	return 0; //終了
}