#include <Novice.h>
#include <cmath>
const char kWindowTitle[] = "GC2A_04_ネイ_トウーアウン";
typedef struct Vector3 {

	float x;
	float y;
	float z;

} Vector3;

typedef struct Matrix4x4 {
	float m[4][4];
} Matrix4x4;

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
	result.m[1][1] = std::cos(radius);
	result.m[1][2] = std::sin(radius);
	result.m[2][1] = -std::sin(radius);
	result.m[2][2] = std::cos(radius);
	result.m[3][3] = 1.0f;
	return result;
}
Matrix4x4 MakeRotateYMatrix(float radius) {
	Matrix4x4 result = {};
	result.m[0][0] = std::cos(radius);
	result.m[0][2] = -std::sin(radius);
	result.m[1][1] = 1.0f;
	result.m[2][0] = std::sin(radius);
	result.m[2][2] = std::cos(radius);
	result.m[3][3] = 1.0f;
	return result;
}
Matrix4x4 MakeRotateZMatrix(float radius) {

	Matrix4x4 result = {};
	result.m[0][0] = std::cos(radius);
	result.m[0][1] = std::sin(radius);
	result.m[1][0] = -std::sin(radius);
	result.m[1][1] = std::cos(radius);
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	return result;
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

	Vector3 rotate = {0.4f, 1.43f, -0.8f};

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
		Matrix4x4 rotateX = MakeRotateXMatrix(rotate.x);
		Matrix4x4 rotateY = MakeRotateYMatrix(rotate.y);
		Matrix4x4 rotateZ = MakeRotateZMatrix(rotate.z);
		Matrix4x4 rotateXYZMatrix = Multiply(rotateX, Multiply(rotateY, rotateZ));
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		MatrixScreenPrintf(0, 0, rotateX, "rotateMatrix X");
		MatrixScreenPrintf(0, kRowHeight * 5, rotateY, "rotateMatrix Y");
		MatrixScreenPrintf(0, kRowHeight * 10, rotateZ, "rotateMatrix Z");
		MatrixScreenPrintf(0, kRowHeight * 15, rotateXYZMatrix, "rotateMatrix XYZ");
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