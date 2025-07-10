#include "Camera.h"
#include "MyMath.h"

Camera::Camera()
{
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{ 0.0f,0.0f,-5.0f} };
	
	fovY = 0.45f;
	aspectRatio = float(WinApp::kClientWindth) / float(WinApp::kClientHeight);
	nearCilp = 0.1f;
	farClip = 100.0f;
	projectionMatrix = MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearCilp, farClip);
	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix = worldMatrix.Inverse();
	viewProjectionMatrix = viewMatrix * projectionMatrix;

}

void Camera::Update()
{
	projectionMatrix = MyMath::MakePerspectiveFovMatrix(fovY, aspectRatio, nearCilp, farClip);
	worldMatrix = MyMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	viewMatrix = worldMatrix.Inverse();
	viewProjectionMatrix = viewMatrix * projectionMatrix;

}
