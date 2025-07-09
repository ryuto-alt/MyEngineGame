#include "Easing.h"

float Easing::Lerp(float startPos, float endPos, float easedT) {
	return (1.0f - easedT) * startPos + easedT * endPos;
}

Vector3 Easing::Lerp(Vector3 startPos, Vector3 endPos, float easedT) {
	return {
		(1.0f - easedT) * startPos.x + easedT * endPos.x,
		(1.0f - easedT) * startPos.y + easedT * endPos.y,
		(1.0f - easedT) * startPos.z + easedT * endPos.z
	};
}

Quaternion Easing::Slerp(Quaternion q0, Quaternion q1, float t) {
    float dot = QuaternionMath::Dot(q0, q1);

    // クォータニオンが逆向きの時、最短経路を取るために反転
    if (dot < 0.0f) {
        q0 = -q0;
        dot = -dot;
    }

    // ドット積がほぼ1（角度が小さい）場合、線形補間に切り替える
    const float THRESHOLD = 0.9995f;
    if (dot > THRESHOLD) {
        // LERP (線形補間) を使用
        Quaternion result = (1.0f - t) * q0 + t * q1;
        return QuaternionMath::Normalize(result); // 正規化
    }

    // 通常の SLERP の計算
    float theta = std::acos(dot);       // なす角
    float sinTheta = std::sin(theta);   // sin(θ)

    float scale0 = std::sin((1.0f - t) * theta) / sinTheta;
    float scale1 = std::sin(t * theta) / sinTheta;

    return scale0 * q0 + scale1 * q1;
}

float Easing::EaseOut(float x) {
	return sinf((x * std::numbers::pi_v<float>) / 2.0f);
}

float Easing::EaseIn(float x) {
	return 1.0f - cosf((x * std::numbers::pi_v<float>) / 2.0f);
}

float Easing::EaseInOut(float x) {
	return -(cosf(std::numbers::pi_v<float> *x) - 1.0f) / 2.0f;
}
