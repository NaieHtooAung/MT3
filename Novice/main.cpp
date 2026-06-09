#define NOMINMAX
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

struct OBB {
	Vector3 center;
	Vector3 orientation[3];
	Vector3 size;
};

struct Segment {
	Vector3 origin;
	Vector3 diff;
};

typedef struct Matrix4x4 {
	float m[4][4];
} Matrix4x4;

Vector3 Add(const Vector3& v1, const Vector3& v2) { return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }
Vector3 Subtract(const Vector3& v1, const Vector3& v2) { return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }
float Dot(const Vector3& v1, const Vector3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
Vector3 Scale(const Vector3& v, float s) { return {v.x * s, v.y * s, v.z * s}; }
float Length(const Vector3& v) { return std::sqrt(Dot(v, v)); }
Vector3 Cross(const Vector3& v1, const Vector3& v2) { return {v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x}; }
Vector3 Normalize(const Vector3& v) {
	float len = Length(v);
	if (len > 0.0f)
		return Scale(v, 1.0f / len);
	return v;
}

bool IsCollision(const OBB& obb, const Segment& seg) {
	Vector3 d = Subtract(seg.origin, obb.center);

	float localOrigin[3], localDiff[3], sizes[3];
	localOrigin[0] = Dot(d, obb.orientation[0]);
	localOrigin[1] = Dot(d, obb.orientation[1]);
	localOrigin[2] = Dot(d, obb.orientation[2]);
	localDiff[0] = Dot(seg.diff, obb.orientation[0]);
	localDiff[1] = Dot(seg.diff, obb.orientation[1]);
	localDiff[2] = Dot(seg.diff, obb.orientation[2]);
	sizes[0] = obb.size.x;
	sizes[1] = obb.size.y;
	sizes[2] = obb.size.z;

	float tMin = 0.0f;
	float tMax = 1.0f;

	for (int i = 0; i < 3; i++) {
		if (std::abs(localDiff[i]) < 1e-6f) {
			if (localOrigin[i] < -sizes[i] || localOrigin[i] > sizes[i])
				return false;
		} else {
			float t1 = (-sizes[i] - localOrigin[i]) / localDiff[i];
			float t2 = (sizes[i] - localOrigin[i]) / localDiff[i];
			if (t1 > t2)
				std::swap(t1, t2);
			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);
			if (tMin > tMax)
				return false;
		}
	}
	return true;
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

Matrix4x4 MakeRotateZMatrix(float angle) {
	Matrix4x4 result = {};
	result.m[0][0] = std::cos(angle);
	result.m[0][1] = std::sin(angle);
	result.m[1][0] = -std::sin(angle);
	result.m[1][1] = std::cos(angle);
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	return result;
}

Matrix4x4 MakeRotateXYZMatrix(float x, float y, float z) { return Multiply(Multiply(MakeRotateXMatrix(x), MakeRotateYMatrix(y)), MakeRotateZMatrix(z)); }

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

void DrawOBB(const OBB& obb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 ax = Scale(obb.orientation[0], obb.size.x);
	Vector3 ay = Scale(obb.orientation[1], obb.size.y);
	Vector3 az = Scale(obb.orientation[2], obb.size.z);

	Vector3 corners[8] = {
	    Add(Add(Add(obb.center, ax), ay), az),
	    Add(Add(Add(obb.center, ax), ay), Scale(az, -1)),
	    Add(Add(Add(obb.center, ax), Scale(ay, -1)), az),
	    Add(Add(Add(obb.center, ax), Scale(ay, -1)), Scale(az, -1)),
	    Add(Add(Add(obb.center, Scale(ax, -1)), ay), az),
	    Add(Add(Add(obb.center, Scale(ax, -1)), ay), Scale(az, -1)),
	    Add(Add(Add(obb.center, Scale(ax, -1)), Scale(ay, -1)), az),
	    Add(Add(Add(obb.center, Scale(ax, -1)), Scale(ay, -1)), Scale(az, -1)),
	};

	int indices[12][2] = {
	    {0, 1},
        {1, 3},
        {3, 2},
        {2, 0},
        {4, 5},
        {5, 7},
        {7, 6},
        {6, 4},
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7}
    };

	for (int i = 0; i < 12; i++) {
		Vector3 start = Transform(Transform(corners[indices[i][0]], viewProjectionMatrix), viewportMatrix);
		Vector3 end = Transform(Transform(corners[indices[i][1]], viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), color);
	}
}

void DrawSegment(const Segment& seg, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	Vector3 screenStart = Transform(Transform(seg.origin, viewProjectionMatrix), viewportMatrix);
	Vector3 screenEnd = Transform(Transform(Add(seg.origin, seg.diff), viewProjectionMatrix), viewportMatrix);
	Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), WHITE);
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 cameraTranslate = {0.0f, 0.0f, -9.5f};
	Vector3 cameraRotate = {0.52f, 0.0f, 0.0f};

	OBB obb = {
	    .center = {0.0f,               0.0f,               0.0f              },
	    .orientation = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	    .size = {0.5f,               0.5f,               0.5f              },
	};
	Vector3 obbRotate = {0.0f, 0.0f, 0.0f};

	Segment segment = {
	    {-0.7f, 0.3f,  0.0f},
        {2.0f,  -0.5f, 0.0f}
    };

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		Matrix4x4 rotXYZ = MakeRotateXYZMatrix(obbRotate.x, obbRotate.y, obbRotate.z);
		obb.orientation[0] = {rotXYZ.m[0][0], rotXYZ.m[0][1], rotXYZ.m[0][2]};
		obb.orientation[1] = {rotXYZ.m[1][0], rotXYZ.m[1][1], rotXYZ.m[1][2]};
		obb.orientation[2] = {rotXYZ.m[2][0], rotXYZ.m[2][1], rotXYZ.m[2][2]};

		bool collision = IsCollision(obb, segment);

		ImGui::Begin("Window");
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);
		ImGui::DragFloat3("OBB Center", &obb.center.x, 0.01f);
		ImGui::DragFloat3("OBB Size", &obb.size.x, 0.01f);
		ImGui::DragFloat3("OBB Rotate", &obbRotate.x, 0.01f);
		ImGui::DragFloat3("Segment origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment diff", &segment.diff.x, 0.01f);
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
		DrawOBB(obb, viewProjectionMatrix, viewportMatrix, collision ? RED : WHITE);
		DrawSegment(segment, viewProjectionMatrix, viewportMatrix);

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