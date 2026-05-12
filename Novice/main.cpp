#include <Novice.h>
#include <math.h>
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
	result.m[1][2] = sinf(radius);
	result.m[2][1] = -sinf(radius);
	result.m[2][2] = cosf(radius);
	result.m[3][3] = 1.0f;
	return result;
}
Matrix4x4 MakeRotateYMatrix(float radius) {
	Matrix4x4 result = {};
	result.m[0][0] = cosf(radius);
	result.m[0][2] = -sinf(radius);
	result.m[1][1] = 1.0f;
	result.m[2][0] = sinf(radius);
	result.m[2][2] = cosf(radius);
	result.m[3][3] = 1.0f;
	return result;
}
Matrix4x4 MakeRotateZMatrix(float radius) {

	Matrix4x4 result = {};
	result.m[0][0] = cosf(radius);
	result.m[0][1] = sinf(radius);
	result.m[1][0] = -sinf(radius);
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
static const int kRowHeight = 20;
static const int kColumnWidth = 60;
void MatrixScreenPrintf(int x, int y, const Matrix4x4& matrix, const char* label) {
	Novice::ScreenPrintf(x, y - 20, label);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {

			Novice::ScreenPrintf(x + j * kColumnWidth, y + i * kRowHeight, "%6.02f", matrix.m[i][j]);
		}
	}
}
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 scale{1.2f,0.79f,-2.1f};
	Vector3 rotate{0.4f,1.43f,-0.8f};
	Vector3 translate{2.7f,-4.15f,1.57f};
	Matrix4x4 worldMatrix = MakeAffineMatrix(scale, rotate, translate);

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///
		
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		MatrixScreenPrintf(0, 100, worldMatrix, "worldMatrix");
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
