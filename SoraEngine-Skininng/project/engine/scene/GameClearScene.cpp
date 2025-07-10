#include "GameClearScene.h"
#include "Object3DCommon.h"
#include "SpriteCommon.h"
#include "ImGuiManager.h"

#include <imgui.h>
#include "Input.h"
#include "SceneManager.h"
#include "CameraManager.h"

void GameClearScene::Initialize()
{
	CameraManager::GetInstance()->Initialize();
}

void GameClearScene::Finalize()
{
}

void GameClearScene::Update()
{
	CameraManager::GetInstance()->GetActiveCamera()->Update();






#ifdef _DEBUG

	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("gameClearScene");
		if (ImGui::Button("TitleScene"))
		{
			SceneManager::GetInstance()->ChangeScene("TITELE");
		}
		



	}

#endif // _DEBUG


}

void GameClearScene::Draw()
{
#pragma region 3Dオブジェクト描画

	//3dオブジェクトの描画準備。3Dオブジェクトの描画に共通のグラフィックスコマンドを積む
	Object3DCommon::GetInstance()->CommonDraw();


#pragma endregion


#pragma region スプライト描画
	//Spriteの描画準備。spriteの描画に共通のグラフィックスコマンドを積む
	SpriteCommon::GetInstance()->CommonDraw();

#pragma endregion

}
