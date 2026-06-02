#include "KamataEngine.h"
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

struct Segment {
	Vector3 origin;
	Vector3 diff;
};

struct Plane {
	Vector3 normal;
	float distance;
};

typedef struct Matrix4x4 {
	float m[4][4];
} Matrix4x4;

Vector3 Add(const Vector3& v1, const Vector3& v2) { return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }
Vector3 Subtract(const Vector3& v1, const Vector3& v2) { return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }
float Dot(const Vector3& v1, const Vector3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
float LengthSq(const Vector3& v) { return Dot(v, v); }
Vector3 Scale(const Vector3& v, float s) { return {v.x * s, v.y * s, v.z * s}; }
float Length(const Vector3& v) { return std::sqrt(Dot(v, v)); }

bool IsCollision(const Segment& segment, const Plane& plane) {
	float denom = Dot(segment.diff, plane.normal);
	if (std::abs(denom) < 1e-6f)
		return false;
	float t = (plane.distance - Dot(segment.origin, plane.normal)) / denom;
	return t >= 0.0f && t <= 1.0f;
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
		Vector3 worldStart = {x, 0.0f, -kGridHalfWidth};
		Vector3 worldEnd = {x, 0.0f, kGridHalfWidth};
		Vector3 screenStart = Transform(Transform(worldStart, viewProjectionMatrix), viewportMatrix);
		Vector3 screenEnd = Transform(Transform(worldEnd, viewProjectionMatrix), viewportMatrix);
		uint32_t color = (xIndex == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF;
		Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
	}
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; zIndex++) {
		float z = -kGridHalfWidth + float(zIndex) * kGridEvery;
		Vector3 worldStart = {-kGridHalfWidth, 0.0f, z};
		Vector3 worldEnd = {kGridHalfWidth, 0.0f, z};
		Vector3 screenStart = Transform(Transform(worldStart, viewProjectionMatrix), viewportMatrix);
		Vector3 screenEnd = Transform(Transform(worldEnd, viewProjectionMatrix), viewportMatrix);
		uint32_t color = (zIndex == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF;
		Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), color);
	}
}

Vector3 Perpendicular(const Vector3& v) {
	if (v.x != 0.0f || v.y != 0.0f)
		return {-v.y, v.x, 0.0f};
	return {0.0f, -v.z, v.y};
}

void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 center = Scale(plane.normal, plane.distance);
	Vector3 perp = Perpendicular(plane.normal);
	float perpLen = Length(perp);
	Vector3 perpN = Scale(perp, 1.0f / perpLen);
	Vector3 cross = {plane.normal.y * perpN.z - plane.normal.z * perpN.y, plane.normal.z * perpN.x - plane.normal.x * perpN.z, plane.normal.x * perpN.y - plane.normal.y * perpN.x};
	const float kPlaneHalfSize = 2.0f;
	Vector3 points[4] = {
	    Add(Add(center, Scale(perpN, kPlaneHalfSize)), Scale(cross, kPlaneHalfSize)),
	    Add(Add(center, Scale(perpN, -kPlaneHalfSize)), Scale(cross, kPlaneHalfSize)),
	    Add(Add(center, Scale(perpN, -kPlaneHalfSize)), Scale(cross, -kPlaneHalfSize)),
	    Add(Add(center, Scale(perpN, kPlaneHalfSize)), Scale(cross, -kPlaneHalfSize)),
	};
	Vector3 screen[4];
	for (int i = 0; i < 4; i++)
		screen[i] = Transform(Transform(points[i], viewProjectionMatrix), viewportMatrix);
	Novice::DrawLine(int(screen[0].x), int(screen[0].y), int(screen[1].x), int(screen[1].y), color);
	Novice::DrawLine(int(screen[1].x), int(screen[1].y), int(screen[2].x), int(screen[2].y), color);
	Novice::DrawLine(int(screen[2].x), int(screen[2].y), int(screen[3].x), int(screen[3].y), color);
	Novice::DrawLine(int(screen[3].x), int(screen[3].y), int(screen[0].x), int(screen[0].y), color);
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 cameraTranslate = {0.0f, 0.0f, -9.5f};
	Vector3 cameraRotate = {0.52f, 0.0f, 0.0f};

	Segment segment = {
	    {-2.0f, -1.0f, 0.0f},
        {3.0f,  2.0f,  2.0f}
    };

	Plane plane = {
	    {0.0f, 1.0f, 0.0f},
        0.0f
    };

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		// 法線を正規化
		float normalLen = Length(plane.normal);
		if (normalLen > 0.0f) {
			plane.normal = Scale(plane.normal, 1.0f / normalLen);
		}

		bool collision = IsCollision(segment, plane);

		ImGui::Begin("Window");
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);
		ImGui::DragFloat3("Segment origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment diff", &segment.diff.x, 0.01f);
		ImGui::DragFloat3("Plane.Normal", &plane.normal.x, 0.01f);
		ImGui::DragFloat("Plane.Distance", &plane.distance, 0.01f);
		ImGui::End();

		Matrix4x4 cameraRotateX = MakeRotateXMatrix(cameraRotate.x);
		Matrix4x4 cameraRotateY = MakeRotateYMatrix(cameraRotate.y);
		Matrix4x4 cameraRotateMatrix = Multiply(cameraRotateY, cameraRotateX);
		Matrix4x4 cameraTranslateMatrix = MakeTranslateMatrix(cameraTranslate.x, cameraTranslate.y, cameraTranslate.z);
		Matrix4x4 cameraMatrix = Multiply(cameraTranslateMatrix, cameraRotateMatrix);
		Matrix4x4 viewMatrix = MakeInverseMatrix(cameraMatrix);

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
		DrawPlane(plane, viewProjectionMatrix, viewportMatrix, WHITE);

		uint32_t segmentColor = collision ? RED : WHITE;
		Vector3 start = Transform(Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
		Vector3 end = Transform(Transform(Add(segment.origin, segment.diff), viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), segmentColor);

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