#define OLC_PGE_APPLICATION
#include "Jenova.h"
#define JENOVA3D
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
	JenovaSpace::GFX3D::Mesh mesh;
	JenovaSpace::GFX3D::PipeLine pipeline;

public:
	bool OnUserCreate() override
	{
		//called once at the start, so create things here
		mesh.LoadObjectFromFile("tifa.obj");
		return true;
	}

	bool OnUserUpdate(double fElapsedTime) override
	{
		pipeline.Render(mesh.triangles, true, JenovaSpace::GFX3D::RENDER_FLAT);
		return true;
	}
};
int main()
{
	Example demo;
	if (demo.Construct(768, 480, 2, 2))
		demo.Start();
	return 0;
}