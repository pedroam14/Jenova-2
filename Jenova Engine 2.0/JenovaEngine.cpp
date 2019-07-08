#define JENOVA3D
#define JENOVAAPP
#include "Jenova.h"
#include "Jenova3D.h"

#include <vector>
#include <list>
#include <algorithm>
#include <utility>
#include <string>
#include <unordered_set>
#include <fstream>

#define NORTH 0
#define SOUTH 1
#define EAST 2
#define WEST 3

#pragma region "Map Editor"

//override base class with your custom functionality
class MapEditor : public JenovaSpace::JenovaPixel
{

public:
	MapEditor()
	{
		sAppName = "Map Editor";
	}

private:
	//define the cell
	struct sCell
	{
		int nHeight = 0;
		int nWorldX = 0;
		int nWorldY = 0;
		bool bRoad = false;
		bool bBuilding = true;
	};

	//map variables
	int nMapWidth;
	int nMapHeight;
	sCell *pMap;

	JenovaSpace::Sprite *sprAll;
	JenovaSpace::Sprite *sprGround;
	JenovaSpace::Sprite *sprRoof;
	JenovaSpace::Sprite *sprFrontage;
	JenovaSpace::Sprite *sprWindows;
	JenovaSpace::Sprite *sprRoad[12];
	JenovaSpace::Sprite *sprChar[12];
	JenovaSpace::Sprite *sprCharSheet;

	float fCameraX = 0.0f;
	float fCameraY = 0.0f;
	float fCameraZ = -10.0f;

	JenovaSpace::GFX3D::Mesh meshCube;
	JenovaSpace::GFX3D::Mesh meshFlat;
	JenovaSpace::GFX3D::Mesh meshWallsOut;

	float fCharAngle = 0.0f;
	float fCharSpeed = 2.0f;
	JenovaSpace::GFX3D::Vector3D vecCharVel = {0, 0, 0};
	JenovaSpace::GFX3D::Vector3D vecCharPos = {0, 0, 0};

	int nMouseWorldX = 0;
	int nMouseWorldY = 0;
	int nOldMouseWorldX = 0;
	int nOldMouseWorldY = 0;

	bool bMouseDown = false;
	std::unordered_set<sCell *> setSelectedCells;

	JenovaSpace::GFX3D::PipeLine pipeRender;
	JenovaSpace::GFX3D::Matrix4x4 matProj;
	JenovaSpace::GFX3D::Vector3D vUp = {0, 1, 0};
	JenovaSpace::GFX3D::Vector3D vEye = {0, 0, -10};
	JenovaSpace::GFX3D::Vector3D vLookDir = {0, 0, 1};

	JenovaSpace::GFX3D::Vector3D viewWorldTopLeft, viewWorldBottomRight;

	void SaveCity(std::string sFilename)
	{
		std::ofstream file(sFilename, std::ios::out | std::ios::binary);
		file.write((char *)&nMapWidth, sizeof(int));
		file.write((char *)&nMapHeight, sizeof(int));
		for (int x = 0; x < nMapWidth; x++)
		{
			for (int y = 0; y < nMapHeight; y++)
			{
				file.write((char *)&pMap[y * nMapWidth + x], sizeof(sCell));
			}
		}
	}

	void LoadCity(std::string sFilename)
	{
		std::ifstream file(sFilename, std::ios::in | std::ios::binary);
		file.read((char *)&nMapWidth, sizeof(int));
		file.read((char *)&nMapHeight, sizeof(int));
		delete[] pMap;
		pMap = new sCell[nMapWidth * nMapHeight];
		for (int x = 0; x < nMapWidth; x++)
		{
			for (int y = 0; y < nMapHeight; y++)
			{
				file.read((char *)&pMap[y * nMapWidth + x], sizeof(sCell));
			}
		}
	}

public:
	bool OnUserCreate() override
	{
		//load Sprite Sheet
		sprAll = new JenovaSpace::Sprite("Resources/MapSpriteSheet.png");

		//break up the sprite sheet into individual textures

		//building Lowest Floor
		sprFrontage = new JenovaSpace::Sprite(96, 96);
		SetDrawTarget(sprFrontage);
		DrawPartialSprite(0, 0, sprAll, 288, 0, 96, 96);

		//building Windows
		sprWindows = new JenovaSpace::Sprite(96, 96);
		SetDrawTarget(sprWindows);
		DrawPartialSprite(0, 0, sprAll, 288, 0, 96, 96);

		//plain Grass Field
		sprGround = new JenovaSpace::Sprite(96, 96);
		SetDrawTarget(sprGround);
		DrawPartialSprite(0, 0, sprAll, 192, 0, 96, 96);

		//building Roof
		sprRoof = new JenovaSpace::Sprite(96, 96);
		SetDrawTarget(sprRoof);
		DrawPartialSprite(0, 0, sprAll, 288, 0, 96, 96);

		//there are 12 Road Textures, aranged in a 3x4 grid
		for (int r = 0; r < 12; r++)
		{
			sprRoad[r] = new JenovaSpace::Sprite(96, 96);
			SetDrawTarget(sprRoad[r]);
			DrawPartialSprite(0, 0, sprAll, (r % 3) * 96, (r / 3) * 96, 96, 96);
		}
		//player character
		sprCharSheet = new JenovaSpace::Sprite("Resources/TerraSpriteSheet.png");
		for (int r = 0; r < 12; r++)
		{
			sprChar[r] = new JenovaSpace::Sprite(15, 24);
			SetDrawTarget(sprChar[r]);
			DrawPartialSprite(0, 0, sprCharSheet, (r % 3) * 15 + 3, (r / 3) * 24 + 49, 15, 24);
		}

		SetDrawTarget(nullptr);
		//don't foregt to set the draw target back to being the main screen

		//define the city map, a 64x32 array of Cells　and Initialise cells to be just grass fields
		nMapWidth = 64;
		nMapHeight = 32;
		pMap = new sCell[nMapWidth * nMapHeight];
		for (int x = 0; x < nMapWidth; x++)
		{
			for (int y = 0; y < nMapHeight; y++)
			{
				pMap[y * nMapWidth + x].bRoad = false;
				pMap[y * nMapWidth + x].nHeight = 0;
				pMap[y * nMapWidth + x].nWorldX = x;
				pMap[y * nMapWidth + x].nWorldY = y;
			}
		}

		//*VERY* unoptimized, 後で修理してください
		//here the geometry is unit in size (1x1x1)

		//a Full cube - Always useful for debugging
		meshCube.tris =
			{
				//SOUTH
				{
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
				},
				{
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
				},

				//EAST
				{
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
				},
				{
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
				},

				//NORTH
				{
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
				},
				{
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
				},

				//WEST
				{
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
				},
				{
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
				},

				//TOP
				{
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
				},
				{
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
				},

				//BOTTOM
				{
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
				},
				{
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
				},

			};

		//A Flat quad
		meshFlat.tris =
			{
				{
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
				},
				{
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
				},
			};

		//The four outer walls of a cell
		meshWallsOut.tris =
			{
				//EAST
				{
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					0.2,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
				},
				{
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					0.2,
					1.0f,
					1.0f,
					0.0f,
					0.2,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
				},

				//WEST
				{
					0.0f,
					0.0f,
					0.2,
					1.0f,
					0.0f,
					1.0f,
					0.2,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
				},
				{
					0.0f,
					0.0f,
					0.2,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
				},

				//TOP
				{
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.2,
					1.0f,
					1.0f,
					1.0f,
					0.2,
					1.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
				},
				{
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					1.0f,
					0.2,
					1.0f,
					1.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
				},

				//BOTTOM
				{
					1.0f,
					0.0f,
					0.2,
					1.0f,
					0.0f,
					0.0f,
					0.2,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
				},
				{
					1.0f,
					0.0f,
					0.2,
					1.0f,
					0.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					1.0f,
					0.0f,
					0.0f,
					1.0f,
					1.0f,
					0.0f,
				},
			};

		//initialise the 3D Graphics Jenova Extension as this is required to setup internal buffers to the same size as the main output
		JenovaSpace::GFX3D::ConfigureDisplay();

		//configure the rendering pipeline with projection and viewport properties
		pipeRender.SetProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1, 1000.0f, 0.0f, 0.0f, ScreenWidth(), ScreenHeight());

		//also make a projection matrix, we might need this later
		matProj = JenovaSpace::GFX3D::Math::Mat_MakeProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1, 1000.0f);

		//setting affine texture mapping to false, in order to correct for perspective
		pipeRender.SetTextureMappingMode(false);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		//directly manipulate camera
		if (GetKey(JenovaSpace::Key::W).bHeld)
		{
			vecCharPos.y += 2.0f * fElapsedTime;
		}
		if (GetKey(JenovaSpace::Key::S).bHeld)
		{
			vecCharPos.y -= 2.0f * fElapsedTime;
		}
		if (GetKey(JenovaSpace::Key::A).bHeld)
		{
			vecCharPos.x -= 2.0f * fElapsedTime;
		}
		if (GetKey(JenovaSpace::Key::D).bHeld)
		{
			vecCharPos.x += 2.0f * fElapsedTime;
		}
		//manipulate it by only going forward and spinning, as you would a car

		if (GetKey(JenovaSpace::Key::Z).bHeld)
			fCameraZ += 5.0f * fElapsedTime;
		if (GetKey(JenovaSpace::Key::X).bHeld)
			fCameraZ -= 5.0f * fElapsedTime;

		if (GetKey(JenovaSpace::Key::F5).bReleased)
			SaveCity("example1.city");
		if (GetKey(JenovaSpace::Key::F8).bReleased)
			LoadCity("example1.city");

		//if there are no selected cells, then only edit the cell under the current mouse cursor, otherwise iterate through the set of sleected cells and apply to all of them

		//check that cell exists in valid 2D map space
		if (nMouseWorldX >= 0 && nMouseWorldX < nMapWidth && nMouseWorldY >= 0 && nMouseWorldY < nMapHeight)
		{
			//press "R" to toggle Road flag for selected cell(s)
			if (GetKey(JenovaSpace::Key::R).bPressed)
			{
				if (!setSelectedCells.empty())
				{
					for (auto &cell : setSelectedCells)
					{
						cell->bRoad = !cell->bRoad;
					}
				}
				else
					pMap[nMouseWorldY * nMapWidth + nMouseWorldX].bRoad = !pMap[nMouseWorldY * nMapWidth + nMouseWorldX].bRoad;
			}

			//press "T" to increase height for selected cell(s)
			if (GetKey(JenovaSpace::Key::T).bPressed)
			{
				if (!setSelectedCells.empty())
				{
					for (auto &cell : setSelectedCells)
					{
						cell->nHeight++;
					}
				}
				else
					pMap[nMouseWorldY * nMapWidth + nMouseWorldX].nHeight++;
			}

			//press "E" to decrease height for selected cell(s)
			if (GetKey(JenovaSpace::Key::E).bPressed)
			{
				if (!setSelectedCells.empty())
				{
					for (auto &cell : setSelectedCells)
					{
						cell->nHeight--;
					}
				}
				else
					pMap[nMouseWorldY * nMapWidth + nMouseWorldX].nHeight--;
			}
		}

		if (GetKey(JenovaSpace::Key::LEFT).bHeld)
			fCharAngle -= 4.0f * fElapsedTime;
		if (GetKey(JenovaSpace::Key::RIGHT).bHeld)
			fCharAngle += 4.0f * fElapsedTime;

		JenovaSpace::GFX3D::Vector3D a = {1, 0, 0};
		JenovaSpace::GFX3D::Matrix4x4 m = JenovaSpace::GFX3D::Math::Mat_MakeRotationZ(fCharAngle);
		vecCharVel = JenovaSpace::GFX3D::Math::Mat_MultiplyVector(m, a);

		if (GetKey(JenovaSpace::Key::UP).bHeld)
		{
			vecCharPos.x += vecCharVel.x * fCharSpeed * fElapsedTime;
			vecCharPos.y += vecCharVel.y * fCharSpeed * fElapsedTime;
		}

		//the camera currently follows the char, and the char stays in the middle of the screen we have to know where the camera is before we can work with on screen interactions
		fCameraY = vecCharPos.y;
		fCameraX = vecCharPos.x;
		vEye = {fCameraX, fCameraY, fCameraZ};
		JenovaSpace::GFX3D::Vector3D vLookTarget = JenovaSpace::GFX3D::Math::Vec_Add(vEye, vLookDir);

		//setup the camera properties for the pipeline - aka "view" transform
		pipeRender.SetCamera(vEye, vLookTarget, vUp);

		JenovaSpace::GFX3D::Matrix4x4 matView = JenovaSpace::GFX3D::Math::Mat_PointAt(vEye, vLookTarget, vUp);

		//presume the origin of the mouse ray is the middle of the screen...
		JenovaSpace::GFX3D::Vector3D vecMouseOrigin = {0.0f, 0.0f, 0.0f};

		//...and that a ray is cast to the mouse location from the origin and translate the mouse coordinates into viewport coordinates
		JenovaSpace::GFX3D::Vector3D vecMouseDir = {
			2.0f * ((GetMouseX() / (float)ScreenWidth()) - 0.5f) / matProj.m[0][0],
			2.0f * ((GetMouseY() / (float)ScreenHeight()) - 0.5f) / matProj.m[1][1],
			1.0f,
			0.0f};

		//now transform the origin point and ray direction by the inverse of the camera
		vecMouseOrigin = JenovaSpace::GFX3D::Math::Mat_MultiplyVector(matView, vecMouseOrigin);
		vecMouseDir = JenovaSpace::GFX3D::Math::Mat_MultiplyVector(matView, vecMouseDir);

		//extend the mouse ray to a large length
		vecMouseDir = JenovaSpace::GFX3D::Math::Vec_Mul(vecMouseDir, 1000.0f);

		//offset the mouse ray by the mouse origin
		vecMouseDir = JenovaSpace::GFX3D::Math::Vec_Add(vecMouseOrigin, vecMouseDir);

		//all of our intersections for mouse checks occur in the ground plane (z=0), so define a plane at that location
		JenovaSpace::GFX3D::Vector3D plane_p = {0.0f, 0.0f, 0.0f};
		JenovaSpace::GFX3D::Vector3D plane_n = {0.0f, 0.0f, 1.0f};

		//calculate Mouse Location in plane, by doing a line/plane intersection test
		float t = 0.0f;
		JenovaSpace::GFX3D::Vector3D mouse3d = JenovaSpace::GFX3D::Math::Vec_IntersectPlane(plane_p, plane_n, vecMouseOrigin, vecMouseDir, t);

		//left click & left click drag selects cells by adding them to the set of selected cells
		if (GetMouse(0).bHeld && ((nMouseWorldX != nOldMouseWorldX) || (nMouseWorldY != nOldMouseWorldY)))
		{
			setSelectedCells.emplace(&pMap[nMouseWorldY * nMapWidth + nMouseWorldX]);
		}
		//single clicking cells also adds them
		if (GetMouse(0).bPressed)
		{
			setSelectedCells.emplace(&pMap[nMouseWorldY * nMapWidth + nMouseWorldX]);
		}
		//if the user right clicks, the set of selected cells is emptied
		if (GetMouse(1).bReleased)
		{
			setSelectedCells.clear();
		}
		//cache the current mouse position to use during comparison in next frame
		nOldMouseWorldX = nMouseWorldX;
		nOldMouseWorldY = nMouseWorldY;

		nMouseWorldX = (int)mouse3d.x;
		nMouseWorldY = (int)mouse3d.y;

		//rendering area

		//first Clear the screen and the depth buffer
		Clear(JenovaSpace::BLUE);
		JenovaSpace::GFX3D::ClearDepth();

		//work out Top Left Ground Cell
		vecMouseDir = {-1.0f / matProj.m[0][0], -1.0f / matProj.m[1][1], 1.0f, 0.0f};
		vecMouseDir = JenovaSpace::GFX3D::Math::Mat_MultiplyVector(matView, vecMouseDir);
		vecMouseDir = JenovaSpace::GFX3D::Math::Vec_Mul(vecMouseDir, 1000.0f);
		vecMouseDir = JenovaSpace::GFX3D::Math::Vec_Add(vecMouseOrigin, vecMouseDir);
		viewWorldTopLeft = JenovaSpace::GFX3D::Math::Vec_IntersectPlane(plane_p, plane_n, vecMouseOrigin, vecMouseDir, t);

		//work out Bottom Right Ground Cell
		vecMouseDir = {1.0f / matProj.m[0][0], 1.0f / matProj.m[1][1], 1.0f, 0.0f};
		vecMouseDir = JenovaSpace::GFX3D::Math::Mat_MultiplyVector(matView, vecMouseDir);
		vecMouseDir = JenovaSpace::GFX3D::Math::Vec_Mul(vecMouseDir, 1000.0f);
		vecMouseDir = JenovaSpace::GFX3D::Math::Vec_Add(vecMouseOrigin, vecMouseDir);
		viewWorldBottomRight = JenovaSpace::GFX3D::Math::Vec_IntersectPlane(plane_p, plane_n, vecMouseOrigin, vecMouseDir, t);

		//calculate visible tiles
		//int nStartX = 0;
		//int nEndX = nMapWidth;
		//int nStartY = 0;
		//int nEndY = nMapHeight;

		int nStartX = std::max(0, (int)viewWorldTopLeft.x - 1);
		int nEndX = std::min(nMapWidth, (int)viewWorldBottomRight.x + 1);
		int nStartY = std::max(0, (int)viewWorldTopLeft.y - 1);
		int nEndY = std::min(nMapHeight, (int)viewWorldBottomRight.y + 1);

		//iterate through all the cells we wish to draw and each cell is 1x1 and elevates in the Z -Axis
		for (int x = nStartX; x < nEndX; x++)
		{
			for (int y = nStartY; y < nEndY; y++)
			{
				if (pMap[y * nMapWidth + x].bRoad)
				{
					//cell is a road, look at neighbouring cells, if they are roads also, then choose the appropriate texture that joins up correctly
					//recycle this for the lighting demo
					int road = 0;
					auto r = [&](int i, int j) {
						return pMap[(y + j) * nMapWidth + (x + i)].bRoad;
					};

					if (r(0, -1) && r(0, +1) && !r(-1, 0) && !r(+1, 0))
						road = 0;
					if (!r(0, -1) && !r(0, +1) && r(-1, 0) && r(+1, 0))
						road = 1;

					if (!r(0, -1) && r(0, +1) && !r(-1, 0) && r(+1, 0))
						road = 3;
					if (!r(0, -1) && r(0, +1) && r(-1, 0) && r(+1, 0))
						road = 4;
					if (!r(0, -1) && r(0, +1) && r(-1, 0) && !r(+1, 0))
						road = 5;

					if (r(0, -1) && r(0, +1) && !r(-1, 0) && r(+1, 0))
						road = 6;
					if (r(0, -1) && r(0, +1) && r(-1, 0) && r(+1, 0))
						road = 7;
					if (r(0, -1) && r(0, +1) && r(-1, 0) && !r(+1, 0))
						road = 8;

					if (r(0, -1) && !r(0, +1) && !r(-1, 0) && r(+1, 0))
						road = 9;
					if (r(0, -1) && !r(0, +1) && r(-1, 0) && r(+1, 0))
						road = 10;
					if (r(0, -1) && !r(0, +1) && r(-1, 0) && !r(+1, 0))
						road = 11;

					//create a translation transform to position the cell in the world
					JenovaSpace::GFX3D::Matrix4x4 matWorld = JenovaSpace::GFX3D::Math::Mat_MakeTranslation(x, y, 0.0f);
					pipeRender.SetTransform(matWorld);

					//set the appropriate texture to use
					pipeRender.SetTexture(sprRoad[road]);

					//draw a flat quad
					pipeRender.Render(meshFlat.tris);
				}
				else //not Road
				{
					//if the cell is not considered road, then draw it appropriately

					if (pMap[y * nMapWidth + x].nHeight < 0)
					{
						//水はまだいないね
					}

					if (pMap[y * nMapWidth + x].nHeight == 0)
					{
						//cell is ground, draw a flat grass quad at height 0
						JenovaSpace::GFX3D::Matrix4x4 matWorld = JenovaSpace::GFX3D::Math::Mat_MakeTranslation(x, y, 0.0f);
						pipeRender.SetTransform(matWorld);
						pipeRender.SetTexture(sprGround);
						pipeRender.Render(meshFlat.tris);
					}

					if (pMap[y * nMapWidth + x].nHeight > 0)
					{
						//Cell is a cube, for now, it'll draw each level as a seperate mesh
						int h, t;
						t = pMap[y * nMapWidth + x].nHeight;

						for (h = 0; h < t; h++)
						{
							//create a transform that positions the 3d object according to its height
							JenovaSpace::GFX3D::Matrix4x4 matWorld = JenovaSpace::GFX3D::Math::Mat_MakeTranslation(x, y, -(h + 1) * 0.2f);
							pipeRender.SetTransform(matWorld);

							//choose a texture, if its ground level, use the dirt road map
							pipeRender.SetTexture(h == 0 ? sprFrontage : sprWindows);
							pipeRender.Render(meshWallsOut.tris);
						}

						//top of the 3d cube
						JenovaSpace::GFX3D::Matrix4x4 matWorld = JenovaSpace::GFX3D::Math::Mat_MakeTranslation(x, y, -(h)*0.2f);
						pipeRender.SetTransform(matWorld);
						pipeRender.SetTexture(sprRoof);
						pipeRender.Render(meshFlat.tris);
					}
				}
			}
		}

		//draw Selected Cells, iterate through the set of cells, and draw a wireframe quad at ground level to indicate it is in the selection set
		for (auto &cell : setSelectedCells)
		{
			//draw CursorCube
			JenovaSpace::GFX3D::Matrix4x4 matWorld = JenovaSpace::GFX3D::Math::Mat_MakeTranslation(cell->nWorldX, cell->nWorldY, 0.0f);
			pipeRender.SetTransform(matWorld);
			pipeRender.SetTexture(sprRoof);
			pipeRender.Render(meshFlat.tris, JenovaSpace::GFX3D::RENDER_WIRE);
		}

		//draw Char, a few transforms required for this

		//1) Offset the char to the middle of the quad
		JenovaSpace::GFX3D::Matrix4x4 matCharOffset = JenovaSpace::GFX3D::Math::Mat_MakeTranslation(-0.5, -0.5, -0.0f);
		//2) The quad is currently unit square, scale it to be more rectangular and smaller than the cells
		JenovaSpace::GFX3D::Matrix4x4 matCharScale = JenovaSpace::GFX3D::Math::Mat_MakeScale(0.2, 0.4, 1.0f);
		//3) Combine into matrix
		JenovaSpace::GFX3D::Matrix4x4 matChar = JenovaSpace::GFX3D::Math::Mat_MultiplyMatrix(matCharOffset, matCharScale);
		//4) Rotate the char around its offset origin, according to its angle
		JenovaSpace::GFX3D::Matrix4x4 matCharRot = JenovaSpace::GFX3D::Math::Mat_MakeRotationZ(fCharAngle);
		matChar = JenovaSpace::GFX3D::Math::Mat_MultiplyMatrix(matChar, matCharRot);
		//5) Translate the char into its position in the world. Give it a little elevation so its baove the ground
		JenovaSpace::GFX3D::Matrix4x4 matCharTrans = JenovaSpace::GFX3D::Math::Mat_MakeTranslation(vecCharPos.x, vecCharPos.y, -0.01f);
		matChar = JenovaSpace::GFX3D::Math::Mat_MultiplyMatrix(matChar, matCharTrans);

		//set the char texture to the pipeline
		pipeRender.SetTexture(sprChar[0]);
		//apply "world" transform to pipeline
		pipeRender.SetTransform(matChar);

		//the char has transparency, so enable it
		SetPixelMode(JenovaSpace::Pixel::ALPHA);
		//Render the quad
		pipeRender.Render(meshFlat.tris);
		//set transparency back to none to optimise drawing other pixels
		SetPixelMode(JenovaSpace::Pixel::NORMAL);

		//draw the current camera position for debug information
		DrawString(10, 30, "CX: " + std::to_string(fCameraX) + " CY: " + std::to_string(fCameraY) + " CZ: " + std::to_string(fCameraZ));
		return true;
	}
};

struct sShape;

//define a node
struct sNode
{
	sShape *parent;
	JenovaSpace::vf2d pos;
};

//the baseE class, defines the interface for all shapes
struct sShape
{
	//shapes are defined by the placment of nodes
	std::vector<sNode> vecNodes;
	uint32_t nMaxNodes = 0;

	//the colour of the shape
	JenovaSpace::Pixel col = JenovaSpace::GREEN;

	//all shapes share word to screen transformation coefficients, so share them staically
	static float fWorldScale;
	static JenovaSpace::vf2d vWorldOffset;

	//convert coordinates from World Space --> Screen Space
	void WorldToScreen(const JenovaSpace::vf2d &v, int &nScreenX, int &nScreenY)
	{
		nScreenX = (int)((v.x - vWorldOffset.x) * fWorldScale);
		nScreenY = (int)((v.y - vWorldOffset.y) * fWorldScale);
	}

	//PURE function, which makes this class abstract, an implementation must come through an override
	virtual void DrawYourself(JenovaSpace::JenovaPixel *pge) = 0;

	//Shapes are defined by nodes　and once the maximum number of nodes for a shape have been placed, it returns nullptr
	sNode *GetNextNode(const JenovaSpace::vf2d &p)
	{
		if (vecNodes.size() == nMaxNodes)
			return nullptr; //Shape is complete so no new nodes to be issued

		//else create new node and add to shapes node vector
		sNode n;
		n.parent = this;
		n.pos = p;
		vecNodes.push_back(n);
		//これはま。。。良くないね。。。
		return &vecNodes[vecNodes.size() - 1];
	}

	//test to see if supplied coordinate exists at same location as any of the nodes for this shape and returns a pointer to that node if it does
	sNode *HitNode(JenovaSpace::vf2d &p)
	{
		for (auto &n : vecNodes)
		{
			if ((p - n.pos).mag() < 0.01f)
				return &n;
		}

		return nullptr;
	}

	//draw all of the nodes that define this shape so far
	void DrawNodes(JenovaSpace::JenovaPixel *pge)
	{
		for (auto &n : vecNodes)
		{
			int sx, sy;
			WorldToScreen(n.pos, sx, sy);
			pge->FillCircle(sx, sy, 2, JenovaSpace::RED);
		}
	}
};

//must provide an implementation of static variables
float sShape::fWorldScale = 1.0f;
JenovaSpace::vf2d sShape::vWorldOffset = {0, 0};

//line sub class, inherits from sShape
struct sLine : public sShape
{
	sLine()
	{
		nMaxNodes = 2;
		vecNodes.reserve(nMaxNodes);
	}

	//implements custom DrawYourself Function, meaning the shape is no longer abstract
	void DrawYourself(JenovaSpace::JenovaPixel *pge) override
	{
		int sx, sy, ex, ey;
		WorldToScreen(vecNodes[0].pos, sx, sy);
		WorldToScreen(vecNodes[1].pos, ex, ey);
		pge->DrawLine(sx, sy, ex, ey, col);
	}
};

//box sub class, inherits from sShape
struct sBox : public sShape
{
	sBox()
	{
		nMaxNodes = 2;
		vecNodes.reserve(nMaxNodes);
	}

	void DrawYourself(JenovaSpace::JenovaPixel *pge) override
	{
		int sx, sy, ex, ey;
		WorldToScreen(vecNodes[0].pos, sx, sy);
		WorldToScreen(vecNodes[1].pos, ex, ey);
		pge->DrawRect(sx, sy, ex - sx, ey - sy, col);
	}
};

//circle sub class, inherits from sShape
struct sCircle : public sShape
{
	sCircle()
	{
		nMaxNodes = 2;
		vecNodes.reserve(nMaxNodes);
	}

	void DrawYourself(JenovaSpace::JenovaPixel *pge) override
	{
		float fRadius = (vecNodes[0].pos - vecNodes[1].pos).mag();
		int sx, sy, ex, ey;
		WorldToScreen(vecNodes[0].pos, sx, sy);
		WorldToScreen(vecNodes[1].pos, ex, ey);
		pge->DrawLine(sx, sy, ex, ey, col, 0xFF00FF00);

		//note the radius is also scaled so it is drawn appropriately
		pge->DrawCircle(sx, sy, (int32_t)(fRadius * fWorldScale), col);
	}
};

//the curve class is actually a Bezier spline so it requires 3 nodes to be defined fully
struct sCurve : public sShape
{
	sCurve()
	{
		nMaxNodes = 3;
		vecNodes.reserve(nMaxNodes);
	}

	void DrawYourself(JenovaSpace::JenovaPixel *pge) override
	{
		int sx, sy, ex, ey;

		if (vecNodes.size() < 3)
		{
			//can only draw line from first to second
			WorldToScreen(vecNodes[0].pos, sx, sy);
			WorldToScreen(vecNodes[1].pos, ex, ey);
			pge->DrawLine(sx, sy, ex, ey, col, 0xFF00FF00);
		}

		if (vecNodes.size() == 3)
		{
			//can draw line from first to second
			WorldToScreen(vecNodes[0].pos, sx, sy);
			WorldToScreen(vecNodes[1].pos, ex, ey);
			pge->DrawLine(sx, sy, ex, ey, col, 0xFF00FF00);

			//can draw second structural line
			WorldToScreen(vecNodes[1].pos, sx, sy);
			WorldToScreen(vecNodes[2].pos, ex, ey);
			pge->DrawLine(sx, sy, ex, ey, col, 0xFF00FF00);

			//and bezier curve
			JenovaSpace::vf2d op = vecNodes[0].pos;
			JenovaSpace::vf2d np = op;
			for (float t = 0; t < 1.0f; t += 0.01f)
			{
				np = (1 - t) * (1 - t) * vecNodes[0].pos + 2 * (1 - t) * t * vecNodes[1].pos + t * t * vecNodes[2].pos;
				WorldToScreen(op, sx, sy);
				WorldToScreen(np, ex, ey);
				pge->DrawLine(sx, sy, ex, ey, col);
				op = np;
			}
		}
	}
};

#pragma endregion

#pragma region "CAD"
//CAD-like application starts here
class Polymorphism : public JenovaSpace::JenovaPixel
{

public:
	Polymorphism()
	{
		sAppName = "Polymorphism";
	}

private:
	//Pan & Zoom variables
	JenovaSpace::vf2d vOffset = {0.0f, 0.0f};
	JenovaSpace::vf2d vStartPan = {0.0f, 0.0f};
	float fScale = 10.0f;
	float fGrid = 1.0f;

	//convert coordinates from World Space --> Screen Space
	void WorldToScreen(const JenovaSpace::vf2d &v, int &nScreenX, int &nScreenY)
	{
		nScreenX = (int)((v.x - vOffset.x) * fScale);
		nScreenY = (int)((v.y - vOffset.y) * fScale);
	}

	//convert coordinates from Screen Space --> World Space
	void ScreenToWorld(int nScreenX, int nScreenY, JenovaSpace::vf2d &v)
	{
		v.x = (float)(nScreenX) / fScale + vOffset.x;
		v.y = (float)(nScreenY) / fScale + vOffset.y;
	}

	//pointer to a shape that is currently being defined by the placment of nodes
	sShape *tempShape = nullptr;

	//list of pointers to all shapes which have been drawn so far
	std::list<sShape *> listShapes;

	//pointer to a node that is currently selected and selected nodes follow the mouse cursor
	sNode *selectedNode = nullptr;

	//"snapped" mouse location
	JenovaSpace::vf2d vCursor = {0, 0};

public:
	bool OnUserCreate() override
	{
		//configure world space (0,0) to be middle of screen space
		vOffset = {(float)(-ScreenWidth() / 2) / fScale, (float)(-ScreenHeight() / 2) / fScale};
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		//get mouse location this frame
		JenovaSpace::vf2d vMouse = {(float)GetMouseX(), (float)GetMouseY()};

		//handle Pan & Zoom
		if (GetMouse(2).bPressed)
		{
			vStartPan = vMouse;
		}

		if (GetMouse(2).bHeld)
		{
			vOffset -= (vMouse - vStartPan) / fScale;
			vStartPan = vMouse;
		}

		JenovaSpace::vf2d vMouseBeforeZoom;
		ScreenToWorld((int)vMouse.x, (int)vMouse.y, vMouseBeforeZoom);

		if (GetKey(JenovaSpace::Key::Q).bHeld || GetMouseWheel() > 0)
		{
			fScale *= 1.1f;
		}

		if (GetKey(JenovaSpace::Key::A).bHeld || GetMouseWheel() < 0)
		{
			fScale *= 0.9f;
		}

		JenovaSpace::vf2d vMouseAfterZoom;
		ScreenToWorld((int)vMouse.x, (int)vMouse.y, vMouseAfterZoom);
		vOffset += (vMouseBeforeZoom - vMouseAfterZoom);

		//snap mouse cursor to nearest grid interval
		vCursor.x = floorf((vMouseAfterZoom.x + 0.5f) * fGrid);
		vCursor.y = floorf((vMouseAfterZoom.y + 0.5f) * fGrid);

		if (GetKey(JenovaSpace::Key::L).bPressed)
		{
			tempShape = new sLine();

			//place first node at location of keypress
			selectedNode = tempShape->GetNextNode(vCursor);

			//get Second node
			selectedNode = tempShape->GetNextNode(vCursor);
		}

		if (GetKey(JenovaSpace::Key::B).bPressed)
		{
			tempShape = new sBox();

			//place first node at location of keypress
			selectedNode = tempShape->GetNextNode(vCursor);

			//get Second node
			selectedNode = tempShape->GetNextNode(vCursor);
		}

		if (GetKey(JenovaSpace::Key::C).bPressed)
		{
			//create new shape as a temporary
			tempShape = new sCircle();

			//place first node at location of keypress
			selectedNode = tempShape->GetNextNode(vCursor);

			//get Second node
			selectedNode = tempShape->GetNextNode(vCursor);
		}

		if (GetKey(JenovaSpace::Key::S).bPressed)
		{
			//create new shape as a temporary
			tempShape = new sCurve();

			//place first node at location of keypress
			selectedNode = tempShape->GetNextNode(vCursor);

			//get Second node
			selectedNode = tempShape->GetNextNode(vCursor);
		}

		//search for any node that exists under the cursor, if one is found then select it
		if (GetKey(JenovaSpace::Key::M).bPressed)
		{
			selectedNode = nullptr;
			for (auto &shape : listShapes)
			{
				selectedNode = shape->HitNode(vCursor);
				if (selectedNode != nullptr)
					break;
			}
		}

		//if a node is selected, make it follow the mouse cursor by updating its position
		if (selectedNode != nullptr)
		{
			selectedNode->pos = vCursor;
		}

		//as the user left clicks to place nodes, the shape can grow until it requires no more nodes, at which point it is completed and added to the list of completed shapes
		if (GetMouse(0).bReleased)
		{
			if (tempShape != nullptr)
			{
				selectedNode = tempShape->GetNextNode(vCursor);
				if (selectedNode == nullptr)
				{
					tempShape->col = JenovaSpace::WHITE;
					listShapes.push_back(tempShape);
				}
			}
		}

		//clear Screen
		Clear(JenovaSpace::VERY_DARK_BLUE);

		int sx, sy;
		int ex, ey;

		//get visible world
		JenovaSpace::vf2d vWorldTopLeft, vWorldBottomRight;
		ScreenToWorld(0, 0, vWorldTopLeft);
		ScreenToWorld(ScreenWidth(), ScreenHeight(), vWorldBottomRight);

		//get values just beyond screen boundaries
		vWorldTopLeft.x = floor(vWorldTopLeft.x);
		vWorldTopLeft.y = floor(vWorldTopLeft.y);
		vWorldBottomRight.x = ceil(vWorldBottomRight.x);
		vWorldBottomRight.y = ceil(vWorldBottomRight.y);

		//draw Grid dots
		for (float x = vWorldTopLeft.x; x < vWorldBottomRight.x; x += fGrid)
		{
			for (float y = vWorldTopLeft.y; y < vWorldBottomRight.y; y += fGrid)
			{
				WorldToScreen({x, y}, sx, sy);
				Draw(sx, sy, JenovaSpace::BLUE);
			}
		}

		//draw World Axis
		WorldToScreen({0, vWorldTopLeft.y}, sx, sy);
		WorldToScreen({0, vWorldBottomRight.y}, ex, ey);
		DrawLine(sx, sy, ex, ey, JenovaSpace::GREY, 0xF0F0F0F0);
		WorldToScreen({vWorldTopLeft.x, 0}, sx, sy);
		WorldToScreen({vWorldBottomRight.x, 0}, ex, ey);
		DrawLine(sx, sy, ex, ey, JenovaSpace::GREY, 0xF0F0F0F0);

		//update shape translation coefficients
		sShape::fWorldScale = fScale;
		sShape::vWorldOffset = vOffset;

		//draw All Existing Shapes
		for (auto &shape : listShapes)
		{
			shape->DrawYourself(this);
			shape->DrawNodes(this);
		}

		//draw shape currently being defined
		if (tempShape != nullptr)
		{
			tempShape->DrawYourself(this);
			tempShape->DrawNodes(this);
		}

		//draw "Snapped" Cursor
		WorldToScreen(vCursor, sx, sy);
		DrawCircle(sx, sy, 3, JenovaSpace::YELLOW);

		//draw Cursor Position
		DrawString(10, 10, "X=" + std::to_string(vCursor.x) + ", Y=" + std::to_string(vCursor.y), JenovaSpace::YELLOW, 2);
		return true;
	}
};

#pragma endregion

#pragma region "Lighting Demo"
//structures required for the shadow casting algorithm
struct sEdge
{
	float sx, sy; //start coordinate
	float ex, ey; //end coordinate
};

struct sCell
{
	int edge_id[4];
	bool edge_exist[4];
	bool exist = false;
};
//2D shadow casting cell
class ShadowCasting2D : public JenovaSpace::JenovaPixel
{
public:
	ShadowCasting2D()
	{
		sAppName = "ShadowCasting2D";
	}

private:
	sCell *world;
	int nWorldWidth = 40;
	int nWorldHeight = 30;

	JenovaSpace::Sprite *sprLightCast;
	JenovaSpace::Sprite *buffLightRay;
	JenovaSpace::Sprite *buffLightTex;

	std::vector<sEdge> vecEdges;

	//				angle	x	y
	std::vector<std::tuple<float, float, float>> vecVisibilityPolygonPoints;

	void ConvertTileMapToPolyMap(int sx, int sy, int w, int h, float fBlockWidth, int pitch)
	{
		//clear "PolyMap"
		vecEdges.clear();

		for (int x = 0; x < w; x++)
			for (int y = 0; y < h; y++)
				for (int j = 0; j < 4; j++)
				{
					world[(y + sy) * pitch + (x + sx)].edge_exist[j] = false;
					world[(y + sy) * pitch + (x + sx)].edge_id[j] = 0;
				}

		//iterate through region from top left to bottom right
		for (int x = 1; x < w - 1; x++)
			for (int y = 1; y < h - 1; y++)
			{
				//create some convenient indices
				int i = (y + sy) * pitch + (x + sx);	 //this
				int n = (y + sy - 1) * pitch + (x + sx); //Northern Neighbour
				int s = (y + sy + 1) * pitch + (x + sx); //Southern Neighbour
				int w = (y + sy) * pitch + (x + sx - 1); //Western Neighbour
				int e = (y + sy) * pitch + (x + sx + 1); //Eastern Neighbour

				//if this cell exists, check if it needs edges
				if (world[i].exist)
				{
					//if this cell has no western neighbour, it needs a western edge
					if (!world[w].exist)
					{
						//it can either extend it from its northern neighbour if they have
						//one, or It can start a new one.
						if (world[n].edge_exist[WEST])
						{
							//northern neighbour has a western edge, so grow it downwards
							vecEdges[world[n].edge_id[WEST]].ey += fBlockWidth;
							world[i].edge_id[WEST] = world[n].edge_id[WEST];
							world[i].edge_exist[WEST] = true;
						}
						else
						{
							//northern neighbour does not have one, so create one
							sEdge edge;
							edge.sx = (sx + x) * fBlockWidth;
							edge.sy = (sy + y) * fBlockWidth;
							edge.ex = edge.sx;
							edge.ey = edge.sy + fBlockWidth;

							//add edge to Polygon Pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							//update tile information with edge information
							world[i].edge_id[WEST] = edge_id;
							world[i].edge_exist[WEST] = true;
						}
					}

					//if this cell dont have an eastern neignbour, It needs a eastern edge
					if (!world[e].exist)
					{
						//it can either extend it from its northern neighbour if they have
						//one, or It can start a new one.
						if (world[n].edge_exist[EAST])
						{
							//northern neighbour has one, so grow it downwards
							vecEdges[world[n].edge_id[EAST]].ey += fBlockWidth;
							world[i].edge_id[EAST] = world[n].edge_id[EAST];
							world[i].edge_exist[EAST] = true;
						}
						else
						{
							//northern neighbour does not have one, so create one
							sEdge edge;
							edge.sx = (sx + x + 1) * fBlockWidth;
							edge.sy = (sy + y) * fBlockWidth;
							edge.ex = edge.sx;
							edge.ey = edge.sy + fBlockWidth;

							//add edge to Polygon Pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							//update tile information with edge information
							world[i].edge_id[EAST] = edge_id;
							world[i].edge_exist[EAST] = true;
						}
					}

					//if this cell doesnt have a northern neignbour, It needs a northern edge
					if (!world[n].exist)
					{
						//it can either extend it from its western neighbour if they have
						//one, or It can start a new one.
						if (world[w].edge_exist[NORTH])
						{
							//western neighbour has one, so grow it eastwards
							vecEdges[world[w].edge_id[NORTH]].ex += fBlockWidth;
							world[i].edge_id[NORTH] = world[w].edge_id[NORTH];
							world[i].edge_exist[NORTH] = true;
						}
						else
						{
							//western neighbour does not have one, so create one
							sEdge edge;
							edge.sx = (sx + x) * fBlockWidth;
							edge.sy = (sy + y) * fBlockWidth;
							edge.ex = edge.sx + fBlockWidth;
							edge.ey = edge.sy;

							//add edge to Polygon Pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							//update tile information with edge information
							world[i].edge_id[NORTH] = edge_id;
							world[i].edge_exist[NORTH] = true;
						}
					}

					//if this cell doesnt have a southern neignbour, It needs a southern edge
					if (!world[s].exist)
					{
						//it can either extend it from its western neighbour if they have one, or It can start a new one.
						if (world[w].edge_exist[SOUTH])
						{
							//western neighbour has one, so grow it eastwards
							vecEdges[world[w].edge_id[SOUTH]].ex += fBlockWidth;
							world[i].edge_id[SOUTH] = world[w].edge_id[SOUTH];
							world[i].edge_exist[SOUTH] = true;
						}
						else
						{
							//western neighbour does not have one, so I need to create one
							sEdge edge;
							edge.sx = (sx + x) * fBlockWidth;
							edge.sy = (sy + y + 1) * fBlockWidth;
							edge.ex = edge.sx + fBlockWidth;
							edge.ey = edge.sy;

							//add edge to Polygon Pool
							int edge_id = vecEdges.size();
							vecEdges.push_back(edge);

							//update tile information with edge information
							world[i].edge_id[SOUTH] = edge_id;
							world[i].edge_exist[SOUTH] = true;
						}
					}
				}
			}
	}

	void CalculateVisibilityPolygon(float ox, float oy, float radius)
	{
		//get rid of existing polygon
		vecVisibilityPolygonPoints.clear();

		//for each edge in PolyMap
		for (auto &e1 : vecEdges)
		{
			//take the start point, then the end point
			for (int i = 0; i < 2; i++)
			{
				float rdx, rdy;
				rdx = (i == 0 ? e1.sx : e1.ex) - ox;
				rdy = (i == 0 ? e1.sy : e1.ey) - oy;

				float base_ang = atan2f(rdy, rdx);

				float ang = 0;
				//for each point, cast 3 rays, 1 directly at point and 1 a little bit either side
				for (int j = 0; j < 3; j++)
				{
					if (j == 0)
						ang = base_ang - 0.0001f;
					if (j == 1)
						ang = base_ang;
					if (j == 2)
						ang = base_ang + 0.0001f;

					//create ray along angle for required distance
					rdx = radius * cosf(ang);
					rdy = radius * sinf(ang);

					float min_t1 = INFINITY;
					float min_px = 0, min_py = 0, min_ang = 0;
					bool bValid = false;

					//check for ray intersection with all edges
					for (auto &e2 : vecEdges)
					{
						//create line segment std::vector
						float sdx = e2.ex - e2.sx;
						float sdy = e2.ey - e2.sy;

						if (fabs(sdx - rdx) > 0.0f && fabs(sdy - rdy) > 0.0f)
						{
							//t2 is normalised distance from line segment start to line segment end of intersect point
							float t2 = (rdx * (e2.sy - oy) + (rdy * (ox - e2.sx))) / (sdx * rdy - sdy * rdx);
							//t1 is normalised distance from source along ray to ray length of intersect point
							float t1 = (e2.sx + sdx * t2 - ox) / rdx;

							//if intersect point exists along ray, and along line segment then intersect point is valid
							if (t1 > 0 && t2 >= 0 && t2 <= 1.0f)
							{
								//check if this intersect point is closest to source and if it is, then store this point and reject others
								if (t1 < min_t1)
								{
									min_t1 = t1;
									min_px = ox + rdx * t1;
									min_py = oy + rdy * t1;
									min_ang = atan2f(min_py - oy, min_px - ox);
									bValid = true;
								}
							}
						}
					}

					if (bValid)
					{
						//add intersection point to visibility polygon perimeter
						vecVisibilityPolygonPoints.push_back({min_ang, min_px, min_py});
					}
				}
			}
		}

		//sort perimeter points by angle from source, this will allow us to draw a triangle fan
		sort(
			vecVisibilityPolygonPoints.begin(),
			vecVisibilityPolygonPoints.end(),
			[&](const std::tuple<float, float, float> &t1, const std::tuple<float, float, float> &t2) {
				return std::get<0>(t1) < std::get<0>(t2);
			});
	}

public:
	bool OnUserCreate() override
	{
		world = new sCell[nWorldWidth * nWorldHeight];

		//add a boundary to the world
		for (int x = 1; x < (nWorldWidth - 1); x++)
		{
			world[1 * nWorldWidth + x].exist = true;
			world[(nWorldHeight - 2) * nWorldWidth + x].exist = true;
		}

		for (int x = 1; x < (nWorldHeight - 1); x++)
		{
			world[x * nWorldWidth + 1].exist = true;
			world[x * nWorldWidth + (nWorldWidth - 2)].exist = true;
		}

		sprLightCast = new JenovaSpace::Sprite("Resources/light_cast.png");

		//create some screen-sized off-screen buffers for lighting effect
		buffLightTex = new JenovaSpace::Sprite(ScreenWidth(), ScreenHeight());
		buffLightRay = new JenovaSpace::Sprite(ScreenWidth(), ScreenHeight());
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		float fBlockWidth = 16.0f;
		float fSourceX = GetMouseX();
		float fSourceY = GetMouseY();

		//set tile map blocks to on or off
		if (GetMouse(0).bReleased)
		{
			//i = y * width + x
			int i = ((int)fSourceY / (int)fBlockWidth) * nWorldWidth + ((int)fSourceX / (int)fBlockWidth);
			world[i].exist = !world[i].exist;
		}

		//take a region of "TileMap" and convert it to "PolyMap"  (also change later, cause as it is now, this is done at every frame lel)
		ConvertTileMapToPolyMap(0, 0, 40, 30, fBlockWidth, nWorldWidth);

		if (GetMouse(1).bHeld)
		{
			CalculateVisibilityPolygon(fSourceX, fSourceY, 1000.0f);
		}

		//drawing
		SetDrawTarget(nullptr);
		Clear(JenovaSpace::BLACK);

		int nRaysCast = vecVisibilityPolygonPoints.size();

		//remove duplicate (or simply similar) points from polygon
		auto it = unique(
			vecVisibilityPolygonPoints.begin(),
			vecVisibilityPolygonPoints.end(),
			[&](const std::tuple<float, float, float> &t1, const std::tuple<float, float, float> &t2) {
				return fabs(std::get<1>(t1) - std::get<1>(t2)) < 0.1f && fabs(std::get<2>(t1) - std::get<2>(t2)) < 0.1f;
			});

		vecVisibilityPolygonPoints.resize(distance(vecVisibilityPolygonPoints.begin(), it));

		int nRaysCast2 = vecVisibilityPolygonPoints.size();
		DrawString(4, 4, "Rays Cast: " + std::to_string(nRaysCast) + " Rays Drawn: " + std::to_string(nRaysCast2));

		//if drawing rays, set an offscreen texture as our target buffer
		if (GetMouse(1).bHeld && vecVisibilityPolygonPoints.size() > 1)
		{
			//clear offscreen buffer for sprite
			SetDrawTarget(buffLightTex);
			Clear(JenovaSpace::BLACK);

			//draw "Radial Light" sprite to offscreen buffer, centered around source location (the mouse coordinates, buffer is 512x512)
			DrawSprite(fSourceX - 255, fSourceY - 255, sprLightCast);

			//clear offsecreen buffer for rays
			SetDrawTarget(buffLightRay);
			Clear(JenovaSpace::BLANK);

			//draw each triangle in fan
			for (int i = 0; i < vecVisibilityPolygonPoints.size() - 1; i++)
			{
				FillTriangle(
					fSourceX,
					fSourceY,

					std::get<1>(vecVisibilityPolygonPoints[i]),
					std::get<2>(vecVisibilityPolygonPoints[i]),

					std::get<1>(vecVisibilityPolygonPoints[i + 1]),
					std::get<2>(vecVisibilityPolygonPoints[i + 1]));
			}

			//fan will have one open edge, so draw last point of fan to first
			FillTriangle(
				fSourceX,
				fSourceY,

				std::get<1>(vecVisibilityPolygonPoints[vecVisibilityPolygonPoints.size() - 1]),
				std::get<2>(vecVisibilityPolygonPoints[vecVisibilityPolygonPoints.size() - 1]),

				std::get<1>(vecVisibilityPolygonPoints[0]),
				std::get<2>(vecVisibilityPolygonPoints[0]));

			//wherever rays exist in ray sprite, copy over radial light sprite pixels
			SetDrawTarget(nullptr);
			for (int x = 0; x < ScreenWidth(); x++)
				for (int y = 0; y < ScreenHeight(); y++)
					if (buffLightRay->GetPixel(x, y).r > 0)
						Draw(x, y, buffLightTex->GetPixel(x, y));
		}

		//draw Blocks from TileMap
		for (int x = 0; x < nWorldWidth; x++)
		{
			for (int y = 0; y < nWorldHeight; y++)
			{
				if (world[y * nWorldWidth + x].exist)
				{
					FillRect(x * fBlockWidth, y * fBlockWidth, fBlockWidth, fBlockWidth, JenovaSpace::BLUE);
				}
			}
		}

		//draw Edges from PolyMap
		for (auto &e : vecEdges)
		{
			DrawLine(e.sx, e.sy, e.ex, e.ey);
			FillCircle(e.sx, e.sy, 3, JenovaSpace::RED);
			FillCircle(e.ex, e.ey, 3, JenovaSpace::RED);
		}

		return true;
	}
};

#pragma endregion

int main()
{
	//3 different demos originating from the very same header files

	/*
	ShadowCasting2D lightingDemo;

	if (lightingDemo.Construct(640, 480, 2, 2))
	{
		lightingDemo.Start();
	}
	//*/

	/*
	MapEditor demo;
	if (demo.Construct(640, 480, 2, 2))
	{
		demo.Start();
	}
	return 0;
	//*/

	///*
	Polymorphism poly;
	if (poly.Construct(1600, 980, 1, 1)) {
		poly.Start();
	}
	//*/
}