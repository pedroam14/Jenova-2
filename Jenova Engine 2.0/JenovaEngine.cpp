#define OLC_PGE_APPLICATION
#include "Jenova.h"
#define JENOVA3D
#include "Jenova3D.h"
#include <strstream>
#include <fstream>
//Override base class with your custom functionality
class JenovaEngine : public JenovaSpace::JenovaPixel
{
public:
	JenovaEngine()
	{
		sAppName = "Map Editor";
	}

private:
	JenovaSpace::GFX3D::Mesh mesh;
	JenovaSpace::GFX3D::PipeLine renderingPipeline;
	JenovaSpace::Sprite *sprite;

	JenovaSpace::GFX3D::Vector3D vectorUp = { 0, 1, 0 };
	JenovaSpace::GFX3D::Vector3D vectorEye = { 0, 0, -10.0 };
	JenovaSpace::GFX3D::Vector3D vectorLookDirection = { 0, 0, 1 };

	double theta;

public:
	bool OnUserCreate() override
	{
		//initialization stuf, setting up the display and the projection in world space
		JenovaSpace::GFX3D::ConfigureDisplay();
		renderingPipeline.SetProjection(90.0, (double)ScreenHeight() / (double)ScreenWidth(), 0.1, 1000.0, 0.0, 0.0, ScreenWidth(), ScreenHeight());

		sprite = new JenovaSpace::Sprite("Resources/tifa.png");
		//cube, meant for debugging only desu
		///*
		mesh.triangles =
		{
			// SOUTH
			{ 0.0f, 0.0f, 0.0f, 1.0f,	    0.0f, 1.0f, 0.0f, 1.0f,		 1.0f, 1.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f, 		1.0f, 0.0f, 0.0f, },
			{ 0.0f, 0.0f, 0.0f, 1.0f,  		1.0f, 1.0f, 0.0f, 1.0f,		 1.0f, 0.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 		1.0f, 1.0f, 0.0f, },

			// EAST
			{ 1.0f, 0.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f, 		1.0f, 0.0f, 0.0f, },
			{ 1.0f, 0.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 		1.0f, 1.0f, 0.0f, },

			// NORTH
			{ 1.0f, 0.0f, 1.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f, 		1.0f, 0.0f, 0.0f, },
			{ 1.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 		1.0f, 1.0f, 0.0f, },

			// WEST
			{ 0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f, 		1.0f, 0.0f, 0.0f, },
			{ 0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 		1.0f, 1.0f, 0.0f, },

			// TOP             																 				    
			{ 0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f, 		1.0f, 0.0f, 0.0f, },
			{ 0.0f, 1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 		1.0f, 1.0f, 0.0f, },

			// BOTTOM         																 				   
			{ 1.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,	0.0f, 0.0f, 0.0f, 		1.0f, 0.0f, 0.0f, },
			{ 1.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f,	1.0f, 0.0f, 0.0f, 		1.0f, 1.0f, 0.0f, },

		};
		//*/
		return true;
	}

	bool OnUserUpdate(double elapsedTime) override
	{

		Clear(JenovaSpace::BLACK);
		JenovaSpace::GFX3D::ClearDepth();

		theta += elapsedTime;
		JenovaSpace::GFX3D::Vector3D vectorLookTarget = JenovaSpace::GFX3D::Math::Vec_Add(vectorEye, vectorLookDirection);

		//setupd the camera properties for the pipeline, aka the view transform
		renderingPipeline.SetCamera(vectorEye, vectorLookTarget, vectorUp);

		JenovaSpace::GFX3D::Matrix4x4 rotationMatrixX = JenovaSpace::GFX3D::Math::Mat_MakeRotationX(theta);
		JenovaSpace::GFX3D::Matrix4x4 rotationMatrixZ = JenovaSpace::GFX3D::Math::Mat_MakeRotationZ(theta / 3.0);
		JenovaSpace::GFX3D::Matrix4x4 worldMatrix = JenovaSpace::GFX3D::Math::Mat_MultiplyMatrix(rotationMatrixX, rotationMatrixZ);

		renderingPipeline.SetTransform(worldMatrix);

		renderingPipeline.SetTexture(sprite);

		renderingPipeline.Render(mesh.triangles, true);

		return true;
	}
};
int main()
{
	JenovaEngine demo;
	if (demo.Construct(768, 480, 2, 2))
	{
		demo.Start();
	}
	return 0;
}