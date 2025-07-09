static const float kPrewittHorizontalKernel[3][3] = {
	{ -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
	{ -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
	{ -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
};

static const float kPrewittVerticalKernel[3][3] = {
	{ -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
	{ 0.0f, 0.0f, 0.0f },
	{ 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f },
};

static const float2 kIndex3x3[3][3] = {
	{ { -1.0f, 1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
	{ { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
	{ { -1.0f, -1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } },
};