#include <Novice.h>
#include <math.h>
const char kWindowTitle[] = "GC2A_04_ネイ_トウーアウン";

typedef struct Vector3 {

	float x;
	float y;
	float z;

}Vector3;

Vector3 addVector3(const Vector3& v1,const Vector3& v2) {
	Vector3 result = {};
	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;
	return result;
};
Vector3 subVector3(const Vector3& v1,const Vector3& v2) {
	Vector3 result = {};
	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;
	return result;
};
Vector3 mulVector3(float k, const Vector3& v) {
	Vector3 result = {};
	result.x = k * v.x;
	result.y = k * v.y;
	result.z = k * v.z;
	return result;
};
float dotVector3(const Vector3& v1,const Vector3& v2) {
	float result = {};
	result = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	return result;
};
float lengthVector3(const Vector3& v) {
	float result = {};
	result = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	return result;
};
Vector3 normalizeVector3(const Vector3& v) {
	Vector3 result = {};
	float length = lengthVector3(v);
	result.x = v.x / length;
	result.y = v.y / length;
	result.z = v.z / length;
	return result;
};
static const int kColoumWidth = 60;
static const int kRowHeight = 20;
void VectorScreenPrintf(int x, int y, const Vector3& vector, const char* label) {

	Novice::ScreenPrintf(x, y, "%.02f", vector.x);
	Novice::ScreenPrintf(x + kColoumWidth, y, "%.02f", vector.y);
	Novice::ScreenPrintf(x + kColoumWidth * 2, y, "%.02f", vector.z);
	Novice::ScreenPrintf(x + kColoumWidth * 3, y, "%s", label);
}
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 v1 = {1.0f,3.0f,-5.0f};
	Vector3 v2 = {4.0f,-1.0f,2.0f};
	float k = {4.0f};

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
		Vector3 add = addVector3(v1, v2);
		Vector3 sub = subVector3(v1, v2);
		Vector3 mul = mulVector3(k, v1);
		float resultDot = dotVector3(v1, v2);
		float lengthV1 = lengthVector3(v1);
		Vector3 normalizeV2 = normalizeVector3(v2);
		
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		VectorScreenPrintf(0, 0, add, " : Add");
		VectorScreenPrintf(0, kRowHeight, sub, " : Subtract");
		VectorScreenPrintf(0, kRowHeight * 2, mul, " : Multiply");
		Novice::ScreenPrintf(0, kRowHeight * 3, "%.02f : Dot", resultDot);
		Novice::ScreenPrintf(0, kRowHeight * 4, "%.02f : Length", lengthV1);
		VectorScreenPrintf(0, kRowHeight * 5, normalizeV2, " : Normalize");
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
