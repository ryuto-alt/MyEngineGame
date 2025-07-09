#pragma once
#include "GameCharacter.h"

class SampleCharacter : public GameCharacter {
public:

	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	void Update() override;
	void UpdateImGui();
	void Draw() override;
	void DrawCollider() override;
	void OnCollisionAction(GameCharacter* other) override;

	void SkinningDisPatch();

private:


};

