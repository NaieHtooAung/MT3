#include <Novice.h>
#include <math.h>

const char kWindowTitle[] = "GC2A_04_ネイ_トウーアウン";

typedef struct Vector3 {

	float x;
	float y;
	float z;

} Vector3;

// Add
Vector3 Add(const Vector3& v1, const Vector3& v2) {
	Vector3 result = {};

	result.x = v1.x + v2.x;
	result.y = v1.y + v2.y;
	result.z = v1.z + v2.z;

	return result;
}

// Subtract
Vector3 Subtract(const Vector3& v1, const Vector3& v2) {
	Vector3 result = {};

	result.x = v1.x - v2.x;
	result.y = v1.y - v2.y;
	result.z = v1.z - v2.z;

	return result;
}

// Multiply
Vector3 Multiply(float k, const Vector3& v) {
	Vector3 result = {};

	result.x = k * v.x;
	result.y = k * v.y;
	result.z = k * v.z;

	return result;
}

// Dot
float Dot(const Vector3& v1, const Vector3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

// Length
float Length(const Vector3& v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }

// Normalize
Vector3 Normalize(const Vector3& v) {

	Vector3 result = {};

	float length = Length(v);

	result.x = v.x / length;
	result.y = v.y / length;
	result.z = v.z / length;

	return result;
}

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

	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 v1 = {1.0f, 3.0f, -5.0f};
	Vector3 v2 = {4.0f, -1.0f, 2.0f};

	float k = 4.0f;

	while (Novice::ProcessMessage() == 0) {

		Novice::BeginFrame();

		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		// 計算
		Vector3 add = Add(v1, v2);
		Vector3 sub = Subtract(v1, v2);
		Vector3 mul = Multiply(k, v1);

		float dot = Dot(v1, v2);
		float length = Length(v1);

		Vector3 normalize = Normalize(v2);

		// 表示
		VectorScreenPrintf(0, 0, add, " : Add");
		VectorScreenPrintf(0, kRowHeight, sub, " : Subtract");
		VectorScreenPrintf(0, kRowHeight * 2, mul, " : Multiply");

		Novice::ScreenPrintf(0, kRowHeight * 3, "%.02f : Dot", dot);

		Novice::ScreenPrintf(0, kRowHeight * 4, "%.02f : Length", length);

		VectorScreenPrintf(0, kRowHeight * 5, normalize, " : Normalize");

		Novice::EndFrame();

		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	Novice::Finalize();

	return 0;
}