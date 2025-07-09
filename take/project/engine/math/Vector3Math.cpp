#include "Vector3Math.h"
#include <cmath>

/////////////////////////////////////////////////////////////////
///		Vector3
/////////////////////////////////////////////////////////////////

//加算
Vector3 Vector3Math::Add(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
}

//減算
Vector3 Vector3Math::Subtract(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
}

//乗算
Vector3 Vector3Math::Multiply(float scalar, const Vector3& v) {
	Vector3 result;
	result.x = scalar * v.x;
	result.y = scalar * v.y;
	result.z = scalar * v.z;
	return result;
}

Vector3 Vector3Math::Multiply(const Vector3& v1, const Vector3& v2) {
	Vector3 result;
	result.x = v1.x * v2.x;
	result.y = v1.y * v2.y;
	result.z = v1.z * v2.z;
	return result;
}

Vector3 Vector3Math::Divide(float scalar, const Vector3& v) {
	Vector3 result;
	result.x = scalar / v.x;
	result.y = scalar / v.y;
	result.z = scalar / v.z;
	return result;
}

//内積
float Vector3Math::Dot(const Vector3& v1, const Vector3& v2) {
	float result;
	result = (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
	return result;
}

//クロス積
Vector3 Vector3Math::Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result = {
	(v1.y * v2.z) - (v1.z * v2.y),
	(v1.z * v2.x) - (v1.x * v2.z),
	(v1.x * v2.y) - (v1.y * v2.x)
	};
	return result;
}


//長さ(ノルム)
float Vector3Math::Length(const Vector3& v) {
	float result;
	result = sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	return result;
}

//正規化
Vector3 Vector3Math::Normalize(const Vector3& v) {
	Vector3 result;
	float length = Length(v);
	result.x = v.x / length;
	result.y = v.y / length;
	result.z = v.z / length;
	return result;
}