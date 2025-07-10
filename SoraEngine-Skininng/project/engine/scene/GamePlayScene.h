#pragma once

#include "Camera.h"	
#include "Model.h"
#include"Sprite.h"
#include "Object3D.h"
#include "Audio.h"
#include "BaseScene.h"

#include "SceneManager.h"
#include "ParticleEmitter.h"
#include "ParticleMnager.h"
#include "Line.h"



class GamePlayScene :public BaseScene
{
public:

	/// <summary>
	/// シーンの初期化
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// シーンの終了処理
	/// </summary>
	void Finalize()override;
	/// <summary>
	/// シーンの更新
	/// </summary>
	void Update()override;
	/// <summary>
	/// シーンの描画
	/// </summary>
	void Draw()override;

	void LoadModel();
	void Loadparticle();
	void LoadAudio();

private:
	std::unique_ptr<Camera> camera1;
	std::unique_ptr<Camera> camera2;
	std::unique_ptr<Object3D> object3D;
	std::unique_ptr<Object3D> terrain;

	//particle
	std::unique_ptr<ParticleEmitter> particleEmitter;
	std::unique_ptr<ParticleEmitter> particleEmitter2;
	bool light = true;
	bool directionLight = true;
	bool pointLight = true;
	bool spotLight = true;
	std::unique_ptr<Sprite> sprite;
	SoundData sampleSoundData;//サウンドデータ
	
	bool number = 0;

	std::unique_ptr<Line> line;

	Vector3 startline;
	Vector3 endline;




};

