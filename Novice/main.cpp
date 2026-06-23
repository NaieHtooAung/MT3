#define NOMINMAX
#include <Novice.h>
#include <cmath>
#include <imgui.h>

const char kWindowTitle[] = "GC2A_04_ネイ_トウーアウン";
const float kPi = 3.14159265358979323846f;

typedef struct Vector3 {
	float x;
	float y;
	float z;
} Vector3;

typedef struct Matrix4x4 {
	float m[4][4];
} Matrix4x4;

Vector3 Add(const Vector3& v1, const Vector3& v2) { return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }
Vector3 Subtract(const Vector3& v1, const Vector3& v2) { return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }
float Dot(const Vector3& v1, const Vector3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
Vector3 Scale(const Vector3& v, float s) { return {v.x * s, v.y * s, v.z * s}; }

// Linearly interpolates between two vectors by t in [0, 1]
Vector3 Lerp(const Vector3& a, const Vector3& b, float t) { return {a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t}; }

// Quadratic Bezier using two nested Lerps: Lerp(Lerp(a,b,t), Lerp(b,c,t), t)
Vector3 QuadraticBezier(const Vector3& a, const Vector3& b, const Vector3& c, float t) {
	Vector3 ab = Lerp(a, b, t);
	Vector3 bc = Lerp(b, c, t);
	return Lerp(ab, bc, t);
}

Matrix4x4 Multiply(Matrix4x4 m1, Matrix4x4 m2) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++)
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
	return result;
}

Matrix4x4 MakeRotateXMatrix(float angle) {
	Matrix4x4 result = {};
	result.m[0][0] = 1.0f;
	result.m[1][1] = std::cos(angle);
	result.m[1][2] = std::sin(angle);
	result.m[2][1] = -std::sin(angle);
	result.m[2][2] = std::cos(angle);
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MakeRotateYMatrix(float angle) {
	Matrix4x4 result = {};
	result.m[0][0] = std::cos(angle);
	result.m[0][2] = -std::sin(angle);
	result.m[1][1] = 1.0f;
	result.m[2][0] = std::sin(angle);
	result.m[2][2] = std::cos(angle);
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MakeTranslateMatrix(float x, float y, float z) {
	Matrix4x4 result = {};
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][0] = x;
	result.m[3][1] = y;
	result.m[3][2] = z;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MakeInverseMatrix(const Matrix4x4& m) {
	Matrix4x4 result = {};
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			result.m[i][j] = m.m[j][i];
	result.m[3][0] = -(m.m[3][0] * result.m[0][0] + m.m[3][1] * result.m[1][0] + m.m[3][2] * result.m[2][0]);
	result.m[3][1] = -(m.m[3][0] * result.m[0][1] + m.m[3][1] * result.m[1][1] + m.m[3][2] * result.m[2][1]);
	result.m[3][2] = -(m.m[3][0] * result.m[0][2] + m.m[3][1] * result.m[1][2] + m.m[3][2] * result.m[2][2]);
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 makeViewportMatrix(float x, float y, float width, float height, float minZ, float maxZ) {
	Matrix4x4 result = {};
	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[2][2] = maxZ - minZ;
	result.m[3][0] = x + width / 2.0f;
	result.m[3][1] = y + height / 2.0f;
	result.m[3][2] = minZ;
	result.m[3][3] = 1.0f;
	return result;
}

Vector3 Transform(const Vector3& vertex, const Matrix4x4& matrix) {
	Vector3 result{};
	result.x = vertex.x * matrix.m[0][0] + vertex.y * matrix.m[1][0] + vertex.z * matrix.m[2][0] + matrix.m[3][0];
	result.y = vertex.x * matrix.m[0][1] + vertex.y * matrix.m[1][1] + vertex.z * matrix.m[2][1] + matrix.m[3][1];
	result.z = vertex.x * matrix.m[0][2] + vertex.y * matrix.m[1][2] + vertex.z * matrix.m[2][2] + matrix.m[3][2];
	float w = vertex.x * matrix.m[0][3] + vertex.y * matrix.m[1][3] + vertex.z * matrix.m[2][3] + matrix.m[3][3];
	if (w != 0.0f) {
		result.x /= w;
		result.y /= w;
		result.z /= w;
	}
	return result;
}

void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f;
	const uint32_t kSubdivision = 10;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);
	for (uint32_t xIndex = 0; xIndex <= kSubdivision; xIndex++) {
		float x = -kGridHalfWidth + float(xIndex) * kGridEvery;
		Vector3 s = Transform(Transform({x, 0.0f, -kGridHalfWidth}, viewProjectionMatrix), viewportMatrix);
		Vector3 e = Transform(Transform({x, 0.0f, kGridHalfWidth}, viewProjectionMatrix), viewportMatrix);
		uint32_t color = (xIndex == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF;
		Novice::DrawLine(int(s.x), int(s.y), int(e.x), int(e.y), color);
	}
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; zIndex++) {
		float z = -kGridHalfWidth + float(zIndex) * kGridEvery;
		Vector3 s = Transform(Transform({-kGridHalfWidth, 0.0f, z}, viewProjectionMatrix), viewportMatrix);
		Vector3 e = Transform(Transform({kGridHalfWidth, 0.0f, z}, viewProjectionMatrix), viewportMatrix);
		uint32_t color = (zIndex == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF;
		Novice::DrawLine(int(s.x), int(s.y), int(e.x), int(e.y), color);
	}
}

// Draws a filled dot at a world-space point using a small filled box
void DrawPoint(const Vector3& pos, const Matrix4x4& vp, const Matrix4x4& viewport, uint32_t color) {
	Vector3 s = Transform(Transform(pos, vp), viewport);
	int x = int(s.x), y = int(s.y);
	const int r = 4;
	Novice::DrawBox(x - r, y - r, r * 2, r * 2, 0.0f, color, kFillModeSolid);
}

// Draws the full quadratic Bezier curve by sampling kDivision segments
void DrawBezierCurve(const Vector3& a, const Vector3& b, const Vector3& c, const Matrix4x4& vp, const Matrix4x4& viewport, uint32_t color) {
	const int kDivision = 64;
	for (int i = 0; i < kDivision; i++) {
		float t0 = float(i) / float(kDivision);
		float t1 = float(i + 1) / float(kDivision);
		Vector3 p0 = QuadraticBezier(a, b, c, t0);
		Vector3 p1 = QuadraticBezier(a, b, c, t1);
		Vector3 s0 = Transform(Transform(p0, vp), viewport);
		Vector3 s1 = Transform(Transform(p1, vp), viewport);
		Novice::DrawLine(int(s0.x), int(s0.y), int(s1.x), int(s1.y), color);
	}
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 cameraTranslate = {0.0f, 1.9f, -6.49f};
	Vector3 cameraRotate = {0.26f, 0.0f, 0.0f};

	// Three control points for the quadratic Bezier curve
	Vector3 controlPoints[3] = {
	    {-0.8f, 0.58f, 1.0f }, // p0: start
	    {1.76f,  1.0f, -0.3f }, // p1: control (pulls the curve)
	    {0.94f,  -0.7f, 2.3f}, // p2: end
	};

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		// ImGui controls
		ImGui::Begin("Window");
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);
		ImGui::Separator();
		ImGui::DragFloat3("controlPoints[0]", &controlPoints[0].x, 0.01f);
		ImGui::DragFloat3("controlPoints[1]", &controlPoints[1].x, 0.01f);
		ImGui::DragFloat3("controlPoints[2]", &controlPoints[2].x, 0.01f);
		ImGui::End();

		// Build view matrix
		Matrix4x4 cameraRotateMatrix = Multiply(MakeRotateYMatrix(cameraRotate.y), MakeRotateXMatrix(cameraRotate.x));
		Matrix4x4 cameraTranslateMatrix = MakeTranslateMatrix(cameraTranslate.x, cameraTranslate.y, cameraTranslate.z);
		Matrix4x4 cameraMatrix = Multiply(cameraTranslateMatrix, cameraRotateMatrix);
		Matrix4x4 viewMatrix = MakeInverseMatrix(cameraMatrix);

		// Build perspective projection matrix
		Matrix4x4 projectionMatrix = {};
		float fovY = 0.45f;
		float aspect = 1280.0f / 720.0f;
		float nearZ = 0.1f;
		float farZ = 100.0f;
		projectionMatrix.m[0][0] = 1.0f / (aspect * std::tan(fovY / 2.0f));
		projectionMatrix.m[1][1] = 1.0f / std::tan(fovY / 2.0f);
		projectionMatrix.m[2][2] = farZ / (farZ - nearZ);
		projectionMatrix.m[2][3] = 1.0f;
		projectionMatrix.m[3][2] = -nearZ * farZ / (farZ - nearZ);

		Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewportMatrix = makeViewportMatrix(0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawGrid(viewProjectionMatrix, viewportMatrix);

		// Draw the full Bezier curve path in black
		DrawBezierCurve(controlPoints[0], controlPoints[1], controlPoints[2], viewProjectionMatrix, viewportMatrix, BLACK);

		// Draw gray lines for control polygon: p0→p1 and p1→p2
		{
			Vector3 s0 = Transform(Transform(controlPoints[0], viewProjectionMatrix), viewportMatrix);
			Vector3 s1 = Transform(Transform(controlPoints[1], viewProjectionMatrix), viewportMatrix);
			Vector3 s2 = Transform(Transform(controlPoints[2], viewProjectionMatrix), viewportMatrix);
			Novice::DrawLine(int(s0.x), int(s0.y), int(s1.x), int(s1.y), 0x888888FF);
			Novice::DrawLine(int(s1.x), int(s1.y), int(s2.x), int(s2.y), 0x888888FF);
		}

		// Draw the three control points as filled dots
		DrawPoint(controlPoints[0], viewProjectionMatrix, viewportMatrix, BLACK);
		DrawPoint(controlPoints[1], viewProjectionMatrix, viewportMatrix, BLACK);
		DrawPoint(controlPoints[2], viewProjectionMatrix, viewportMatrix, BLACK);

		///
		/// ↑描画処理ここまで
		///

		Novice::EndFrame();
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0)
			break;
	}

	Novice::Finalize();
	return 0;
}