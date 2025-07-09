#include "Vector3.h"
#include "Matrix4x4.h"
#include "Quaternion.h"


///////////////////////////////////////////////////////////////////////////////
//	4x4行列
///////////////////////////////////////////////////////////////////////////////

namespace MatrixMath {

	//行列の加法
	Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);

	//行列の減法
	Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2);

	//行列の積
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

	//逆行列
	Matrix4x4 Inverse(const Matrix4x4& m);

	//転置行列
	Matrix4x4 Transpose(const Matrix4x4& m);

	//単位行列の作成
	Matrix4x4 MakeIdentity4x4();

	//平行移動行列
	Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

	//拡大縮小行列
	Matrix4x4 MakeScaleMatrix(const Vector3& scale);

	//X軸回転行列
	Matrix4x4 MakeRotateXMatrix(float radian);

	//Y軸回転行列
	Matrix4x4 MakeRotateYMatrix(float radian);

	//Z軸回転行列
	Matrix4x4 MakeRotateZMatrix(float radian);

	Matrix4x4 MakeRotateMatrix(const Vector3& rotate);

	//任意軸回転行列
	Matrix4x4 MakeRotateAxisAngle(const Vector3& axis, float angle);

	//Quaternionを使った回転行列
	Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion);


	//3次元アフィン変換行列
	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Quaternion& rotate, const Vector3& translate);

	//透視投影行列
	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

	//正射影行列
	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

	//ビューポート変換行列
	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);

	//Matrix4x4からVector3に座標変換
	Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

	//Matrix4x4からVector3に回転とスケーリングのみ反映
	Vector3 TransformNormal(const Vector3& v, const Matrix4x4& matrix);

	//InverseTranspose
	Matrix4x4 InverseTranspose(const Matrix4x4& m);

	//ある方向からある方向へ向ける回転行列
	Matrix4x4 DirectionToDirection(const Vector3& from, const Vector3& to);
};
