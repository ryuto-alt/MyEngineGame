#pragma once

#include "MyMath.h"
#include "RenderingData.h"



class LineCommon;
class Line
{
public:


	//ラインを描画する
	void Draw(const Vector3& start, const Vector3& end, const Vector4& color);
	//AABBを描画する
	void DrawAABB(const Vector3& min, const Vector3& max, const Vector4& color);
	//centerを中心に半径radiusのAABBを描画する
	void DrawAABBVector3( Vector3 center, float radius, Vector4 color);
	//centerを中心にグリッドを描画する
	void DrawGrid(Vector3 center,float Gridhalfwidth = 2.0, uint32_t Subdivision = 50);
	//centerを中心に半径radiusの球を描画する
	void DrawSphere(const Vector3& center, float radius, const Vector4& color);
	//skeletonを描画する
	void DrawSkeleton(const Skeleton& skeleton, const std::vector<Matrix4x4>& skeletonPose, const Matrix4x4& worldMatrix, const Vector4& color={1.0f,1.0f,1.0f,1.0f});


};

