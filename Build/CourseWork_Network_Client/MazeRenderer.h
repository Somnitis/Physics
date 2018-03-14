#pragma once

#include <ncltech\GameObject.h>
#include <ncltech\CommonMeshes.h>
//#include "MazeGenerator.h"
#include "SearchAlgorithm.h"
#include <ncltech\CommonUtils.h>


struct WallDescriptor
{
	uint _xs, _xe;
	uint _ys, _ye;

	WallDescriptor(uint x, uint y) : _xs(x), _xe(x + 1), _ys(y), _ye(y + 1) {}
};

typedef std::vector<WallDescriptor> WallDescriptorVec;


class MazeRenderer : public GameObject
{
public:
	MazeRenderer(int size, vector<bool> Edges, Mesh* wallmesh = CommonMeshes::Cube());
	virtual ~MazeRenderer();

	//The search history draws from edges because they already store the 'to'
	// and 'from' of GraphNodes.
	void DrawWay(vector<Vector3> pos, float line_width);
	void StartEndPOS(int Start_x, int Start_y, int End_x, int End_y);


protected:
	//Turn MazeGenerator data into flat 2D map (3 size x 3 size) of boolean's
	// - True for wall
	// - False for empty
	//Returns uint: Guess at the number of walls required
	uint Generate_FlatMaze(int size, vector<bool> Edges);

	//Construct a list of WallDescriptors from the flat 2D map generated above.
	void Generate_ConstructWalls();

	//Finally turns the wall descriptors into actual renderable walls ready
	// to be seen on screen.
	void Generate_BuildRenderNodes();

protected:
	vector<bool>	allEdges;
	int				size;
	Mesh*			mesh;
	
	Vector3			start = Vector3(-1,-1,-1);
	RenderNode		*cubeS;
	Vector3			end = Vector3(-1, -1, -1);
	RenderNode		*cubeE;

	bool*	flat_maze;	//[flat_maze_size x flat_maze_size]
	uint	flat_maze_size;

	WallDescriptorVec	wall_descriptors;
};