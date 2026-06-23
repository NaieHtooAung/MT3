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
float Dot(const Vector3& v1, const Vector3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
Vector3 Scale(const Vector3& v, float s) { return {v.x * s, v.y * s, v.z * s}; }

Matrix4x4 Multiply(Matrix4x4 m1, Matrix4x4 m2) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++)
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
	return result;
}

Matrix4x4 MakeScaleMatrix(float sx, float sy, float sz) {
	Matrix4x4 r = {};
	r.m[0][0] = sx;
	r.m[1][1] = sy;
	r.m[2][2] = sz;
	r.m[3][3] = 1.0f;
	return r;
}

Matrix4x4 MakeRotateXMatrix(float angle) {
	Matrix4x4 r = {};
	r.m[0][0] = 1.0f;
	r.m[1][1] = std::cos(angle);
	r.m[1][2] = std::sin(angle);
	r.m[2][1] = -std::sin(angle);
	r.m[2][2] = std::cos(angle);
	r.m[3][3] = 1.0f;
	return r;
}

Matrix4x4 MakeRotateYMatrix(float angle) {
	Matrix4x4 r = {};
	r.m[0][0] = std::cos(angle);
	r.m[0][2] = -std::sin(angle);
	r.m[1][1] = 1.0f;
	r.m[2][0] = std::sin(angle);
	r.m[2][2] = std::cos(angle);
	r.m[3][3] = 1.0f;
	return r;
}

Matrix4x4 MakeRotateZMatrix(float angle) {
	Matrix4x4 r = {};
	r.m[0][0] = std::cos(angle);
	r.m[0][1] = std::sin(angle);
	r.m[1][0] = -std::sin(angle);
	r.m[1][1] = std::cos(angle);
	r.m[2][2] = 1.0f;
	r.m[3][3] = 1.0f;
	return r;
}

Matrix4x4 MakeRotateXYZMatrix(float x, float y, float z) { return Multiply(Multiply(MakeRotateXMatrix(x), MakeRotateYMatrix(y)), MakeRotateZMatrix(z)); }

Matrix4x4 MakeTranslateMatrix(float x, float y, float z) {
	Matrix4x4 r = {};
	r.m[0][0] = 1.0f;
	r.m[1][1] = 1.0f;
	r.m[2][2] = 1.0f;
	r.m[3][0] = x;
	r.m[3][1] = y;
	r.m[3][2] = z;
	r.m[3][3] = 1.0f;
	return r;
}

// Builds a local SRT matrix: Scale * Rotate * Translate
Matrix4x4 MakeLocalMatrix(const Vector3& translate, const Vector3& rotate, const Vector3& scale) {
	return Multiply(Multiply(MakeScaleMatrix(scale.x, scale.y, scale.z), MakeRotateXYZMatrix(rotate.x, rotate.y, rotate.z)), MakeTranslateMatrix(translate.x, translate.y, translate.z));
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
	Matrix4x4 r = {};
	r.m[0][0] = width / 2.0f;
	r.m[1][1] = -height / 2.0f;
	r.m[2][2] = maxZ - minZ;
	r.m[3][0] = x + width / 2.0f;
	r.m[3][1] = y + height / 2.0f;
	r.m[3][2] = minZ;
	r.m[3][3] = 1.0f;
	return r;
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

void DrawGrid(const Matrix4x4& vp, const Matrix4x4& viewport) {
	const float kGridHalfWidth = 2.0f;
	const uint32_t kSubdivision = 10;
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision);
	for (uint32_t i = 0; i <= kSubdivision; i++) {
		float x = -kGridHalfWidth + float(i) * kGridEvery;
		Vector3 s = Transform(Transform({x, 0.0f, -kGridHalfWidth}, vp), viewport);
		Vector3 e = Transform(Transform({x, 0.0f, kGridHalfWidth}, vp), viewport);
		Novice::DrawLine(int(s.x), int(s.y), int(e.x), int(e.y), (i == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF);
	}
	for (uint32_t i = 0; i <= kSubdivision; i++) {
		float z = -kGridHalfWidth + float(i) * kGridEvery;
		Vector3 s = Transform(Transform({-kGridHalfWidth, 0.0f, z}, vp), viewport);
		Vector3 e = Transform(Transform({kGridHalfWidth, 0.0f, z}, vp), viewport);
		Novice::DrawLine(int(s.x), int(s.y), int(e.x), int(e.y), (i == kSubdivision / 2) ? 0x000000FF : 0xAAAAAAFF);
	}
}

// Draws a wireframe sphere at a world-space position
void DrawSphere(const Vector3& center, float radius, const Matrix4x4& vp, const Matrix4x4& viewport, uint32_t color) {
	const int kDiv = 16;
	for (int lat = 0; lat < kDiv; lat++) {
		float latA = kPi * (-0.5f + float(lat) / kDiv);
		float latB = kPi * (-0.5f + float(lat + 1) / kDiv);
		float cosA = std::cos(latA), sinA = std::sin(latA);
		float cosB = std::cos(latB), sinB = std::sin(latB);
		for (int lon = 0; lon < kDiv; lon++) {
			float lonA = 2.0f * kPi * float(lon) / kDiv;
			float lonB = 2.0f * kPi * float(lon + 1) / kDiv;
			Vector3 pA = {center.x + radius * cosA * std::cos(lonA), center.y + radius * sinA, center.z + radius * cosA * std::sin(lonA)};
			Vector3 pB = {center.x + radius * cosA * std::cos(lonB), center.y + radius * sinA, center.z + radius * cosA * std::sin(lonB)};
			Vector3 pC = {center.x + radius * cosB * std::cos(lonA), center.y + radius * sinB, center.z + radius * cosB * std::sin(lonA)};
			Vector3 sA = Transform(Transform(pA, vp), viewport);
			Vector3 sB = Transform(Transform(pB, vp), viewport);
			Vector3 sC = Transform(Transform(pC, vp), viewport);
			Novice::DrawLine(int(sA.x), int(sA.y), int(sB.x), int(sB.y), color);
			Novice::DrawLine(int(sA.x), int(sA.y), int(sC.x), int(sC.y), color);
		}
	}
}

// Draws a line between two world-space positions
void DrawBone(const Vector3& from, const Vector3& to, const Matrix4x4& vp, const Matrix4x4& viewport) {
	Vector3 s = Transform(Transform(from, vp), viewport);
	Vector3 e = Transform(Transform(to, vp), viewport);
	Novice::DrawLine(int(s.x), int(s.y), int(e.x), int(e.y), WHITE);
}

// Extracts the world-space position (translation row) from a world matrix
Vector3 ExtractPosition(const Matrix4x4& worldMatrix) { return {worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2]}; }

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 cameraTranslate = {0.0f, 0.9f, -9.49f};
	Vector3 cameraRotate = {0.26f, 0.0f, 0.0f};

	// Joint 0: Shoulder (root)
	Vector3 translates[3] = {
	    {0.0f, 1.0f, 0.0f}, // shoulder world offset
	    {0.4f, 1.0f, 0.0f}, // elbow local offset from shoulder
	    {0.5f, 0.0f, 0.0f}, // hand local offset from elbow
	};
	Vector3 rotates[3] = {
	    {0.0f, 0.0f, -5.683f}, // shoulder rotate
	    {0.0f, 0.0f, -1.528f}, // elbow rotate
	    {0.0f, 0.0f, 0.0f   }, // hand rotate
	};
	Vector3 scales[3] = {
	    {1.0f, 1.0f, 1.0f},
	    {1.0f, 1.0f, 1.0f},
	    {1.0f, 1.0f, 1.0f},
	};

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		// Build world matrices: each child inherits its parent's world matrix
		Matrix4x4 localMatrix[3];
		Matrix4x4 worldMatrix[3];
		for (int i = 0; i < 3; i++)
			localMatrix[i] = MakeLocalMatrix(translates[i], rotates[i], scales[i]);

		worldMatrix[0] = localMatrix[0];                           // shoulder: no parent
		worldMatrix[1] = Multiply(localMatrix[1], worldMatrix[0]); // elbow inherits shoulder
		worldMatrix[2] = Multiply(localMatrix[2], worldMatrix[1]); // hand inherits elbow

		// Extract world positions for each joint
		Vector3 posA = ExtractPosition(worldMatrix[0]); // shoulder
		Vector3 posB = ExtractPosition(worldMatrix[1]); // elbow
		Vector3 posC = ExtractPosition(worldMatrix[2]); // hand

		// ImGui controls
		ImGui::Begin("Window");
		ImGui::DragFloat3("translates[0]", &translates[0].x, 0.01f);
		ImGui::DragFloat3("rotates[0]", &rotates[0].x, 0.01f);
		ImGui::DragFloat3("scales[0]", &scales[0].x, 0.01f);
		ImGui::Separator();
		ImGui::DragFloat3("translates[1]", &translates[1].x, 0.01f);
		ImGui::DragFloat3("rotates[1]", &rotates[1].x, 0.01f);
		ImGui::DragFloat3("scales[1]", &scales[1].x, 0.01f);
		ImGui::Separator();
		ImGui::DragFloat3("translates[2]", &translates[2].x, 0.01f);
		ImGui::DragFloat3("rotates[2]", &rotates[2].x, 0.01f);
		ImGui::DragFloat3("scales[2]", &scales[2].x, 0.01f);
		ImGui::End();

		// Build view matrix
		Matrix4x4 cameraRotateMatrix = Multiply(MakeRotateYMatrix(cameraRotate.y), MakeRotateXMatrix(cameraRotate.x));
		Matrix4x4 cameraTranslateMatrix = MakeTranslateMatrix(cameraTranslate.x, cameraTranslate.y, cameraTranslate.z);
		Matrix4x4 cameraMatrix = Multiply(cameraTranslateMatrix, cameraRotateMatrix);
		Matrix4x4 viewMatrix = MakeInverseMatrix(cameraMatrix);

		// Build perspective projection matrix
		Matrix4x4 projectionMatrix = {};
		float fovY = 0.45f, aspect = 1280.0f / 720.0f, nearZ = 0.1f, farZ = 100.0f;
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

		// Draw bones (lines connecting joints)
		DrawBone(posA, posB, viewProjectionMatrix, viewportMatrix); // shoulder → elbow
		DrawBone(posB, posC, viewProjectionMatrix, viewportMatrix); // elbow → hand

		// Draw joint spheres: red=shoulder, green=elbow, blue=hand
		DrawSphere(posA, 0.08f, viewProjectionMatrix, viewportMatrix, RED);
		DrawSphere(posB, 0.08f, viewProjectionMatrix, viewportMatrix, GREEN);
		DrawSphere(posC, 0.08f, viewProjectionMatrix, viewportMatrix, BLUE);

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