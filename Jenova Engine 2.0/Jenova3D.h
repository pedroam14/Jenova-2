
#ifndef OLC_PGEX_GFX3D
#define OLC_PGEX_GFX3D

#include <algorithm>
#include <vector>
#include <list>
#undef min
#undef max

namespace JenovaSpace
{
//Container class for Advanced 2D Drawing functions
class GFX3D : public JenovaSpace::JenovaExtension
{

public:
	struct Vector2D
	{
		float x = 0;
		float y = 0;
		float z = 0;
	};

	struct Vector3D
	{
		float x = 0;
		float y = 0;
		float z = 0;
		float w = 1; //Need a 4th term to perform sensible matrix vector multiplication
	};

	struct Triangle
	{
		Vector3D p[3];
		Vector2D t[3];
		JenovaSpace::Pixel col;
	};

	struct Matrix4x4
	{
		float m[4][4] = {0};
	};

	struct Mesh
	{
		std::vector<Triangle> tris;
	};

	class Math
	{
	public:
		inline Math();

	public:
		inline static Vector3D Mat_MultiplyVector(Matrix4x4 &m, Vector3D &i);
		inline static Matrix4x4 Mat_MultiplyMatrix(Matrix4x4 &m1, Matrix4x4 &m2);
		inline static Matrix4x4 Mat_MakeIdentity();
		inline static Matrix4x4 Mat_MakeRotationX(float fAngleRad);
		inline static Matrix4x4 Mat_MakeRotationY(float fAngleRad);
		inline static Matrix4x4 Mat_MakeRotationZ(float fAngleRad);
		inline static Matrix4x4 Mat_MakeScale(float x, float y, float z);
		inline static Matrix4x4 Mat_MakeTranslation(float x, float y, float z);
		inline static Matrix4x4 Mat_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar);
		inline static Matrix4x4 Mat_PointAt(Vector3D &pos, Vector3D &target, Vector3D &up);
		inline static Matrix4x4 Mat_QuickInverse(Matrix4x4 &m); //Only for Rotation/Translation Matrices
		inline static Matrix4x4 Mat_Inverse(JenovaSpace::GFX3D::Matrix4x4 &m);

		inline static Vector3D Vec_Add(Vector3D &v1, Vector3D &v2);
		inline static Vector3D Vec_Sub(Vector3D &v1, Vector3D &v2);
		inline static Vector3D Vec_Mul(Vector3D &v1, float k);
		inline static Vector3D Vec_Div(Vector3D &v1, float k);
		inline static float Vec_DotProduct(Vector3D &v1, Vector3D &v2);
		inline static float Vec_Length(Vector3D &v);
		inline static Vector3D Vec_Normalise(Vector3D &v);
		inline static Vector3D Vec_CrossProduct(Vector3D &v1, Vector3D &v2);
		inline static Vector3D Vec_IntersectPlane(Vector3D &plane_p, Vector3D &plane_n, Vector3D &lineStart, Vector3D &lineEnd, float &t);

		inline static int Triangle_ClipAgainstPlane(Vector3D plane_p, Vector3D plane_n, Triangle &in_tri, Triangle &out_tri1, Triangle &out_tri2);
	};

	enum RENDERFLAGS
	{
		RENDER_WIRE = 0x01,
		RENDER_FLAT = 0x02,
		RENDER_TEXTURED = 0x04,
		RENDER_CULL_CW = 0x08,
		RENDER_CULL_CCW = 0x10,
		RENDER_DEPTH = 0x20,
	};

	class PipeLine
	{
	public:
		PipeLine();
		bool affine;

	public:
		void SetProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar, float fLeft, float fTop, float fWidth, float fHeight);
		void SetCamera(JenovaSpace::GFX3D::Vector3D &pos, JenovaSpace::GFX3D::Vector3D &lookat, JenovaSpace::GFX3D::Vector3D &up);
		void SetTransform(JenovaSpace::GFX3D::Matrix4x4 &transform);
		void SetTexture(JenovaSpace::Sprite *texture);
		void SetLightSource(JenovaSpace::GFX3D::Vector3D &pos, JenovaSpace::GFX3D::Vector3D &dir, JenovaSpace::Pixel &col);
		uint32_t Render(std::vector<JenovaSpace::GFX3D::Triangle> &Triangles, uint32_t flags = RENDER_CULL_CW | RENDER_TEXTURED | RENDER_DEPTH);
		void SetTextureMappingMode(bool bAffine);

	private:
		JenovaSpace::GFX3D::Matrix4x4 matProj;
		JenovaSpace::GFX3D::Matrix4x4 matView;
		JenovaSpace::GFX3D::Matrix4x4 matWorld;
		JenovaSpace::Sprite *sprTexture;
		float fViewX;
		float fViewY;
		float fViewW;
		float fViewH;
	};

public:
	//static const int RF_TEXTURE = 0x00000001;
	//static const int RF_ = 0x00000002;

	inline static void ConfigureDisplay();
	inline static void ClearDepth();
	inline static void AddTriangleToScene(JenovaSpace::GFX3D::Triangle &tri);
	inline static void RenderScene();

	inline static void DrawTriangleFlat(JenovaSpace::GFX3D::Triangle &tri);
	inline static void DrawTriangleWire(JenovaSpace::GFX3D::Triangle &tri, JenovaSpace::Pixel col = JenovaSpace::WHITE);
	inline static void DrawTriangleTex(JenovaSpace::GFX3D::Triangle &tri, JenovaSpace::Sprite *spr);
	inline static void TexturedTriangle(int x1, int y1, float u1, float v1, float w1,
										int x2, int y2, float u2, float v2, float w2,
										int x3, int y3, float u3, float v3, float w3, JenovaSpace::Sprite *spr, bool affine);

	//Draws a sprite with the transform applied
	//inline static void DrawSprite(JenovaSpace::Sprite *sprite, JenovaSpace::GFX2D::Transform2D &transform);

private:
	static float *m_DepthBuffer;
};
} //namespace JenovaSpace

namespace JenovaSpace
{
JenovaSpace::GFX3D::Math::Math()
{
}
JenovaSpace::GFX3D::Vector3D JenovaSpace::GFX3D::Math::Mat_MultiplyVector(JenovaSpace::GFX3D::Matrix4x4 &m, JenovaSpace::GFX3D::Vector3D &i)
{
	Vector3D v;
	v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
	v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
	v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
	v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
	return v;
}

JenovaSpace::GFX3D::Matrix4x4 JenovaSpace::GFX3D::Math::Mat_MakeIdentity()
{
	JenovaSpace::GFX3D::Matrix4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

JenovaSpace::GFX3D::Matrix4x4 JenovaSpace::GFX3D::Math::Mat_MakeRotationX(float fAngleRad)
{
	JenovaSpace::GFX3D::Matrix4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = cosf(fAngleRad);
	matrix.m[1][2] = sinf(fAngleRad);
	matrix.m[2][1] = -sinf(fAngleRad);
	matrix.m[2][2] = cosf(fAngleRad);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

JenovaSpace::GFX3D::Matrix4x4 JenovaSpace::GFX3D::Math::Mat_MakeRotationY(float fAngleRad)
{
	JenovaSpace::GFX3D::Matrix4x4 matrix;
	matrix.m[0][0] = cosf(fAngleRad);
	matrix.m[0][2] = sinf(fAngleRad);
	matrix.m[2][0] = -sinf(fAngleRad);
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = cosf(fAngleRad);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

JenovaSpace::GFX3D::Matrix4x4 JenovaSpace::GFX3D::Math::Mat_MakeRotationZ(float fAngleRad)
{
	JenovaSpace::GFX3D::Matrix4x4 matrix;
	matrix.m[0][0] = cosf(fAngleRad);
	matrix.m[0][1] = sinf(fAngleRad);
	matrix.m[1][0] = -sinf(fAngleRad);
	matrix.m[1][1] = cosf(fAngleRad);
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

JenovaSpace::GFX3D::Matrix4x4 JenovaSpace::GFX3D::Math::Mat_MakeScale(float x, float y, float z)
{
	JenovaSpace::GFX3D::Matrix4x4 matrix;
	matrix.m[0][0] = x;
	matrix.m[1][1] = y;
	matrix.m[2][2] = z;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

JenovaSpace::GFX3D::Matrix4x4 JenovaSpace::GFX3D::Math::Mat_MakeTranslation(float x, float y, float z)
{
	JenovaSpace::GFX3D::Matrix4x4 matrix;
	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = 1.0f;
	matrix.m[2][2] = 1.0f;
	matrix.m[3][3] = 1.0f;
	matrix.m[3][0] = x;
	matrix.m[3][1] = y;
	matrix.m[3][2] = z;
	return matrix;
}

JenovaSpace::GFX3D::Matrix4x4 JenovaSpace::GFX3D::Math::Mat_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
{
	float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
	JenovaSpace::GFX3D::Matrix4x4 matrix;
	matrix.m[0][0] = fAspectRatio * fFovRad;
	matrix.m[1][1] = fFovRad;
	matrix.m[2][2] = fFar / (fFar - fNear);
	matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	matrix.m[2][3] = 1.0f;
	matrix.m[3][3] = 0.0f;
	return matrix;
}

JenovaSpace::GFX3D::Matrix4x4 JenovaSpace::GFX3D::Math::Mat_MultiplyMatrix(JenovaSpace::GFX3D::Matrix4x4 &m1, JenovaSpace::GFX3D::Matrix4x4 &m2)
{
	JenovaSpace::GFX3D::Matrix4x4 matrix;
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
	return matrix;
}

JenovaSpace::GFX3D::Matrix4x4 JenovaSpace::GFX3D::Math::Mat_PointAt(JenovaSpace::GFX3D::Vector3D &pos, JenovaSpace::GFX3D::Vector3D &target, JenovaSpace::GFX3D::Vector3D &up)
{
	//Calculate new forward direction
	JenovaSpace::GFX3D::Vector3D newForward = Vec_Sub(target, pos);
	newForward = Vec_Normalise(newForward);

	//Calculate new Up direction
	JenovaSpace::GFX3D::Vector3D a = Vec_Mul(newForward, Vec_DotProduct(up, newForward));
	JenovaSpace::GFX3D::Vector3D newUp = Vec_Sub(up, a);
	newUp = Vec_Normalise(newUp);

	//New Right direction is easy, its just cross product
	JenovaSpace::GFX3D::Vector3D newRight = Vec_CrossProduct(newUp, newForward);

	//Construct Dimensioning and Translation Matrix
	JenovaSpace::GFX3D::Matrix4x4 matrix;
	matrix.m[0][0] = newRight.x;
	matrix.m[0][1] = newRight.y;
	matrix.m[0][2] = newRight.z;
	matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = newUp.x;
	matrix.m[1][1] = newUp.y;
	matrix.m[1][2] = newUp.z;
	matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = newForward.x;
	matrix.m[2][1] = newForward.y;
	matrix.m[2][2] = newForward.z;
	matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = pos.x;
	matrix.m[3][1] = pos.y;
	matrix.m[3][2] = pos.z;
	matrix.m[3][3] = 1.0f;
	return matrix;
}

JenovaSpace::GFX3D::Matrix4x4 JenovaSpace::GFX3D::Math::Mat_QuickInverse(JenovaSpace::GFX3D::Matrix4x4 &m) //Only for Rotation/Translation Matrices
{
	JenovaSpace::GFX3D::Matrix4x4 matrix;
	matrix.m[0][0] = m.m[0][0];
	matrix.m[0][1] = m.m[1][0];
	matrix.m[0][2] = m.m[2][0];
	matrix.m[0][3] = 0.0f;
	matrix.m[1][0] = m.m[0][1];
	matrix.m[1][1] = m.m[1][1];
	matrix.m[1][2] = m.m[2][1];
	matrix.m[1][3] = 0.0f;
	matrix.m[2][0] = m.m[0][2];
	matrix.m[2][1] = m.m[1][2];
	matrix.m[2][2] = m.m[2][2];
	matrix.m[2][3] = 0.0f;
	matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
	matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
	matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
	matrix.m[3][3] = 1.0f;
	return matrix;
}

JenovaSpace::GFX3D::Matrix4x4 JenovaSpace::GFX3D::Math::Mat_Inverse(JenovaSpace::GFX3D::Matrix4x4 &m)
{
	float det;

	Matrix4x4 matInv;

	matInv.m[0][0] = m.m[1][1] * m.m[2][2] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2] - m.m[2][1] * m.m[1][2] * m.m[3][3] + m.m[2][1] * m.m[1][3] * m.m[3][2] + m.m[3][1] * m.m[1][2] * m.m[2][3] - m.m[3][1] * m.m[1][3] * m.m[2][2];
	matInv.m[1][0] = -m.m[1][0] * m.m[2][2] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2] + m.m[2][0] * m.m[1][2] * m.m[3][3] - m.m[2][0] * m.m[1][3] * m.m[3][2] - m.m[3][0] * m.m[1][2] * m.m[2][3] + m.m[3][0] * m.m[1][3] * m.m[2][2];
	matInv.m[2][0] = m.m[1][0] * m.m[2][1] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1] - m.m[2][0] * m.m[1][1] * m.m[3][3] + m.m[2][0] * m.m[1][3] * m.m[3][1] + m.m[3][0] * m.m[1][1] * m.m[2][3] - m.m[3][0] * m.m[1][3] * m.m[2][1];
	matInv.m[3][0] = -m.m[1][0] * m.m[2][1] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1] + m.m[2][0] * m.m[1][1] * m.m[3][2] - m.m[2][0] * m.m[1][2] * m.m[3][1] - m.m[3][0] * m.m[1][1] * m.m[2][2] + m.m[3][0] * m.m[1][2] * m.m[2][1];
	matInv.m[0][1] = -m.m[0][1] * m.m[2][2] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2] + m.m[2][1] * m.m[0][2] * m.m[3][3] - m.m[2][1] * m.m[0][3] * m.m[3][2] - m.m[3][1] * m.m[0][2] * m.m[2][3] + m.m[3][1] * m.m[0][3] * m.m[2][2];
	matInv.m[1][1] = m.m[0][0] * m.m[2][2] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2] - m.m[2][0] * m.m[0][2] * m.m[3][3] + m.m[2][0] * m.m[0][3] * m.m[3][2] + m.m[3][0] * m.m[0][2] * m.m[2][3] - m.m[3][0] * m.m[0][3] * m.m[2][2];
	matInv.m[2][1] = -m.m[0][0] * m.m[2][1] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1] + m.m[2][0] * m.m[0][1] * m.m[3][3] - m.m[2][0] * m.m[0][3] * m.m[3][1] - m.m[3][0] * m.m[0][1] * m.m[2][3] + m.m[3][0] * m.m[0][3] * m.m[2][1];
	matInv.m[3][1] = m.m[0][0] * m.m[2][1] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1] - m.m[2][0] * m.m[0][1] * m.m[3][2] + m.m[2][0] * m.m[0][2] * m.m[3][1] + m.m[3][0] * m.m[0][1] * m.m[2][2] - m.m[3][0] * m.m[0][2] * m.m[2][1];
	matInv.m[0][2] = m.m[0][1] * m.m[1][2] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2] - m.m[1][1] * m.m[0][2] * m.m[3][3] + m.m[1][1] * m.m[0][3] * m.m[3][2] + m.m[3][1] * m.m[0][2] * m.m[1][3] - m.m[3][1] * m.m[0][3] * m.m[1][2];
	matInv.m[1][2] = -m.m[0][0] * m.m[1][2] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2] + m.m[1][0] * m.m[0][2] * m.m[3][3] - m.m[1][0] * m.m[0][3] * m.m[3][2] - m.m[3][0] * m.m[0][2] * m.m[1][3] + m.m[3][0] * m.m[0][3] * m.m[1][2];
	matInv.m[2][2] = m.m[0][0] * m.m[1][1] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1] - m.m[1][0] * m.m[0][1] * m.m[3][3] + m.m[1][0] * m.m[0][3] * m.m[3][1] + m.m[3][0] * m.m[0][1] * m.m[1][3] - m.m[3][0] * m.m[0][3] * m.m[1][1];
	matInv.m[3][2] = -m.m[0][0] * m.m[1][1] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1] + m.m[1][0] * m.m[0][1] * m.m[3][2] - m.m[1][0] * m.m[0][2] * m.m[3][1] - m.m[3][0] * m.m[0][1] * m.m[1][2] + m.m[3][0] * m.m[0][2] * m.m[1][1];
	matInv.m[0][3] = -m.m[0][1] * m.m[1][2] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2] + m.m[1][1] * m.m[0][2] * m.m[2][3] - m.m[1][1] * m.m[0][3] * m.m[2][2] - m.m[2][1] * m.m[0][2] * m.m[1][3] + m.m[2][1] * m.m[0][3] * m.m[1][2];
	matInv.m[1][3] = m.m[0][0] * m.m[1][2] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2] - m.m[1][0] * m.m[0][2] * m.m[2][3] + m.m[1][0] * m.m[0][3] * m.m[2][2] + m.m[2][0] * m.m[0][2] * m.m[1][3] - m.m[2][0] * m.m[0][3] * m.m[1][2];
	matInv.m[2][3] = -m.m[0][0] * m.m[1][1] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1] + m.m[1][0] * m.m[0][1] * m.m[2][3] - m.m[1][0] * m.m[0][3] * m.m[2][1] - m.m[2][0] * m.m[0][1] * m.m[1][3] + m.m[2][0] * m.m[0][3] * m.m[1][1];
	matInv.m[3][3] = m.m[0][0] * m.m[1][1] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1] - m.m[1][0] * m.m[0][1] * m.m[2][2] + m.m[1][0] * m.m[0][2] * m.m[2][1] + m.m[2][0] * m.m[0][1] * m.m[1][2] - m.m[2][0] * m.m[0][2] * m.m[1][1];

	det = m.m[0][0] * matInv.m[0][0] + m.m[0][1] * matInv.m[1][0] + m.m[0][2] * matInv.m[2][0] + m.m[0][3] * matInv.m[3][0];
	//	if (det == 0) return false;

	det = 1.0 / det;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			matInv.m[i][j] *= (float)det;

	return matInv;
}

JenovaSpace::GFX3D::Vector3D JenovaSpace::GFX3D::Math::Vec_Add(JenovaSpace::GFX3D::Vector3D &v1, JenovaSpace::GFX3D::Vector3D &v2)
{
	return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

JenovaSpace::GFX3D::Vector3D JenovaSpace::GFX3D::Math::Vec_Sub(JenovaSpace::GFX3D::Vector3D &v1, JenovaSpace::GFX3D::Vector3D &v2)
{
	return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

JenovaSpace::GFX3D::Vector3D JenovaSpace::GFX3D::Math::Vec_Mul(JenovaSpace::GFX3D::Vector3D &v1, float k)
{
	return {v1.x * k, v1.y * k, v1.z * k};
}

JenovaSpace::GFX3D::Vector3D JenovaSpace::GFX3D::Math::Vec_Div(JenovaSpace::GFX3D::Vector3D &v1, float k)
{
	return {v1.x / k, v1.y / k, v1.z / k};
}

float JenovaSpace::GFX3D::Math::Vec_DotProduct(JenovaSpace::GFX3D::Vector3D &v1, JenovaSpace::GFX3D::Vector3D &v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float JenovaSpace::GFX3D::Math::Vec_Length(JenovaSpace::GFX3D::Vector3D &v)
{
	return sqrtf(Vec_DotProduct(v, v));
}

JenovaSpace::GFX3D::Vector3D JenovaSpace::GFX3D::Math::Vec_Normalise(JenovaSpace::GFX3D::Vector3D &v)
{
	float l = Vec_Length(v);
	return {v.x / l, v.y / l, v.z / l};
}

JenovaSpace::GFX3D::Vector3D JenovaSpace::GFX3D::Math::Vec_CrossProduct(JenovaSpace::GFX3D::Vector3D &v1, JenovaSpace::GFX3D::Vector3D &v2)
{
	Vector3D v;
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}

JenovaSpace::GFX3D::Vector3D JenovaSpace::GFX3D::Math::Vec_IntersectPlane(JenovaSpace::GFX3D::Vector3D &plane_p, JenovaSpace::GFX3D::Vector3D &plane_n, JenovaSpace::GFX3D::Vector3D &lineStart, JenovaSpace::GFX3D::Vector3D &lineEnd, float &t)
{
	plane_n = Vec_Normalise(plane_n);
	float plane_d = -Vec_DotProduct(plane_n, plane_p);
	float ad = Vec_DotProduct(lineStart, plane_n);
	float bd = Vec_DotProduct(lineEnd, plane_n);
	t = (-plane_d - ad) / (bd - ad);
	JenovaSpace::GFX3D::Vector3D lineStartToEnd = Vec_Sub(lineEnd, lineStart);
	JenovaSpace::GFX3D::Vector3D lineToIntersect = Vec_Mul(lineStartToEnd, t);
	return Vec_Add(lineStart, lineToIntersect);
}

int JenovaSpace::GFX3D::Math::Triangle_ClipAgainstPlane(Vector3D plane_p, Vector3D plane_n, Triangle &in_tri, Triangle &out_tri1, Triangle &out_tri2)
{
	//Make sure plane normal is indeed normal
	plane_n = Math::Vec_Normalise(plane_n);

	out_tri1.t[0] = in_tri.t[0];
	out_tri2.t[0] = in_tri.t[0];
	out_tri1.t[1] = in_tri.t[1];
	out_tri2.t[1] = in_tri.t[1];
	out_tri1.t[2] = in_tri.t[2];
	out_tri2.t[2] = in_tri.t[2];

	//Return signed shortest distance from point to plane, plane normal must be normalised
	auto dist = [&](Vector3D &p) {
		Vector3D n = Math::Vec_Normalise(p);
		return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Math::Vec_DotProduct(plane_n, plane_p));
	};

	//Create two temporary storage arrays to classify points either side of plane
	//If distance sign is positive, point lies on "inside" of plane
	Vector3D *inside_points[3];
	int nInsidePointCount = 0;
	Vector3D *outside_points[3];
	int nOutsidePointCount = 0;
	Vector2D *inside_tex[3];
	int nInsideTexCount = 0;
	Vector2D *outside_tex[3];
	int nOutsideTexCount = 0;

	//Get signed distance of each point in Triangle to plane
	float d0 = dist(in_tri.p[0]);
	float d1 = dist(in_tri.p[1]);
	float d2 = dist(in_tri.p[2]);

	if (d0 >= 0)
	{
		inside_points[nInsidePointCount++] = &in_tri.p[0];
		inside_tex[nInsideTexCount++] = &in_tri.t[0];
	}
	else
	{
		outside_points[nOutsidePointCount++] = &in_tri.p[0];
		outside_tex[nOutsideTexCount++] = &in_tri.t[0];
	}
	if (d1 >= 0)
	{
		inside_points[nInsidePointCount++] = &in_tri.p[1];
		inside_tex[nInsideTexCount++] = &in_tri.t[1];
	}
	else
	{
		outside_points[nOutsidePointCount++] = &in_tri.p[1];
		outside_tex[nOutsideTexCount++] = &in_tri.t[1];
	}
	if (d2 >= 0)
	{
		inside_points[nInsidePointCount++] = &in_tri.p[2];
		inside_tex[nInsideTexCount++] = &in_tri.t[2];
	}
	else
	{
		outside_points[nOutsidePointCount++] = &in_tri.p[2];
		outside_tex[nOutsideTexCount++] = &in_tri.t[2];
	}

	//Now classify Triangle points, and break the input Triangle into
	//smaller output Triangles if required. There are four possible
	//outcomes...

	if (nInsidePointCount == 0)
	{
		//All points lie on the outside of plane, so clip whole Triangle
		//It ceases to exist

		return 0; //No returned Triangles are valid
	}

	if (nInsidePointCount == 3)
	{
		//All points lie on the inside of plane, so do nothing
		//and allow the Triangle to simply pass through
		out_tri1 = in_tri;

		return 1; //Just the one returned original Triangle is valid
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		//Triangle should be clipped. As two points lie outside
		//the plane, the Triangle simply becomes a smaller Triangle

		//Copy appearance info to new Triangle
		out_tri1.col = JenovaSpace::MAGENTA; //in_tri.col;

		//The inside point is valid, so keep that...
		out_tri1.p[0] = *inside_points[0];
		out_tri1.t[0] = *inside_tex[0];

		//but the two new points are at the locations where the
		//original sides of the Triangle (lines) intersect with the plane
		float t;
		out_tri1.p[1] = Math::Vec_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
		out_tri1.t[1].x = t * (outside_tex[0]->x - inside_tex[0]->x) + inside_tex[0]->x;
		out_tri1.t[1].y = t * (outside_tex[0]->y - inside_tex[0]->y) + inside_tex[0]->y;
		out_tri1.t[1].z = t * (outside_tex[0]->z - inside_tex[0]->z) + inside_tex[0]->z;

		out_tri1.p[2] = Math::Vec_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1], t);
		out_tri1.t[2].x = t * (outside_tex[1]->x - inside_tex[0]->x) + inside_tex[0]->x;
		out_tri1.t[2].y = t * (outside_tex[1]->y - inside_tex[0]->y) + inside_tex[0]->y;
		out_tri1.t[2].z = t * (outside_tex[1]->z - inside_tex[0]->z) + inside_tex[0]->z;

		return 1; //Return the newly formed single Triangle
	}

	if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		//Triangle should be clipped. As two points lie inside the plane,
		//the clipped Triangle becomes a "quad". Fortunately, we can
		//represent a quad with two new Triangles

		//Copy appearance info to new Triangles
		out_tri1.col = JenovaSpace::GREEN; //in_tri.col;
		out_tri2.col = JenovaSpace::RED;   //in_tri.col;

		//The first Triangle consists of the two inside points and a new
		//point determined by the location where one side of the Triangle
		//intersects with the plane
		out_tri1.p[0] = *inside_points[0];
		out_tri1.t[0] = *inside_tex[0];

		out_tri1.p[1] = *inside_points[1];
		out_tri1.t[1] = *inside_tex[1];

		float t;
		out_tri1.p[2] = Math::Vec_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
		out_tri1.t[2].x = t * (outside_tex[0]->x - inside_tex[0]->x) + inside_tex[0]->x;
		out_tri1.t[2].y = t * (outside_tex[0]->y - inside_tex[0]->y) + inside_tex[0]->y;
		out_tri1.t[2].z = t * (outside_tex[0]->z - inside_tex[0]->z) + inside_tex[0]->z;

		//The second Triangle is composed of one of he inside points, a
		//new point determined by the intersection of the other side of the
		//Triangle and the plane, and the newly created point above
		out_tri2.p[1] = *inside_points[1];
		out_tri2.t[1] = *inside_tex[1];
		out_tri2.p[0] = out_tri1.p[2];
		out_tri2.t[0] = out_tri1.t[2];
		out_tri2.p[2] = Math::Vec_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0], t);
		out_tri2.t[2].x = t * (outside_tex[0]->x - inside_tex[1]->x) + inside_tex[1]->x;
		out_tri2.t[2].y = t * (outside_tex[0]->y - inside_tex[1]->y) + inside_tex[1]->y;
		out_tri2.t[2].z = t * (outside_tex[0]->z - inside_tex[1]->z) + inside_tex[1]->z;
		return 2; //Return two newly formed Triangles which form a quad
	}

	return 0;
}

void GFX3D::DrawTriangleFlat(JenovaSpace::GFX3D::Triangle &tri)
{
	pge->FillTriangle(tri.p[0].x, tri.p[0].y, tri.p[1].x, tri.p[1].y, tri.p[2].x, tri.p[2].y, tri.col);
}

void GFX3D::DrawTriangleWire(JenovaSpace::GFX3D::Triangle &tri, JenovaSpace::Pixel col)
{
	pge->DrawTriangle(tri.p[0].x, tri.p[0].y, tri.p[1].x, tri.p[1].y, tri.p[2].x, tri.p[2].y, col);
}

void GFX3D::TexturedTriangle(int x1, int y1, float u1, float v1, float w1,
							 int x2, int y2, float u2, float v2, float w2,
							 int x3, int y3, float u3, float v3, float w3, JenovaSpace::Sprite *spr, bool affine)

{
	///*
	if (affine)
	{
		if (y2 < y1)
		{
			std::swap(y1, y2);
			std::swap(x1, x2);
			std::swap(u1, u2);
			std::swap(v1, v2);
		}

		if (y3 < y1)
		{
			std::swap(y1, y3);
			std::swap(x1, x3);
			std::swap(u1, u3);
			std::swap(v1, v3);
		}

		if (y3 < y2)
		{
			std::swap(y2, y3);
			std::swap(x2, x3);
			std::swap(u2, u3);
			std::swap(v2, v3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;
		float dv1 = v2 - v1;
		float du1 = u2 - u1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;
		float dv2 = v3 - v1;
		float du2 = u3 - u1;

		float tex_u, tex_v;

		float dax_step = 0, dbx_step = 0,
			  du1_step = 0, dv1_step = 0,
			  du2_step = 0, dv2_step = 0;

		if (dy1)
			dax_step = dx1 / (float)abs(dy1);
		if (dy2)
			dbx_step = dx2 / (float)abs(dy2);

		if (dy1)
			du1_step = du1 / (float)abs(dy1);
		if (dy1)
			dv1_step = dv1 / (float)abs(dy1);

		if (dy2)
			du2_step = du2 / (float)abs(dy2);
		if (dy2)
			dv2_step = dv2 / (float)abs(dy2);

		if (dy1)
		{
			for (int i = y1; i <= y2; i++)
			{
				int ax = x1 + (float)(i - y1) * dax_step;
				int bx = x1 + (float)(i - y1) * dbx_step;

				float tex_su = u1 + (float)(i - y1) * du1_step;
				float tex_sv = v1 + (float)(i - y1) * dv1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_sv, tex_ev);
				}

				tex_u = tex_su;
				tex_v = tex_sv;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++)
				{
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;

					pge->Draw(j, i, spr->Sample(tex_u, tex_v));
					t += tstep;
				}
			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;
		dv1 = v3 - v2;
		du1 = u3 - u2;

		if (dy1)
			dax_step = dx1 / (float)abs(dy1);
		if (dy2)
			dbx_step = dx2 / (float)abs(dy2);

		du1_step = 0, dv1_step = 0;
		if (dy1)
			du1_step = du1 / (float)abs(dy1);
		if (dy1)
			dv1_step = dv1 / (float)abs(dy1);

		if (dy1)
		{
			for (int i = y2; i <= y3; i++)
			{
				int ax = x2 + (float)(i - y2) * dax_step;
				int bx = x1 + (float)(i - y1) * dbx_step;

				float tex_su = u2 + (float)(i - y2) * du1_step;
				float tex_sv = v2 + (float)(i - y2) * dv1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
				}

				tex_u = tex_su;
				tex_v = tex_sv;

				float tstep = 1.0f / ((float)(bx - ax));
				float t = 0.0f;

				for (int j = ax; j < bx; j++)
				{
					tex_u = (1.0f - t) * tex_su + t * tex_eu;
					tex_v = (1.0f - t) * tex_sv + t * tex_ev;

					pge->Draw(j, i, spr->Sample(tex_u, tex_v));

					t += tstep;
				}
			}
		}
	}
	//*/
	else
	{
		if (y2 < y1)
		{
			std::swap(y1, y2);
			std::swap(x1, x2);
			std::swap(u1, u2);
			std::swap(v1, v2);
			std::swap(w1, w2);
		}

		if (y3 < y1)
		{
			std::swap(y1, y3);
			std::swap(x1, x3);
			std::swap(u1, u3);
			std::swap(v1, v3);
			std::swap(w1, w3);
		}

		if (y3 < y2)
		{
			std::swap(y2, y3);
			std::swap(x2, x3);
			std::swap(u2, u3);
			std::swap(v2, v3);
			std::swap(w2, w3);
		}

		int dy1 = y2 - y1;
		int dx1 = x2 - x1;
		float dv1 = v2 - v1;
		float du1 = u2 - u1;
		float dw1 = w2 - w1;

		int dy2 = y3 - y1;
		int dx2 = x3 - x1;
		float dv2 = v3 - v1;
		float du2 = u3 - u1;
		float dw2 = w3 - w1;

		float tex_u, tex_v, tex_w;

		float dax_step = 0, dbx_step = 0,
			  du1_step = 0, dv1_step = 0,
			  du2_step = 0, dv2_step = 0,
			  dw1_step = 0, dw2_step = 0;

		if (dy1)
			dax_step = dx1 / (float)abs(dy1);
		if (dy2)
			dbx_step = dx2 / (float)abs(dy2);

		if (dy1)
			du1_step = du1 / (float)abs(dy1);
		if (dy1)
			dv1_step = dv1 / (float)abs(dy1);
		if (dy1)
			dw1_step = dw1 / (float)abs(dy1);

		if (dy2)
			du2_step = du2 / (float)abs(dy2);
		if (dy2)
			dv2_step = dv2 / (float)abs(dy2);
		if (dy2)
			dw2_step = dw2 / (float)abs(dy2);

		if (dy1)
		{
			for (int i = y1; i <= y2; i++)
			{
				int ax = x1 + (float)(i - y1) * dax_step;
				int bx = x1 + (float)(i - y1) * dbx_step;

				float tex_su = u1 + (float)(i - y1) * du1_step;
				float tex_sv = v1 + (float)(i - y1) * dv1_step;
				float tex_sw = w1 + (float)(i - y1) * dw1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;
				float tex_ew = w1 + (float)(i - y1) * dw2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
					std::swap(tex_sw, tex_ew);
				}

				tex_u = tex_su;
				tex_v = tex_sv;
				tex_w = tex_sw;

				float tstep = 1.0 / ((float)(bx - ax));
				float t = 0.0;

				for (int j = ax; j < bx; j++)
				{
					tex_u = (1.0 - t) * tex_su + t * tex_eu;
					tex_v = (1.0 - t) * tex_sv + t * tex_ev;
					tex_w = (1.0 - t) * tex_sw + t * tex_ew;
					if (tex_w > m_DepthBuffer[i * pge->ScreenWidth() + j])
					{
						pge->Draw(j, i, spr->Sample(tex_u / tex_w, tex_v / tex_w));
						m_DepthBuffer[i * pge->ScreenWidth() + j] = tex_w;
					}
					t += tstep;
				}
			}
		}

		dy1 = y3 - y2;
		dx1 = x3 - x2;
		dv1 = v3 - v2;
		du1 = u3 - u2;
		dw1 = w3 - w2;

		if (dy1)
			dax_step = dx1 / (float)abs(dy1);
		if (dy2)
			dbx_step = dx2 / (float)abs(dy2);

		du1_step = 0, dv1_step = 0;
		if (dy1)
			du1_step = du1 / (float)abs(dy1);
		if (dy1)
			dv1_step = dv1 / (float)abs(dy1);
		if (dy1)
			dw1_step = dw1 / (float)abs(dy1);

		if (dy1)
		{
			for (int i = y2; i <= y3; i++)
			{
				int ax = x2 + (float)(i - y2) * dax_step;
				int bx = x1 + (float)(i - y1) * dbx_step;

				float tex_su = u2 + (float)(i - y2) * du1_step;
				float tex_sv = v2 + (float)(i - y2) * dv1_step;
				float tex_sw = w2 + (float)(i - y2) * dw1_step;

				float tex_eu = u1 + (float)(i - y1) * du2_step;
				float tex_ev = v1 + (float)(i - y1) * dv2_step;
				float tex_ew = w1 + (float)(i - y1) * dw2_step;

				if (ax > bx)
				{
					std::swap(ax, bx);
					std::swap(tex_su, tex_eu);
					std::swap(tex_sv, tex_ev);
					std::swap(tex_sw, tex_ew);
				}

				tex_u = tex_su;
				tex_v = tex_sv;
				tex_w = tex_sw;

				float tstep = 1.0 / ((float)(bx - ax));
				float t = 0.0;

				for (int j = ax; j < bx; j++)
				{
					tex_u = (1.0 - t) * tex_su + t * tex_eu;
					tex_v = (1.0 - t) * tex_sv + t * tex_ev;
					tex_w = (1.0 - t) * tex_sw + t * tex_ew;

					if (tex_w > m_DepthBuffer[i * pge->ScreenWidth() + j])
					{
						pge->Draw(j, i, spr->Sample(tex_u / tex_w, tex_v / tex_w));
						m_DepthBuffer[i * pge->ScreenWidth() + j] = tex_w;
					}
					t += tstep;
				}
			}
		}
	}
} //namespace JenovaSpace

void GFX3D::DrawTriangleTex(JenovaSpace::GFX3D::Triangle &tri, JenovaSpace::Sprite *spr)
{
	if (tri.p[1].y < tri.p[0].y)
	{
		std::swap(tri.p[0].y, tri.p[1].y);
		std::swap(tri.p[0].x, tri.p[1].x);
		std::swap(tri.t[0].x, tri.t[1].x);
		std::swap(tri.t[0].y, tri.t[1].y);
		std::swap(tri.t[0].z, tri.t[1].z);
	}

	if (tri.p[2].y < tri.p[0].y)
	{
		std::swap(tri.p[0].y, tri.p[2].y);
		std::swap(tri.p[0].x, tri.p[2].x);
		std::swap(tri.t[0].x, tri.t[2].x);
		std::swap(tri.t[0].y, tri.t[2].y);
		std::swap(tri.t[0].z, tri.t[2].z);
	}

	if (tri.p[2].y < tri.p[1].y)
	{
		std::swap(tri.p[1].y, tri.p[2].y);
		std::swap(tri.p[1].x, tri.p[2].x);
		std::swap(tri.t[1].x, tri.t[2].x);
		std::swap(tri.t[1].y, tri.t[2].y);
		std::swap(tri.t[1].z, tri.t[2].z);
	}

	int dy1 = tri.p[1].y - tri.p[0].y;
	int dx1 = tri.p[1].x - tri.p[0].x;
	float dv1 = tri.t[1].y - tri.t[0].y;
	float du1 = tri.t[1].x - tri.t[0].x;
	float dz1 = tri.t[1].z - tri.t[0].z;

	int dy2 = tri.p[2].y - tri.p[0].y;
	int dx2 = tri.p[2].x - tri.p[0].x;
	float dv2 = tri.t[2].y - tri.t[0].y;
	float du2 = tri.t[2].x - tri.t[0].x;
	float dz2 = tri.t[2].z - tri.t[0].z;

	float tex_x, tex_y, tex_z;

	float du1_step = 0, dv1_step = 0, du2_step = 0, dv2_step = 0, dz1_step = 0, dz2_step = 0;
	float dax_step = 0, dbx_step = 0;

	if (dy1)
		dax_step = dx1 / (float)abs(dy1);
	if (dy2)
		dbx_step = dx2 / (float)abs(dy2);

	if (dy1)
		du1_step = du1 / (float)abs(dy1);
	if (dy1)
		dv1_step = dv1 / (float)abs(dy1);
	if (dy1)
		dz1_step = dz1 / (float)abs(dy1);

	if (dy2)
		du2_step = du2 / (float)abs(dy2);
	if (dy2)
		dv2_step = dv2 / (float)abs(dy2);
	if (dy2)
		dz2_step = dz2 / (float)abs(dy2);

	if (dy1)
	{
		for (int i = tri.p[0].y; i <= tri.p[1].y; i++)
		{
			int ax = tri.p[0].x + (i - tri.p[0].y) * dax_step;
			int bx = tri.p[0].x + (i - tri.p[0].y) * dbx_step;

			//Start and end points in texture space
			float tex_su = tri.t[0].x + (float)(i - tri.p[0].y) * du1_step;
			float tex_sv = tri.t[0].y + (float)(i - tri.p[0].y) * dv1_step;
			float tex_sz = tri.t[0].z + (float)(i - tri.p[0].y) * dz1_step;

			float tex_eu = tri.t[0].x + (float)(i - tri.p[0].y) * du2_step;
			float tex_ev = tri.t[0].y + (float)(i - tri.p[0].y) * dv2_step;
			float tex_ez = tri.t[0].z + (float)(i - tri.p[0].y) * dz2_step;

			if (ax > bx)
			{
				std::swap(ax, bx);
				std::swap(tex_su, tex_eu);
				std::swap(tex_sv, tex_ev);
				std::swap(tex_sz, tex_ez);
			}

			tex_x = tex_su;
			tex_y = tex_sv;
			tex_z = tex_sz;

			float tstep = 1.0 / ((float)(bx - ax));
			float t = 0;

			for (int j = ax; j < bx; j++)
			{
				tex_x = (1.0 - t) * tex_su + t * tex_eu;
				tex_y = (1.0 - t) * tex_sv + t * tex_ev;
				tex_z = (1.0 - t) * tex_sz + t * tex_ez;

				if (tex_z > m_DepthBuffer[i * pge->ScreenWidth() + j])
				{
					pge->Draw(j, i, spr->Sample(tex_x / tex_z, tex_y / tex_z));
					m_DepthBuffer[i * pge->ScreenWidth() + j] = tex_z;
				}
				t += tstep;
			}
		}
	}

	dy1 = tri.p[2].y - tri.p[1].y;
	dx1 = tri.p[2].x - tri.p[1].x;
	dv1 = tri.t[2].y - tri.t[1].y;
	du1 = tri.t[2].x - tri.t[1].x;
	dz1 = tri.t[2].z - tri.t[1].z;

	if (dy1)
		dax_step = dx1 / (float)abs(dy1);
	if (dy2)
		dbx_step = dx2 / (float)abs(dy2);

	du1_step = 0, dv1_step = 0; //, dz1_step = 0;//, du2_step = 0, dv2_step = 0;
	if (dy1)
		du1_step = du1 / (float)abs(dy1);
	if (dy1)
		dv1_step = dv1 / (float)abs(dy1);
	if (dy1)
		dz1_step = dz1 / (float)abs(dy1);

	if (dy1)
	{
		for (int i = tri.p[1].y; i <= tri.p[2].y; i++)
		{
			int ax = tri.p[1].x + (i - tri.p[1].y) * dax_step;
			int bx = tri.p[0].x + (i - tri.p[0].y) * dbx_step;

			//Start and end points in texture space
			float tex_su = tri.t[1].x + (float)(i - tri.p[1].y) * du1_step;
			float tex_sv = tri.t[1].y + (float)(i - tri.p[1].y) * dv1_step;
			float tex_sz = tri.t[1].z + (float)(i - tri.p[1].y) * dz1_step;

			float tex_eu = tri.t[0].x + (float)(i - tri.p[0].y) * du2_step;
			float tex_ev = tri.t[0].y + (float)(i - tri.p[0].y) * dv2_step;
			float tex_ez = tri.t[0].z + (float)(i - tri.p[0].y) * dz2_step;

			if (ax > bx)
			{
				std::swap(ax, bx);
				std::swap(tex_su, tex_eu);
				std::swap(tex_sv, tex_ev);
				std::swap(tex_sz, tex_ez);
			}

			tex_x = tex_su;
			tex_y = tex_sv;
			tex_z = tex_sz;

			float tstep = 1.0 / ((float)(bx - ax));
			float t = 0;

			for (int j = ax; j < bx; j++)
			{
				tex_x = (1.0 - t) * tex_su + t * tex_eu;
				tex_y = (1.0 - t) * tex_sv + t * tex_ev;
				tex_z = (1.0 - t) * tex_sz + t * tex_ez;

				if (tex_z > m_DepthBuffer[i * pge->ScreenWidth() + j])
				{
					pge->Draw(j, i, spr->Sample(tex_x / tex_z, tex_y / tex_z));
					m_DepthBuffer[i * pge->ScreenWidth() + j] = tex_z;
				}

				t += tstep;
			}
		}
	}
}

float *GFX3D::m_DepthBuffer = nullptr;

void GFX3D::ConfigureDisplay()
{
	m_DepthBuffer = new float[pge->ScreenWidth() * pge->ScreenHeight()]{0};
}

void GFX3D::ClearDepth()
{
	memset(m_DepthBuffer, 0, pge->ScreenWidth() * pge->ScreenHeight() * sizeof(float));
}

GFX3D::PipeLine::PipeLine()
{
}

void GFX3D::PipeLine::SetProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar, float fLeft, float fTop, float fWidth, float fHeight)
{
	matProj = GFX3D::Math::Mat_MakeProjection(fFovDegrees, fAspectRatio, fNear, fFar);
	fViewX = fLeft;
	fViewY = fTop;
	fViewW = fWidth;
	fViewH = fHeight;
}
void GFX3D::PipeLine::SetTextureMappingMode(bool affineMode)
{
	affine = affineMode;
}

void GFX3D::PipeLine::SetCamera(JenovaSpace::GFX3D::Vector3D &pos, JenovaSpace::GFX3D::Vector3D &lookat, JenovaSpace::GFX3D::Vector3D &up)
{
	matView = GFX3D::Math::Mat_PointAt(pos, lookat, up);
	matView = GFX3D::Math::Mat_QuickInverse(matView);
}

void GFX3D::PipeLine::SetTransform(JenovaSpace::GFX3D::Matrix4x4 &transform)
{
	matWorld = transform;
}

void GFX3D::PipeLine::SetTexture(JenovaSpace::Sprite *texture)
{
	sprTexture = texture;
}

void GFX3D::PipeLine::SetLightSource(JenovaSpace::GFX3D::Vector3D &pos, JenovaSpace::GFX3D::Vector3D &dir, JenovaSpace::Pixel &col)
{
}

uint32_t GFX3D::PipeLine::Render(std::vector<JenovaSpace::GFX3D::Triangle> &Triangles, uint32_t flags)
{
	///*
	if (affine)
	{
		//calculate Transformation Matrix
		Matrix4x4 matWorldView = Math::Mat_MultiplyMatrix(matWorld, matView);
		//matWorldViewProj = Math::Mat_MultiplyMatrix(matWorldView, matProj);

		//store Triangles for rastering later
		std::vector<GFX3D::Triangle> vecTrianglesToRaster;

		int nTriangleDrawnCount = 0;

		//process Triangles
		for (auto &tri : Triangles)
		{
			GFX3D::Triangle triTransformed;

			//just copy through texture coordinates
			triTransformed.t[0] = {tri.t[0].x, tri.t[0].y};
			triTransformed.t[1] = {tri.t[1].x, tri.t[1].y};
			triTransformed.t[2] = {tri.t[2].x, tri.t[2].y}; //Think!

			//transform Triangle from object into projected space
			triTransformed.p[0] = GFX3D::Math::Mat_MultiplyVector(matWorldView, tri.p[0]);
			triTransformed.p[1] = GFX3D::Math::Mat_MultiplyVector(matWorldView, tri.p[1]);
			triTransformed.p[2] = GFX3D::Math::Mat_MultiplyVector(matWorldView, tri.p[2]);

			//calculate Triangle Normal in WorldView Space
			GFX3D::Vector3D normal, line1, line2;
			line1 = GFX3D::Math::Vec_Sub(triTransformed.p[1], triTransformed.p[0]);
			line2 = GFX3D::Math::Vec_Sub(triTransformed.p[2], triTransformed.p[0]);
			normal = GFX3D::Math::Vec_CrossProduct(line1, line2);
			normal = GFX3D::Math::Vec_Normalise(normal);

			//cull Triangles that face away from viewer
			if (flags & RENDER_CULL_CW && GFX3D::Math::Vec_DotProduct(normal, triTransformed.p[0]) > 0.0f)
				continue;
			if (flags & RENDER_CULL_CCW && GFX3D::Math::Vec_DotProduct(normal, triTransformed.p[0]) < 0.0f)
				continue;

			//if Lighting, calculate shading
			triTransformed.col = JenovaSpace::WHITE;

			//clip Triangle against near plane
			int nClippedTriangles = 0;
			Triangle clipped[2];
			nClippedTriangles = GFX3D::Math::Triangle_ClipAgainstPlane({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f}, triTransformed, clipped[0], clipped[1]);

			//this may yield two new Triangles
			for (int n = 0; n < nClippedTriangles; n++)
			{
				Triangle triProjected = clipped[n];

				//project new Triangle
				triProjected.p[0] = GFX3D::Math::Mat_MultiplyVector(matProj, clipped[n].p[0]);
				triProjected.p[1] = GFX3D::Math::Mat_MultiplyVector(matProj, clipped[n].p[1]);
				triProjected.p[2] = GFX3D::Math::Mat_MultiplyVector(matProj, clipped[n].p[2]);

				//apply Projection to Verts
				triProjected.p[0].x = triProjected.p[0].x / triProjected.p[0].w;
				triProjected.p[1].x = triProjected.p[1].x / triProjected.p[1].w;
				triProjected.p[2].x = triProjected.p[2].x / triProjected.p[2].w;

				triProjected.p[0].y = triProjected.p[0].y / triProjected.p[0].w;
				triProjected.p[1].y = triProjected.p[1].y / triProjected.p[1].w;
				triProjected.p[2].y = triProjected.p[2].y / triProjected.p[2].w;

				triProjected.p[0].z = triProjected.p[0].z / triProjected.p[0].w;
				triProjected.p[1].z = triProjected.p[1].z / triProjected.p[1].w;
				triProjected.p[2].z = triProjected.p[2].z / triProjected.p[2].w;

				//clip against viewport in screen space
				//clip Triangles against all four screen edges, this could yield a bunch of Triangles, so create a queue that we traverse to ensure we only test new Triangles generated against planes
				Triangle sclipped[2];
				std::list<Triangle> listTriangles;

				//add initial Triangle
				listTriangles.push_back(triProjected);
				int nNewTriangles = 1;

				for (int p = 0; p < 4; p++)
				{
					int nTrisToAdd = 0;
					while (nNewTriangles > 0)
					{
						//take Triangle from front of queue
						Triangle test = listTriangles.front();
						listTriangles.pop_front();
						nNewTriangles--;

						//clip it against a plane. We only need to test each subsequent plane, against subsequent new Triangles as all Triangles after a plane clip are guaranteed to lie on the inside of the plane. I like how this comment is almost completely and utterly justified
						switch (p)
						{
						case 0:
							nTrisToAdd = GFX3D::Math::Triangle_ClipAgainstPlane({0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, test, sclipped[0], sclipped[1]);
							break;
						case 1:
							nTrisToAdd = GFX3D::Math::Triangle_ClipAgainstPlane({0.0f, +1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, test, sclipped[0], sclipped[1]);
							break;
						case 2:
							nTrisToAdd = GFX3D::Math::Triangle_ClipAgainstPlane({-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, test, sclipped[0], sclipped[1]);
							break;
						case 3:
							nTrisToAdd = GFX3D::Math::Triangle_ClipAgainstPlane({+1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, test, sclipped[0], sclipped[1]);
							break;
						}

						//clipping may yield a variable number of Triangles, so add these new ones to the back of the queue for subsequent clipping against next planes
						for (int w = 0; w < nTrisToAdd; w++)
							listTriangles.push_back(sclipped[w]);
					}
					nNewTriangles = listTriangles.size();
				}

				for (auto &triRaster : listTriangles)
				{
					//scale to viewport
					/*
							triRaster.p[0].x *= -1.0f;
							triRaster.p[1].x *= -1.0f;
							triRaster.p[2].x *= -1.0f;
							triRaster.p[0].y *= -1.0f;
							triRaster.p[1].y *= -1.0f;
							triRaster.p[2].y *= -1.0f;
					*/
					Vector3D vOffsetView = {1, 1, 0};
					triRaster.p[0] = Math::Vec_Add(triRaster.p[0], vOffsetView);
					triRaster.p[1] = Math::Vec_Add(triRaster.p[1], vOffsetView);
					triRaster.p[2] = Math::Vec_Add(triRaster.p[2], vOffsetView);
					triRaster.p[0].x *= 0.5f * fViewW;
					triRaster.p[0].y *= 0.5f * fViewH;
					triRaster.p[1].x *= 0.5f * fViewW;
					triRaster.p[1].y *= 0.5f * fViewH;
					triRaster.p[2].x *= 0.5f * fViewW;
					triRaster.p[2].y *= 0.5f * fViewH;
					vOffsetView = {fViewX, fViewY, 0};
					triRaster.p[0] = Math::Vec_Add(triRaster.p[0], vOffsetView);
					triRaster.p[1] = Math::Vec_Add(triRaster.p[1], vOffsetView);
					triRaster.p[2] = Math::Vec_Add(triRaster.p[2], vOffsetView);

					//for now, just draw Triangle

					if (flags & RENDER_TEXTURED)
					{
						TexturedTriangle(
							triRaster.p[0].x, triRaster.p[0].y, triRaster.t[0].x, triRaster.t[0].y, triRaster.t[0].z,
							triRaster.p[1].x, triRaster.p[1].y, triRaster.t[1].x, triRaster.t[1].y, triRaster.t[1].z,
							triRaster.p[2].x, triRaster.p[2].y, triRaster.t[2].x, triRaster.t[2].y, triRaster.t[2].z,
							sprTexture, affine);
					}

					if (flags & RENDER_WIRE)
					{
						DrawTriangleWire(triRaster, JenovaSpace::RED);
					}

					if (flags & RENDER_FLAT)
					{
						DrawTriangleFlat(triRaster);
					}

					nTriangleDrawnCount++;
				}
			}
		}
		return nTriangleDrawnCount;
	}
	//*/

	//Calculate Transformation Matrix
	Matrix4x4 matWorldView = Math::Mat_MultiplyMatrix(matWorld, matView);
	//matWorldViewProj = Math::Mat_MultiplyMatrix(matWorldView, matProj);

	//Store Triangles for rastering later
	std::vector<GFX3D::Triangle> vecTrianglesToRaster;

	int nTriangleDrawnCount = 0;

	//Process Triangles
	for (auto &tri : Triangles)
	{
		GFX3D::Triangle triTransformed;

		//Just copy through texture coordinates
		triTransformed.t[0] = {tri.t[0].x, tri.t[0].y, tri.t[0].z};
		triTransformed.t[1] = {tri.t[1].x, tri.t[1].y, tri.t[1].z};
		triTransformed.t[2] = {tri.t[2].x, tri.t[2].y, tri.t[2].z}; //良く考えなさい

		//transform Triangle from object into projected space
		triTransformed.p[0] = GFX3D::Math::Mat_MultiplyVector(matWorldView, tri.p[0]);
		triTransformed.p[1] = GFX3D::Math::Mat_MultiplyVector(matWorldView, tri.p[1]);
		triTransformed.p[2] = GFX3D::Math::Mat_MultiplyVector(matWorldView, tri.p[2]);

		//calculate Triangle Normal in WorldView Space
		GFX3D::Vector3D normal, line1, line2;
		line1 = GFX3D::Math::Vec_Sub(triTransformed.p[1], triTransformed.p[0]);
		line2 = GFX3D::Math::Vec_Sub(triTransformed.p[2], triTransformed.p[0]);
		normal = GFX3D::Math::Vec_CrossProduct(line1, line2);
		normal = GFX3D::Math::Vec_Normalise(normal);

		//cull Triangles that face away from viewer
		if (flags & RENDER_CULL_CW && GFX3D::Math::Vec_DotProduct(normal, triTransformed.p[0]) > 0.0f)
			continue;
		if (flags & RENDER_CULL_CCW && GFX3D::Math::Vec_DotProduct(normal, triTransformed.p[0]) < 0.0f)
			continue;

		//if Lighting, calculate shading
		triTransformed.col = JenovaSpace::WHITE;

		//clip Triangle against near plane
		int nClippedTriangles = 0;
		Triangle clipped[2];
		nClippedTriangles = GFX3D::Math::Triangle_ClipAgainstPlane({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f}, triTransformed, clipped[0], clipped[1]);

		//this may yield two new Triangles
		for (int n = 0; n < nClippedTriangles; n++)
		{
			Triangle triProjected = clipped[n];

			//project new Triangle
			triProjected.p[0] = GFX3D::Math::Mat_MultiplyVector(matProj, clipped[n].p[0]);
			triProjected.p[1] = GFX3D::Math::Mat_MultiplyVector(matProj, clipped[n].p[1]);
			triProjected.p[2] = GFX3D::Math::Mat_MultiplyVector(matProj, clipped[n].p[2]);

			//apply Projection to Verts
			triProjected.p[0].x = triProjected.p[0].x / triProjected.p[0].w;
			triProjected.p[1].x = triProjected.p[1].x / triProjected.p[1].w;
			triProjected.p[2].x = triProjected.p[2].x / triProjected.p[2].w;

			triProjected.p[0].y = triProjected.p[0].y / triProjected.p[0].w;
			triProjected.p[1].y = triProjected.p[1].y / triProjected.p[1].w;
			triProjected.p[2].y = triProjected.p[2].y / triProjected.p[2].w;

			triProjected.p[0].z = triProjected.p[0].z / triProjected.p[0].w;
			triProjected.p[1].z = triProjected.p[1].z / triProjected.p[1].w;
			triProjected.p[2].z = triProjected.p[2].z / triProjected.p[2].w;

			//apply Projection to Tex coords
			triProjected.t[0].x = triProjected.t[0].x / triProjected.p[0].w;
			triProjected.t[1].x = triProjected.t[1].x / triProjected.p[1].w;
			triProjected.t[2].x = triProjected.t[2].x / triProjected.p[2].w;

			triProjected.t[0].y = triProjected.t[0].y / triProjected.p[0].w;
			triProjected.t[1].y = triProjected.t[1].y / triProjected.p[1].w;
			triProjected.t[2].y = triProjected.t[2].y / triProjected.p[2].w;

			triProjected.t[0].z = 1.0f / triProjected.p[0].w;
			triProjected.t[1].z = 1.0f / triProjected.p[1].w;
			triProjected.t[2].z = 1.0f / triProjected.p[2].w;

			//clip against viewport in screen space
			//clip Triangles against all four screen edges, this could yield a bunch of Triangles, so create a queue that we traverse to ensure we only test new Triangles generated against planes
			Triangle sclipped[2];
			std::list<Triangle> listTriangles;

			//Add initial Triangle
			listTriangles.push_back(triProjected);
			int nNewTriangles = 1;

			for (int p = 0; p < 4; p++)
			{
				int nTrisToAdd = 0;
				while (nNewTriangles > 0)
				{
					//Take Triangle from front of queue
					Triangle test = listTriangles.front();
					listTriangles.pop_front();
					nNewTriangles--;

					//Clip it against a plane. We only need to test each
					//subsequent plane, against subsequent new Triangles
					//as all Triangles after a plane clip are guaranteed
					//to lie on the inside of the plane. I like how this
					//comment is almost completely and utterly justified
					switch (p)
					{
					case 0:
						nTrisToAdd = GFX3D::Math::Triangle_ClipAgainstPlane({0.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, test, sclipped[0], sclipped[1]);
						break;
					case 1:
						nTrisToAdd = GFX3D::Math::Triangle_ClipAgainstPlane({0.0f, +1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, test, sclipped[0], sclipped[1]);
						break;
					case 2:
						nTrisToAdd = GFX3D::Math::Triangle_ClipAgainstPlane({-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, test, sclipped[0], sclipped[1]);
						break;
					case 3:
						nTrisToAdd = GFX3D::Math::Triangle_ClipAgainstPlane({+1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, test, sclipped[0], sclipped[1]);
						break;
					}

					//Clipping may yield a variable number of Triangles, so
					//add these new ones to the back of the queue for subsequent
					//clipping against next planes
					for (int w = 0; w < nTrisToAdd; w++)
						listTriangles.push_back(sclipped[w]);
				}
				nNewTriangles = listTriangles.size();
			}

			for (auto &triRaster : listTriangles)
			{
				//Scale to viewport
				/*triRaster.p[0].x *= -1.0f;
					triRaster.p[1].x *= -1.0f;
					triRaster.p[2].x *= -1.0f;
					triRaster.p[0].y *= -1.0f;
					triRaster.p[1].y *= -1.0f;
					triRaster.p[2].y *= -1.0f;*/
				Vector3D vOffsetView = {1, 1, 0};
				triRaster.p[0] = Math::Vec_Add(triRaster.p[0], vOffsetView);
				triRaster.p[1] = Math::Vec_Add(triRaster.p[1], vOffsetView);
				triRaster.p[2] = Math::Vec_Add(triRaster.p[2], vOffsetView);
				triRaster.p[0].x *= 0.5f * fViewW;
				triRaster.p[0].y *= 0.5f * fViewH;
				triRaster.p[1].x *= 0.5f * fViewW;
				triRaster.p[1].y *= 0.5f * fViewH;
				triRaster.p[2].x *= 0.5f * fViewW;
				triRaster.p[2].y *= 0.5f * fViewH;
				vOffsetView = {fViewX, fViewY, 0};
				triRaster.p[0] = Math::Vec_Add(triRaster.p[0], vOffsetView);
				triRaster.p[1] = Math::Vec_Add(triRaster.p[1], vOffsetView);
				triRaster.p[2] = Math::Vec_Add(triRaster.p[2], vOffsetView);

				//For now, just draw Triangle

				if (flags & RENDER_TEXTURED)
				{
					TexturedTriangle(
						triRaster.p[0].x, triRaster.p[0].y, triRaster.t[0].x, triRaster.t[0].y, triRaster.t[0].z,
						triRaster.p[1].x, triRaster.p[1].y, triRaster.t[1].x, triRaster.t[1].y, triRaster.t[1].z,
						triRaster.p[2].x, triRaster.p[2].y, triRaster.t[2].x, triRaster.t[2].y, triRaster.t[2].z,
						sprTexture, affine);
				}

				if (flags & RENDER_WIRE)
				{
					DrawTriangleWire(triRaster, JenovaSpace::RED);
				}

				if (flags & RENDER_FLAT)
				{
					DrawTriangleFlat(triRaster);
				}

				nTriangleDrawnCount++;
			}
		}
	}

	return nTriangleDrawnCount;
}
} //namespace JenovaSpace

#endif