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

typedef struct Matrix4x4 {
	float m[4][4];
} Matrix4x4;

// Returns the component-wise sum of two vectors
Vector3 Add(const Vector3& v1, const Vector3& v2) { return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }

// Returns the component-wise difference of two vectors
Vector3 Subtract(const Vector3& v1, const Vector3& v2) { return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }

// Returns the dot product of two vectors
float Dot(const Vector3& v1, const Vector3& v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }

// Returns a vector scaled by scalar s
Vector3 Scale(const Vector3& v, float s) { return {v.x * s, v.y * s, v.z * s}; }

// Returns the length (magnitude) of a vector
float Length(const Vector3& v) { return std::sqrt(Dot(v, v)); }

// Returns the cross product of two vectors
Vector3 Cross(const Vector3& v1, const Vector3& v2) { return {v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x}; }

// Returns a unit vector in the same direction; returns the original vector if length is zero
Vector3 Normalize(const Vector3& v) {
	float len = Length(v);
	if (len > 0.0f)
		return Scale(v, 1.0f / len);
	return v;
}

// Returns the product of two 4x4 matrices
Matrix4x4 Multiply(Matrix4x4 m1, Matrix4x4 m2) {
	Matrix4x4 result = {};
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++)
				result.m[i][j] += m1.m[i][k] * m2.m[k][j];
	return result;
}

// Returns a rotation matrix around the X axis by the given angle (radians)
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

// Returns a rotation matrix around the Y axis by the given angle (radians)
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

// Returns a rotation matrix around the Z axis by the given angle (radians)
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

// Returns a combined rotation matrix applied in X -> Y -> Z order
Matrix4x4 MakeRotateXYZMatrix(float x, float y, float z) { return Multiply(Multiply(MakeRotateXMatrix(x), MakeRotateYMatrix(y)), MakeRotateZMatrix(z)); }

// Returns a translation matrix for the given x, y, z offsets
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

// Returns the inverse of a rotation-translation matrix by transposing the 3x3 rotation part
// and recomputing the translation row accordingly
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

// Returns a viewport matrix that maps NDC coordinates to screen pixel coordinates
// x, y: top-left corner of the viewport; width, height: dimensions; minZ, maxZ: depth range
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

// Transforms a 3D vertex by a 4x4 matrix and performs perspective divide
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

// OBB vs OBB - Hyperplane Separation Theorem (SAT):
// Projects both OBBs onto 15 candidate separating axes:
//   - 3 face normals of OBB A
//   - 3 face normals of OBB B
//   - 9 cross products of each pair of edges (one from A, one from B)
// For each axis, computes the projection radius of each OBB and the distance
// between centers along that axis. If any axis shows separation, the OBBs do
// not collide. If no separating axis is found, the OBBs are colliding.
bool IsCollision(const OBB& obbA, const OBB& obbB) {
	// Compute the vector between the two OBB centers
	Vector3 distance = Subtract(obbB.center, obbA.center);

	// Gather all 15 candidate separating axes
	Vector3 axes[15];

	// 3 face normals of OBB A
	axes[0] = obbA.orientation[0];
	axes[1] = obbA.orientation[1];
	axes[2] = obbA.orientation[2];

	// 3 face normals of OBB B
	axes[3] = obbB.orientation[0];
	axes[4] = obbB.orientation[1];
	axes[5] = obbB.orientation[2];

	// 9 edge cross products (one edge axis from A crossed with one from B)
	int idx = 6;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			axes[idx++] = Cross(obbA.orientation[i], obbB.orientation[j]);
		}
	}

	// Test each axis for separation
	for (int i = 0; i < 15; i++) {
		// Skip near-zero axes that arise from parallel edges
		if (Length(axes[i]) < 1e-6f)
			continue;

		Vector3 axis = Normalize(axes[i]);

		// Project OBB A's half-extents onto the axis
		float rA =
		    std::abs(Dot(Scale(obbA.orientation[0], obbA.size.x), axis)) + std::abs(Dot(Scale(obbA.orientation[1], obbA.size.y), axis)) + std::abs(Dot(Scale(obbA.orientation[2], obbA.size.z), axis));

		// Project OBB B's half-extents onto the axis
		float rB =
		    std::abs(Dot(Scale(obbB.orientation[0], obbB.size.x), axis)) + std::abs(Dot(Scale(obbB.orientation[1], obbB.size.y), axis)) + std::abs(Dot(Scale(obbB.orientation[2], obbB.size.z), axis));

		// Project the center-to-center vector onto the axis
		float d = std::abs(Dot(distance, axis));

		// If the projected distance exceeds the sum of radii, a separating axis exists
		if (d > rA + rB)
			return false;
	}

	// No separating axis found — OBBs are colliding
	return true;
}

// Draws a world-space grid on the XZ plane using subdivided lines;
// the center lines along each axis are drawn in black, others in gray
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

// Draws the 12 edges of an OBB by computing its 8 corners from the center,
// orientation axes, and half-size extents, then projecting each edge to screen space
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

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 cameraTranslate = {0.0f, 0.0f, -9.5f};
	Vector3 cameraRotate = {0.52f, 0.0f, 0.0f};

	OBB obbA = {
	    .center = {0.0f,0.0f,0.0f},
	    .orientation = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	    .size = {0.83f,0.26f,0.24f},
	};
	Vector3 obbRotateA = {0.0f, 0.0f, 0.0f};

	OBB obbB = {
	    .center = {0.9f,0.66f,0.78f},
	    .orientation = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	    .size = {0.5f,0.37f,0.5f},
	};
	Vector3 obbRotateB = {-0.05f, -2.49f, 0.15f};

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		// Update OBB A orientation axes from its current rotation angles
		Matrix4x4 rotXYZ_A = MakeRotateXYZMatrix(obbRotateA.x, obbRotateA.y, obbRotateA.z);
		obbA.orientation[0] = {rotXYZ_A.m[0][0], rotXYZ_A.m[0][1], rotXYZ_A.m[0][2]};
		obbA.orientation[1] = {rotXYZ_A.m[1][0], rotXYZ_A.m[1][1], rotXYZ_A.m[1][2]};
		obbA.orientation[2] = {rotXYZ_A.m[2][0], rotXYZ_A.m[2][1], rotXYZ_A.m[2][2]};

		// Update OBB B orientation axes from its current rotation angles
		Matrix4x4 rotXYZ_B = MakeRotateXYZMatrix(obbRotateB.x, obbRotateB.y, obbRotateB.z);
		obbB.orientation[0] = {rotXYZ_B.m[0][0], rotXYZ_B.m[0][1], rotXYZ_B.m[0][2]};
		obbB.orientation[1] = {rotXYZ_B.m[1][0], rotXYZ_B.m[1][1], rotXYZ_B.m[1][2]};
		obbB.orientation[2] = {rotXYZ_B.m[2][0], rotXYZ_B.m[2][1], rotXYZ_B.m[2][2]};

		// Test whether the two OBBs intersect using the Hyperplane Separation Theorem
		bool collision = IsCollision(obbA, obbB);

		// ImGui controls for camera, OBB A, and OBB B parameters
		ImGui::Begin("Window");
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);
		ImGui::Separator();
		ImGui::DragFloat3("OBB A Center", &obbA.center.x, 0.01f);
		ImGui::DragFloat3("OBB A Size", &obbA.size.x, 0.01f);
		ImGui::DragFloat3("OBB A Rotate", &obbRotateA.x, 0.01f);
		ImGui::Separator();
		ImGui::DragFloat3("OBB B Center", &obbB.center.x, 0.01f);
		ImGui::DragFloat3("OBB B Size", &obbB.size.x, 0.01f);
		ImGui::DragFloat3("OBB B Rotate", &obbRotateB.x, 0.01f);
		ImGui::End();

		// Build view matrix from camera rotation and translation
		Matrix4x4 cameraRotateX = MakeRotateXMatrix(cameraRotate.x);
		Matrix4x4 cameraRotateY = MakeRotateYMatrix(cameraRotate.y);
		Matrix4x4 cameraRotateMatrix = Multiply(cameraRotateY, cameraRotateX);
		Matrix4x4 cameraTranslateMatrix = MakeTranslateMatrix(cameraTranslate.x, cameraTranslate.y, cameraTranslate.z);
		Matrix4x4 cameraMatrix = Multiply(cameraTranslateMatrix, cameraRotateMatrix);
		Matrix4x4 viewMatrix = MakeInverseMatrix(cameraMatrix);

		// Build perspective projection matrix
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

		// Combine view and projection, then build the viewport transform
		Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewportMatrix = makeViewportMatrix(0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		// Draw scene; turn RED on collision, WHITE otherwise
		DrawGrid(viewProjectionMatrix, viewportMatrix);
		DrawOBB(obbA, viewProjectionMatrix, viewportMatrix, collision ? RED : WHITE);
		DrawOBB(obbB, viewProjectionMatrix, viewportMatrix, collision ? RED : WHITE);

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