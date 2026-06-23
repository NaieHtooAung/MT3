#define NOMINMAX
#include <Novice.h>
#include <cmath>
#include <imgui.h>

const char kWindowTitle[] = "GC2A_04_ネイ_トウーアウン";
const float kPi = 3.14159265358979323846f;

struct Vector3 {
	float x;
	float y;
	float z;

	Vector3 operator+(const Vector3& v) const { return {x + v.x, y + v.y, z + v.z}; }
	Vector3 operator-(const Vector3& v) const { return {x - v.x, y - v.y, z - v.z}; }
	Vector3 operator*(float s) const { return {x * s, y * s, z * s}; }
};

struct Matrix4x4 {
	float m[4][4];

	Matrix4x4 operator*(const Matrix4x4& other) const {
		Matrix4x4 result = {};
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				for (int k = 0; k < 4; k++)
					result.m[i][j] += m[i][k] * other.m[k][j];
		return result;
	}
};

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

Matrix4x4 MakeScaleMatrix(const Vector3& s) {
	Matrix4x4 r = {};
	r.m[0][0] = s.x;
	r.m[1][1] = s.y;
	r.m[2][2] = s.z;
	r.m[3][3] = 1.0f;
	return r;
}

Matrix4x4 MakeTranslateMatrix(const Vector3& t) {
	Matrix4x4 r = {};
	r.m[0][0] = 1.0f;
	r.m[1][1] = 1.0f;
	r.m[2][2] = 1.0f;
	r.m[3][0] = t.x;
	r.m[3][1] = t.y;
	r.m[3][2] = t.z;
	r.m[3][3] = 1.0f;
	return r;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 S = MakeScaleMatrix(scale);
	Matrix4x4 R = MakeRotateXMatrix(rotate.x) * MakeRotateYMatrix(rotate.y) * MakeRotateZMatrix(rotate.z);
	Matrix4x4 T = MakeTranslateMatrix(translate);
	return S * R * T;
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = {0};
	char preKeys[256] = {0};

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		// Vector3 operator overload usage — matching the reference slide exactly
		Vector3 a{0.2f, 1.0f, 0.0f};
		Vector3 b{2.4f, 3.1f, 1.2f};
		Vector3 c = a + b;    // operator+
		Vector3 d = a - b;    // operator-
		Vector3 e = a * 2.4f; // operator* (scalar)

		// Matrix4x4 operator* overload usage
		Vector3 rotate{0.4f, 1.43f, -0.8f};
		Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
		Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
		Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
		Matrix4x4 rotateMatrix = rotateXMatrix * rotateYMatrix * rotateZMatrix; // operator*

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		ImGui::Begin("Window");
		ImGui::Text("c:%f, %f, %f", c.x, c.y, c.z);
		ImGui::Text("d:%f, %f, %f", d.x, d.y, d.z);
		ImGui::Text("e:%f, %f, %f", e.x, e.y, e.z);
		ImGui::Text(
		    "matrix:\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n", rotateMatrix.m[0][0], rotateMatrix.m[0][1], rotateMatrix.m[0][2], rotateMatrix.m[0][3], rotateMatrix.m[1][0],
		    rotateMatrix.m[1][1], rotateMatrix.m[1][2], rotateMatrix.m[1][3], rotateMatrix.m[2][0], rotateMatrix.m[2][1], rotateMatrix.m[2][2], rotateMatrix.m[2][3], rotateMatrix.m[3][0],
		    rotateMatrix.m[3][1], rotateMatrix.m[3][2], rotateMatrix.m[3][3]);
		ImGui::End();

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