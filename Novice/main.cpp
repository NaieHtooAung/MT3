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

struct AABB {
	Vector3 center;
	Vector3 size;
};

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

// OBB vs Segment:
// Builds the OBB world matrix from its orientation axes and center,
// inverts it to get the OBB local space transform,
// transforms both segment endpoints into local space,
// then runs a slab test against the local AABB [-size, +size].
// Returns true if the overlapping t range intersects [0, 1] (the segment extent).
bool IsCollision(const OBB& obb, const Segment& segment) {
	// Build OBB world matrix from orientation axes and center
	Matrix4x4 obbWorld = {};
	obbWorld.m[0][0] = obb.orientation[0].x;
	obbWorld.m[0][1] = obb.orientation[0].y;
	obbWorld.m[0][2] = obb.orientation[0].z;
	obbWorld.m[1][0] = obb.orientation[1].x;
	obbWorld.m[1][1] = obb.orientation[1].y;
	obbWorld.m[1][2] = obb.orientation[1].z;
	obbWorld.m[2][0] = obb.orientation[2].x;
	obbWorld.m[2][1] = obb.orientation[2].y;
	obbWorld.m[2][2] = obb.orientation[2].z;
	obbWorld.m[3][0] = obb.center.x;
	obbWorld.m[3][1] = obb.center.y;
	obbWorld.m[3][2] = obb.center.z;
	obbWorld.m[3][3] = 1.0f;

	// Invert to get transform from world space into OBB local space
	Matrix4x4 obbInverse = MakeInverseMatrix(obbWorld);

	// Transform segment endpoints into OBB local space
	Vector3 localOrigin = Transform(segment.origin, obbInverse);
	Vector3 localEnd = Transform(Add(segment.origin, segment.diff), obbInverse);
	Vector3 localDiff = Subtract(localEnd, localOrigin);

	// Slab test: find t range where the segment overlaps each axis-aligned slab
	float tMin = -1e10f, tMax = 1e10f;
	float dirs[3] = {localDiff.x, localDiff.y, localDiff.z};
	float origs[3] = {localOrigin.x, localOrigin.y, localOrigin.z};
	float sizes[3] = {obb.size.x, obb.size.y, obb.size.z};

	for (int i = 0; i < 3; i++) {
		if (std::abs(dirs[i]) < 1e-8f) {
			// Segment is parallel to this slab; origin must lie within it
			if (origs[i] < -sizes[i] || origs[i] > sizes[i])
				return false;
		} else {
			// Compute intersection t values with the two slab planes
			float t1 = (-sizes[i] - origs[i]) / dirs[i];
			float t2 = (sizes[i] - origs[i]) / dirs[i];
			if (t1 > t2)
				std::swap(t1, t2);
			tMin = std::max(tMin, t1);
			tMax = std::min(tMax, t2);
			if (tMin > tMax)
				return false;
		}
	}
	// Segment spans t in [0, 1]; check if the slab overlap intersects that range
	return tMax >= 0.0f && tMin <= 1.0f;
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

// Draws a segment as a single line from origin to origin+diff in screen space
void DrawSegment(const Segment& segment, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	Vector3 screenStart = Transform(Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
	Vector3 screenEnd = Transform(Transform(Add(segment.origin, segment.diff), viewProjectionMatrix), viewportMatrix);
	Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y),WHITE);
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	Novice::Initialize(kWindowTitle, 1280, 720);

	char keys[256] = {0};
	char preKeys[256] = {0};

	Vector3 cameraTranslate = {0.0f, 0.0f, -9.5f};
	Vector3 cameraRotate = {0.52f, 0.0f, 0.0f};

	OBB obb = {
	    .center = {-1.0f,              0.0f,               0.0f              },
	    .orientation = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	    .size = {0.5f,               0.5f,               0.5f              },
	};
	Vector3 obbRotate = {0.0f, 0.0f, 0.0f};

	Segment segment = {
	    .origin = {-0.8f, -0.3f, 0.0f},
	    .diff = {0.5f,  0.5f,  0.5f},
	};

	while (Novice::ProcessMessage() == 0) {
		Novice::BeginFrame();
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		// Update OBB orientation axes from the current rotation angles
		Matrix4x4 rotXYZ = MakeRotateXYZMatrix(obbRotate.x, obbRotate.y, obbRotate.z);
		obb.orientation[0] = {rotXYZ.m[0][0], rotXYZ.m[0][1], rotXYZ.m[0][2]};
		obb.orientation[1] = {rotXYZ.m[1][0], rotXYZ.m[1][1], rotXYZ.m[1][2]};
		obb.orientation[2] = {rotXYZ.m[2][0], rotXYZ.m[2][1], rotXYZ.m[2][2]};

		// Test whether the segment intersects the OBB
		bool collision = IsCollision(obb, segment);

		// ImGui controls for camera, OBB, and segment parameters
		ImGui::Begin("Window");
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRotate", &cameraRotate.x, 0.01f);
		ImGui::DragFloat3("OBB Center", &obb.center.x, 0.01f);
		ImGui::DragFloat3("OBB Size", &obb.size.x, 0.01f);
		ImGui::DragFloat3("OBB Rotate", &obbRotate.x, 0.01f);
		ImGui::DragFloat3("Segment Origin", &segment.origin.x, 0.01f);
		ImGui::DragFloat3("Segment Diff", &segment.diff.x, 0.01f);
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