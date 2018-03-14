#include "MazeRenderer.h"
#include <ncltech\CommonUtils.h>

const Vector4 wall_color = Vector4(1.f, 0.8f, 0.3f, 1);



MazeRenderer::MazeRenderer(int siz, vector<bool> Edges, Mesh* wallmesh)
	: GameObject("")
	, size(siz)
	, mesh(wallmesh)
	, allEdges(Edges)
	, flat_maze(NULL)
{
	this->SetRender(new RenderNode());
	

		uint num_walls = Generate_FlatMaze(size,allEdges);

		wall_descriptors.reserve(num_walls);

		Generate_ConstructWalls();

		Generate_BuildRenderNodes();

}

MazeRenderer::~MazeRenderer()
{
	mesh = NULL;
	//allEdges = NULL;

	if (flat_maze)
	{
		delete[] flat_maze;
		flat_maze = NULL;
	}
}

//The search history draws from edges because they already store the 'to'
// and 'from' of GraphNodes.
void MazeRenderer::DrawWay(vector<Vector3> pos, float line_width)
{
	float grid_scalar = 1.0f / (float)size;

	Matrix4 transform = this->Render()->GetWorldTransform();

	float index = 0.0f;
	for (int i=0; i < pos.size()-1;)
	{
		Vector3 start = transform * Vector3(
			(pos[i].x + 0.5f) * grid_scalar,
			0.1f,
			(pos[i].y + 0.5f) * grid_scalar);

		Vector3 end = transform * Vector3(
			(pos[i+1].x + 0.5f) * grid_scalar,
			0.1f,
			(pos[i+1].y + 0.5f) * grid_scalar);

		NCLDebug::DrawThickLine(start, end, line_width, CommonUtils::GenColor(0.8f));
		index += 1.0f;

		i += 2;
	}
}

uint MazeRenderer::Generate_FlatMaze(int siz, vector<bool> allEdges)
{
	//Generates a 3xsize by 3xsize array of booleans, where 
	// a true value corresponds to a solid wall and false to open space.
	// - Each GraphNode is a 2x2 open space with a 1 pixel wall around it.
	int size = siz;

	flat_maze_size = size * 3 - 1;

	if (flat_maze) delete[] flat_maze;
	flat_maze = new bool[flat_maze_size * flat_maze_size];
	memset(flat_maze, 0, flat_maze_size * flat_maze_size * sizeof(bool));


	int base_offset = size * (size - 1);
	int num_walls = 0;
	//Iterate over each cell in the maze
	for (int y = 0; y < size; ++y)
	{
		int y3 = y * 3;
		for (int x = 0; x < size; ++x)
		{
			int x3 = x * 3;
			
			//Lookup the corresponding edges that occupy that grid cell
			// and if they are walls, set plot their locations on our 2D
			// map.
			//- Yes... it's a horrible branching inner for-loop, my bad! :(
			if (x < size - 1)
			{
				bool edgeX = allEdges[(y * (size - 1) + x)];
				if (edgeX)
				{
					flat_maze[(y * 3) * flat_maze_size + (x * 3 + 2)] = true;
					flat_maze[(y * 3 + 1) * flat_maze_size + (x * 3 + 2)] = true;
					num_walls += 2;
				}
			}
			if (y < size - 1)
			{
				bool edgeY = allEdges[base_offset + (x * (size - 1) + y)];
				if (edgeY)
				{
					flat_maze[(y * 3 + 2) * flat_maze_size + (x * 3)]	= true;
					flat_maze[(y * 3 + 2) * flat_maze_size + (x * 3 + 1)] = true;
					num_walls += 2;
				}
			}

			//As it's now a 3x3 cell for each, and the doorways are 2x1 or 1x2
			// we need to add an extra wall for the diagonals.
			if (x < size - 1 && y < size - 1)
			{
				flat_maze[(y3 + 2) * flat_maze_size + x3 + 2] = true;
				num_walls++;
			}
		}
	}

	return num_walls;
}

void MazeRenderer::Generate_ConstructWalls()
{
//First try and compact adjacent walls down, so we don't
// just end up creating lots of little cube's.

	//Horizontal wall pass
	for (uint y = 0; y < flat_maze_size; ++y)
	{
		for (uint x = 0; x < flat_maze_size; ++x)
		{
			//Is the current cell a wall?
			if (flat_maze[y*flat_maze_size + x])
			{
				WallDescriptor w(x, y);

				uint old_x = x;

				//If we found a wall, keep iterating in the current
				// search direction and see if we can join it with
				// adjacent walls.
				for (++x; x < flat_maze_size; ++x)
				{
					if (!flat_maze[y * flat_maze_size + x])
						break;

					flat_maze[y * flat_maze_size + x] = false;
				}

				w._xe = x;

				//If the wall is only 1x1, ignore it for the vertical-pass.
				if (w._xe - w._xs > 1)
				{
					flat_maze[y * flat_maze_size + old_x] = false;
					wall_descriptors.push_back(w);
				}
			}
		}
	}

	//Vertical wall pass
	for (uint x = 0; x < flat_maze_size; ++x)
	{
		for (uint y = 0; y < flat_maze_size; ++y)
		{
			if (flat_maze[y * flat_maze_size + x])
			{
				WallDescriptor w(x, y);

				for (++y; y < flat_maze_size && flat_maze[y * flat_maze_size + x]; ++y) {}

				w._ye = y;
				wall_descriptors.push_back(w);
			}
		}
	}



}

void MazeRenderer::Generate_BuildRenderNodes()
{
	//Turn compacted walls into RenderNodes
	RenderNode *cube, *root = Render();

	//Turn walls into 3D Cuboids
	const float scalar = 1.f / (float)flat_maze_size;
	for (const WallDescriptor& w : wall_descriptors)
	{
		Vector3 start = Vector3(
			float(w._xs),
			0.0f,
			float(w._ys));

		Vector3 size = Vector3(
			float(w._xe - w._xs),
			0.0f,
			float(w._ye - w._ys)
		);


		start = start * scalar;
		Vector3 end = start + size * scalar;
		end.y = 0.75f;

		Vector3 centre = (end + start) * 0.5f;
		Vector3 halfDims = centre - start;

		cube = new RenderNode(mesh, wall_color);
		cube->SetTransform(Matrix4::Translation(centre) * Matrix4::Scale(halfDims));
		root->AddChild(cube);

	}

	//Add bounding edge walls to the maze
	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(-scalar*0.5f, 0.25f, 0.5)) * Matrix4::Scale(Vector3(scalar*0.5f, 0.25f, scalar + 0.5f)));
	root->AddChild(cube);

	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(1.f + scalar*0.5f, 0.25f, 0.5)) * Matrix4::Scale(Vector3(scalar*0.5f, 0.25f, scalar + 0.5f)));
	root->AddChild(cube);

	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(0.5, 0.25f, -scalar*0.5f)) * Matrix4::Scale(Vector3(0.5f, 0.25f, scalar*0.5f)));
	root->AddChild(cube);

	cube = new RenderNode(mesh, wall_color);
	cube->SetTransform(Matrix4::Translation(Vector3(0.5, 0.25f, 1.f + scalar*0.5f)) * Matrix4::Scale(Vector3(0.5f, 0.25f, scalar*0.5f)));
	root->AddChild(cube);


	this->SetRender(root);
}
//Finally - our start/end goals
void MazeRenderer::StartEndPOS(int Start_x, int Start_y, int End_x, int End_y) {

	RenderNode *root = Render();

	if (end != Vector3(-1, -1, -1))
	{
		root->RemoveChild(cubeE);
	}
	if (start != Vector3(-1, -1, -1))
	{
		root->RemoveChild(cubeS);
	}

	const float scalar = 1.f / (float)flat_maze_size;

	start = Vector3(Start_x, Start_y, 0);
	end = Vector3(End_x, End_y, 0);

	Vector3 cellpos = Vector3(
		start.x * 3,
		0.0f,
		start.y * 3
	) * scalar;
	Vector3 cellsize = Vector3(
		scalar * 2,
		1.0f,
		scalar * 2
	);

	cubeS = new RenderNode(mesh, Vector4(0.0f, 0.0f, 1.0f, 1.0f));
	cubeS->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.5f));
	root->AddChild(cubeS);

	cellpos = Vector3(
		end.x * 3,
		0.0f,
		end.y * 3
	) * scalar;
	cubeE = new RenderNode(mesh, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
	cubeE->SetTransform(Matrix4::Translation(cellpos + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.5f));
	root->AddChild(cubeE);

	this->SetRender(root);
}