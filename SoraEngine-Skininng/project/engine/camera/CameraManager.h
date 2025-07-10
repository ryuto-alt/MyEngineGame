#pragma once
#include <Camera.h>
#include <unordered_map>
#include <string>
#include <memory>
class CameraManager
{
	static CameraManager* instance;
	CameraManager() = default;
	~CameraManager() = default;
	CameraManager(CameraManager&) = default;
	CameraManager& operator=(CameraManager&) = delete;

public:
	//シングルトンインスタンスの取得
	static CameraManager* GetInstance();
	//終了
	void Finalize();

	//初期化
	void Initialize();



	//カメラの追加
	void AddCamera(const std::string& name, const Camera* camera);

	//カメラの削除
	void RemoveCamera(const std::string& name);

	//カメラの取得
	Camera* GetCamera(const std::string& name);

	// アクティブカメラの取得
	Camera* GetActiveCamera();


	// アクティブカメラの設定
	void SetActiveCamera(const std::string& name);



private:
	//カメラデータ
	std::unordered_map<std::string, Camera> cameras;

	// アクティブカメラ名
	std::string activeCameraName;

	//デフォルトカメラ
	
	//デフォルトカメラ
	Camera* defaultCamera=nullptr;


};

