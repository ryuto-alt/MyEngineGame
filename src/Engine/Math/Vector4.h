#pragma once
struct Vector4
{
	float x;
	float y;
	float z;
	float w;
};

inline Vector4 operator-(const Vector4& q1) {
	Vector4 result;

	result.w = -q1.w;
	result.x = -q1.x;
	result.y = -q1.y;
	result.z = -q1.z;
	return result;
}
inline Vector4 operator+(Vector4 q1, Vector4 q2) {
	q1.w += q2.w;
	q1.x += q2.x;
	q1.y += q2.y;
	q1.z += q2.z;
	return q1;
}

inline Vector4 operator*(float n, Vector4 q1) {
	Vector4 result;

	result.w = q1.w * n;
	result.x = q1.x * n;
	result.y = q1.y * n;
	result.z = q1.z * n;
	return result;
}