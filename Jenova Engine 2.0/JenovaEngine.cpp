#define OLC_PGE_APPLICATION
#define OLC_PGEX_GFX3D
#include "Jenova.h"
#include "Jenova3D.h"
#include <strstream>
#include <fstream>
// Override base class with your custom functionality
class Example : public JenovaSpace::JenovaPixel
{
public:
	Example()
	{
		sAppName = "Example";
	}
public:
	bool OnUserCreate() override
	{
		//called once at the start, so create things here
		return true;
	}

	bool OnUserUpdate(double fElapsedTime) override
	{
		JenovaSpace::GFX3D::Mesh mesh;
		mesh.LoadObjectFromFile("tifa.obj");
		JenovaSpace::GFX3D::PipeLine pipeline;
		pipeline.Render(mesh.tris);
		return true;
	}
	int GetLight(double lum) 
	{
		return (int)(255.0 * lum);
	}
};
int main()
{
	Example demo;
	if (demo.Construct(256, 240, 4, 4))
		demo.Start();
	return 0;
}