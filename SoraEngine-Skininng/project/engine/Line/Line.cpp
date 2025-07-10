#include "Line.h"
#include "LineCommon.h"
#include <numbers>
#include "Vector3.h"




void Line::Draw(const Vector3& start, const Vector3& end, const Vector4& color)
{

	LineCommon::GetInstance()->DrawLine(start, end, color);


}

void Line::DrawAABB(const Vector3& min, const Vector3& max, const Vector4& color)
{ // 8頂点を定義
	Vector3 p[8] = {
		{ min.x, min.y, min.z }, // 0
		{ max.x, min.y, min.z }, // 1
		{ max.x, max.y, min.z }, // 2
		{ min.x, max.y, min.z }, // 3
		{ min.x, min.y, max.z }, // 4
		{ max.x, min.y, max.z }, // 5
		{ max.x, max.y, max.z }, // 6
		{ min.x, max.y, max.z }  // 7
	};

	// 前面
	LineCommon::GetInstance()->DrawLine(p[0], p[1], color);
	LineCommon::GetInstance()->DrawLine(p[1], p[2], color);
	LineCommon::GetInstance()->DrawLine(p[2], p[3], color);
	LineCommon::GetInstance()->DrawLine(p[3], p[0], color);

	// 背面
	LineCommon::GetInstance()->DrawLine(p[4], p[5], color);
	LineCommon::GetInstance()->DrawLine(p[5], p[6], color);
	LineCommon::GetInstance()->DrawLine(p[6], p[7], color);
	LineCommon::GetInstance()->DrawLine(p[7], p[4], color);

	// 側面（前後の接続）
	LineCommon::GetInstance()->DrawLine(p[0], p[4], color);
	LineCommon::GetInstance()->DrawLine(p[1], p[5], color);
	LineCommon::GetInstance()->DrawLine(p[2], p[6], color);
	LineCommon::GetInstance()->DrawLine(p[3], p[7], color);
}

void Line::DrawAABBVector3(Vector3 center, float radius, Vector4 color)
{
	float half = radius;
	Vector3 min = center - half;

	Vector3 max = center + half;

	DrawAABB(min, max, color); // 既存の min/max 版を再利用
}







void Line::DrawGrid(Vector3 center, float Gridhalfwidth, uint32_t Subdivision)
{
	assert(Subdivision > 0);

	const float step = (Gridhalfwidth * 2.0f) / Subdivision;
	const float start = -Gridhalfwidth;
	const float end = Gridhalfwidth;

	Vector4 lineColor = { 0.5f, 0.5f, 0.5f, 1.0f };  // 灰色

	// Z方向のライン（Xを固定してZを移動）
	for (uint32_t i = 0; i <= Subdivision; ++i) {
		float z = start + step * i;
		LineCommon::GetInstance()->DrawLine(
			{ center.x + start, center.y, center.z + z },
			{ center.x + end,   center.y, center.z + z },
			lineColor
		);
	}

	// X方向のライン（Zを固定してXを移動）
	for (uint32_t i = 0; i <= Subdivision; ++i) {
		float x = start + step * i;
		LineCommon::GetInstance()->DrawLine(
			{ center.x + x, center.y, center.z + start },
			{ center.x + x, center.y, center.z + end },
			lineColor
		);
	}

}

void Line::DrawSphere(const Vector3& center, float radius, const Vector4& color)
{
	const uint32_t kSbdivision = 16;
	const float kLonEvery = 2 * std::numbers::pi_v<float> / kSbdivision;
	const float KLatEvery = std::numbers::pi_v<float> / kSbdivision;

	for (uint32_t latIndex = 0; latIndex < kSbdivision; ++latIndex) {
		float lat = -std::numbers::pi_v<float> / 2.0f + KLatEvery * latIndex;
		for (uint32_t lonIndex = 0; lonIndex < kSbdivision; ++lonIndex) {

			float lon = lonIndex * kLonEvery;
			Vector3 a = {
				radius * std::cosf(lat) * std::cosf(lon) + center.x,
				radius * std::sinf(lat) + center.y,
				radius * std::cosf(lat) * std::sinf(lon) + center.z
			};

			Vector3 b = {
				radius * std::cosf(lat + KLatEvery) * std::cosf(lon) + center.x,
				radius * std::sinf(lat + KLatEvery) + center.y,
				radius * std::cosf(lat + KLatEvery) * std::sinf(lon) + center.z
			};

			Vector3 c = {
				radius * std::cosf(lat) * std::cosf(lon + kLonEvery) + center.x,
				radius * std::sinf(lat) + center.y,
				radius * std::cosf(lat) * std::sinf(lon + kLonEvery) + center.z
			};


			// 線を描画（緯度線・経度線）
			LineCommon::GetInstance()->DrawLine(a, b, color); // 縦方向
			LineCommon::GetInstance()->DrawLine(a, c, color); // 横方向



		}


	}


}

void Line::DrawSkeleton(const Skeleton& skeleton, const std::vector<Matrix4x4>& skeletonPose, const Matrix4x4& worldMatrix, const Vector4& color)
{
	for (const Joint& joint : skeleton.joints) {
		// ジョイントの位置（子）
		Vector3 jointPos = MyMath::Transform(MyMath::GetTranslate(skeletonPose[joint.index]), worldMatrix);

		//ジョイント球を描画（小さめの球）
		DrawSphere(jointPos, 0.005f, color);

		// 親がいるならラインを描画
		if (joint.parent.has_value()) {
			int parentIndex = joint.parent.value();
			Vector3 parentPos = MyMath::Transform(MyMath::GetTranslate(skeletonPose[parentIndex]), worldMatrix);

			LineCommon::GetInstance()->DrawLine(parentPos, jointPos, color);
		}
	}

	


}


