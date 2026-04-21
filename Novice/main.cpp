#include <Novice.h>
#include <math.h>
const char kWindowTitle[] = "GC2A_04_ネイ_トウーアウン";
typedef struct Matrix4x4 {

	float m[4][4];

} Matrix4x4;

Matrix4x4 add(const Matrix4x4& m1, const Matrix4x4 m2) {
	Matrix4x4 result{};
	for (int i = 0; i < 4; i++) {
		for (int y = 0; y < 4; y++) {
			result.m[i][y] = m1.m[i][y] + m2.m[i][y];
		}
	}
	return result;
}
Matrix4x4 subtract(const Matrix4x4& m1, const Matrix4x4& m2)
{

	Matrix4x4 result{};
	for (int i = 0; i < 4; i++) {
		for (int y = 0; y < 4; y++) {
			result.m[i][y] = m1.m[i][y] - m2.m[i][y];
		}
	}
	return result;

}
Matrix4x4 multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result{};
	for (int i = 0; i < 4; i++) {
		for (int y = 0; y < 4; y++) {
			result.m[i][y] = m1.m[i][0] * m2.m[0][y] + m1.m[i][1] * m2.m[1][y] + m1.m[i][2] * m2.m[2][y] + m1.m[i][3] * m2.m[3][y];
		}
	}
	return result;
}
Matrix4x4 inverseM(const Matrix4x4& m) {
	Matrix4x4 result{};
	float a[4][8];

	// copy + identity
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			a[i][j] = m.m[i][j];
			a[i][j + 4] = (i == j); // identity
		}
	}

	// make left side = identity
	for (int i = 0; i < 4; i++) {

		float pivot = a[i][i];
		if (pivot == 0)
			return result; // no inverse

		// divide row
		for (int j = 0; j < 8; j++) {
			a[i][j] /= pivot;
		}

		// remove other rows
		for (int k = 0; k < 4; k++) {
			if (k == i)
				continue;

			float f = a[k][i];
			for (int j = 0; j < 8; j++) {
				a[k][j] -= f * a[i][j];
			}
		}
	}

	// copy result
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = a[i][j + 4];
		}
	}

	return result;
}
Matrix4x4 transposeM1(const Matrix4x4& m) {
	Matrix4x4 result{};
	for (int i = 0; i < 4; i++) {
		for (int y = 0; y < 4; y++) {
			result.m[i][y] = m.m[y][i];
		}
	}
	return result;
}
Matrix4x4 transposeM2(const Matrix4x4& m) {
	Matrix4x4 result{};
	for (int i = 0; i < 4; i++) {
		for (int y = 0; y < 4; y++) {
			result.m[i][y] = m.m[y][i];
		}
	}
	return result;
}
Matrix4x4 MakeIdentity4x4() 
{ 
	Matrix4x4 result{};
	for (int i = 0; i < 4; i++) {
		for (int y = 0; y < 4; y++) {
			if ( i == y) {
				result.m[i][y] = 1.0f;
			} 
		}
	}

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
	
	Matrix4x4 m1 = {
	3.2f,0.7f,9.6f,4.4f,
	5.5f,1.3f,7.8f,2.1f,
	6.9f,8.0f,2.6f,1.0f,
	0.5f,7.2f,5.1f,3.3f
	};
	Matrix4x4 m2 = {
	4.1f,6.5f,3.3f,2.2f,
	8.8f,0.6f,9.9f,7.7f,
	1.1f,5.5f,6.6f,0.0f,
	3.3f,9.9f,8.8f,2.2f
	};
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
		Matrix4x4 addResult = add(m1, m2);
		Matrix4x4 multiplyResult = multiply(m1, m2);
		Matrix4x4 subtractResult = subtract(m1, m2);
		Matrix4x4 inverseM1Result = inverseM(m1);
		Matrix4x4 inverseM2Result = inverseM(m2);
		Matrix4x4 transposeM1Result = transposeM1(m1);
		Matrix4x4 transposeM2Result = transposeM2(m2);
		Matrix4x4 identitfy = MakeIdentity4x4();
		
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		
		MatrixScreenPrintf(0, 20, addResult, "Add");
		MatrixScreenPrintf(0, kRowHeight * 5 + 20, subtractResult, "Subtract");
		MatrixScreenPrintf(0, kRowHeight * 5 * 2 + 20, multiplyResult, "Multiply");
		MatrixScreenPrintf(0,kRowHeight * 5 * 3 + 20,inverseM1Result, "Inverse M1");
		MatrixScreenPrintf(0, kRowHeight * 5 * 4 + 20, inverseM2Result, "Inverse M2");
		MatrixScreenPrintf(kColumnWidth * 5, 20, transposeM1Result, "Transpose M1");
		MatrixScreenPrintf(kColumnWidth * 5,kRowHeight * 5 + 20,transposeM2Result, "Transpose M2");
		MatrixScreenPrintf(kColumnWidth * 5,kRowHeight * 5 * 2 + 20,identitfy, "Identity");

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
