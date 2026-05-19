#include <Novice.h>
#include <math.h>
#include<cstring>
const char kWindowTitle[] = "GC2A_04_ネイ_トウーアウン";

typedef struct Vector3 {
	float x;
	float y;
	float z;
} Vector3;

typedef struct Matrix4x4 {
	float m[4][4];
} Matrix4x4;

Matrix4x4 MakeTranslateMatrix(const Vector3& translate) {

	Matrix4x4 result{};

	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;

	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;

	return result;
}
Matrix4x4 MakeScaleMatrix(const Vector3& scale) {

	Matrix4x4 result{};

	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = scale.z;
	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4 Multiply(Matrix4x4 m1, Matrix4x4 m2) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
			}
		}
	}
	return result;
}
Matrix4x4 MakeRotateXMatrix(float radius) {
	Matrix4x4 result = {};

	result.m[0][0] = 1.0f;

	result.m[1][1] = cosf(radius);
	result.m[1][2] = -sinf(radius);

	result.m[2][1] = sinf(radius);
	result.m[2][2] = cosf(radius);

	result.m[3][3] = 1.0f;

	return result;
}
Matrix4x4 MakeRotateYMatrix(float radius) {
	Matrix4x4 result = {};

	result.m[0][0] = cosf(radius);
	result.m[0][2] = sinf(radius);

	result.m[1][1] = 1.0f;

	result.m[2][0] = -sinf(radius);
	result.m[2][2] = cosf(radius);

	result.m[3][3] = 1.0f;

	return result;
}
Matrix4x4 MakeRotateZMatrix(float radius) {

	Matrix4x4 result = {};
	result.m[0][0] = cosf(radius);
	result.m[0][1] = -sinf(radius);
	result.m[1][0] = sinf(radius);
	result.m[1][1] = cosf(radius);
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	return result;
}
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);
	return Multiply(scaleMatrix, Multiply(rotateXMatrix, Multiply(rotateYMatrix, Multiply(rotateZMatrix, translateMatrix))));
}
Matrix4x4 Inverse(const Matrix4x4& matrix) {
	Matrix4x4 result = {};

	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;

	result.m[3][0] = -matrix.m[3][0];
	result.m[3][1] = -matrix.m[3][1];
	result.m[3][2] = -matrix.m[3][2];

	return result;
}
Matrix4x4 MakePerspectiveFovmatrix(float fovY, float aspect, float nearZ, float farZ) {
	Matrix4x4 result = {};
	float f = 1.0f / tanf(fovY / 2.0f);
	result.m[0][0] = f / aspect;
	result.m[1][1] = f;
	result.m[2][2] = (farZ + nearZ) / (nearZ - farZ);
	result.m[2][3] = (2.0f * farZ * nearZ) / (nearZ - farZ);
	result.m[3][2] = -1.0f;
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
Vector3 Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result{};

	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;

	return result;
}
static const int kWindowWidth = 1280;
static const int kWindowHeight = 720;

static const int kColoumnWidth = 60;
static const int kRowHeight = 20;
void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label) {

	Novice::ScreenPrintf(x, y, "X: %.2f", vector.x);
	Novice::ScreenPrintf(x, y + kRowHeight, "Y: %.2f", vector.y);
	Novice::ScreenPrintf(x, y + kRowHeight * 2, "Z: %.2f", vector.z);

	Novice::ScreenPrintf(x + kColoumnWidth * 3, y + kRowHeight, "%s", label);
}
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	
		Vector3 cameraPosition = {0.0f, 0.0f, 5.0f};

	Vector3 scale{0.1f, 0.1f, 0.1f};
	Vector3 rotate{0.0f, 0.0f, 0.0f};
	Vector3 translate{0.0f, 0.0f, 10.0f};

	Vector3 v1 = {1.2f, -3.9f, 2.5f};
	Vector3 v2 = {2.8f, 0.4f, -1.3f};
	Vector3 cross = Cross(v1, v2);

	Vector3 kLocalVertices[3] = {
	    {0.0f,  -1.0f, 0.0f},
        {-1.0f, 1.0f,  0.0f},
        {1.0f,  1.0f,  0.0f}
    };

	bool isRotate = false;

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	
	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		Matrix4x4 worldMatrix = MakeAffineMatrix(scale, rotate, translate);
		Matrix4x4 cameraMatrix = MakeAffineMatrix(scale, {0.0f, 0.0f, 0.0f}, cameraPosition);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovmatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);

		Matrix4x4 worldViewMatrix = Multiply(worldMatrix, viewMatrix);

		Matrix4x4 worldViewProjectionMatrix = Multiply(worldViewMatrix, projectionMatrix);
		Matrix4x4 viewportMatrix = makeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);
	

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///
		if (keys[DIK_Y] && !preKeys[DIK_Y]) {
			isRotate = !isRotate;
		}

		if (isRotate) {
			rotate.y += 0.02f;
		}
		if (keys[DIK_A]) {
			cameraPosition.x -= 0.05f;
		}

		if (keys[DIK_D]) {
			cameraPosition.x += 0.05f;
		}

		if (keys[DIK_W]) {
			cameraPosition.z -= 0.05f;
		}

		if (keys[DIK_S]) {
			cameraPosition.z += 0.05f;
		}
		Vector3 screenVertices[3];
		for (int i = 0; i < 3; i++) {
			Vector3 ndcVertex = Transform(kLocalVertices[i], worldViewProjectionMatrix);
			screenVertices[i] = Transform(ndcVertex, viewportMatrix);
		}
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		VectorScreenPrintf(0, 0, cross, "Cross");

		Novice::DrawTriangle(int(screenVertices[0].x), int(screenVertices[0].y), int(screenVertices[1].x), int(screenVertices[1].y), int(screenVertices[2].x), int(screenVertices[2].y),RED,kFillModeSolid);
		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
