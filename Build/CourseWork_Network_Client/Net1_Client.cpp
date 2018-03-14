/******************************************************************************
Class: Net1_Client
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

:README:
- In order to run this demo, we also need to run "Tuts_Network_Server" at the same time.
- To do this:-
	1. right click on the entire solution (top of the solution exporerer) and go to properties
	2. Go to Common Properties -> Statup Project
	3. Select Multiple Statup Projects
	4. Select 'Start with Debugging' for both "Tuts_Network_Client" and "Tuts_Network_Server"

- Now when you run the program it will build and run both projects at the same time. =]
- You can also optionally spawn more instances by right clicking on the specific project
  and going to Debug->Start New Instance.




This demo scene will demonstrate a very simple network example, with a single server
and multiple clients. The client will attempt to connect to the server, and say "Hellooo!" 
if it successfully connects. The server, will continually broadcast a packet containing a 
Vector3 position to all connected clients informing them where to place the server's player.

This designed as an example of how to setup networked communication between clients, it is
by no means the optimal way of handling a networked game (sending position updates at xhz).
If your interested in this sort of thing, I highly recommend finding a good book or an
online tutorial as there are many many different ways of handling networked game updates
all with varying pitfalls and benefits. In general, the problem always comes down to the
fact that sending updates for every game object 60+ frames a second is just not possible,
so sacrifices and approximations MUST be made. These approximations do result in a sub-optimal
solution however, so most work on networking (that I have found atleast) is on building
a network bespoke communication system that sends the minimal amount of data needed to
produce satisfactory results on the networked peers.


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::: IF YOUR BORED! :::
::::::::::::::::::::::
	1. Try setting up both the server and client within the same Scene (disabling collisions
	on the objects as they now share the same physics engine). This way we can clearly
	see the affect of packet-loss and latency on the network. There is a program called "Clumsy"
	which is found within the root directory of this framework that allows you to inject
	latency/packet loss etc on network. Try messing around with various latency/packet-loss
	values.

	2. Packet Loss
		This causes the object to jump in large (and VERY noticable) gaps from one position to 
		another.

	   A good place to start in compensating for this is to build a buffer and store the
	   last x update packets, now if we miss a packet it isn't too bad as the likelyhood is
	   that by the time we need that position update, we will already have the next position
	   packet which we can use to interpolate that missing data from. The number of packets we
	   will need to backup will be relative to the amount of expected packet loss. This method
	   will also insert additional 'buffer' latency to our system, so we better not make it wait
	   too long.

	3. Latency
	   There is no easy way of solving this, and will have all felt it's punishing effects
	   on all networked games. The way most games attempt to hide any latency is by actually
	   running different games on different machines, these will react instantly to your actions
	   such as shooting which the server will eventually process at some point and tell you if you
	   have hit anything. This makes it appear (client side) like have no latency at all as you
	   moving instantly when you hit forward and shoot when you hit shoot, though this is all smoke
	   and mirrors and the server is still doing all the hard stuff (except now it has to take into account
	   the fact that you shot at time - latency time).

	   This smoke and mirrors approach also leads into another major issue, which is what happens when
	   the instances are desyncrhonised. If player 1 shoots and and player 2 moves at the same time, does
	   player 1 hit player 2? On player 1's screen he/she does, but on player 2's screen he/she gets
	   hit. This leads to issues which the server has to decipher on it's own, this will also happen
	   alot with generic physical elements which will ocasional 'snap' back to it's actual position on 
	   the servers game simulation. This methodology is known as "Dead Reckoning".

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


*//////////////////////////////////////////////////////////////////////////////

#include "Net1_Client.h"
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <nclgl\NCLDebug.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonUtils.h>
#include <nclgl\OBJMesh.h>
#include <deque>
#include <list>
#include <unordered_map>
#include "MazeRenderer.h"
#include "SearchAStar.h"
#include "Lexer.h"

SearchAStar*	search_as;
MazeRenderer*	maze;
Mesh* wallmesh;
bool MazeBuild = true;
bool PlayerActive = false;
int grid_size;
vector<bool> allEdges;
int StartMoving = 0;
bool DrawAstar = false;
string pack;
int mazedim;
int Sx;
int Sy;
int Ex;
int Ey;
Vector3 PlayerPos;
vector<Vector3> AstarPos;
float scalar = 1.f / (mazedim * 3 - 1);
Vector3 cellsize = Vector3(
	scalar * 2, 1.0f, scalar * 2);

enum PacketID {
	CREATEMAZE = 10,
	MAZEGEN, MAZEREND,
	ASKASTAR, ASTAR,
	MOVEPOS, STARTMOVING,
	PRINTASTAR,
	POS
};



const Vector3 status_color3 = Vector3(1.0f, 0.6f, 0.6f);
const Vector4 status_color = Vector4(status_color3.x, status_color3.y, status_color3.z, 1.0f);

Net1_Client::Net1_Client(const std::string& friendly_name)
	: Scene(friendly_name)
	, serverConnection(NULL)
	, player(NULL)
{
}



int packetID = 0;
MazeRenderer* MazeRen;

void Net1_Client::OnInitializeScene()
{
	packetID=0;
	//Initialize Client Network
	if (network.Initialize(0))
	{
		NCLDebug::Log("Network: Initialized!");

		//Attempt to connect to the server on localhost:1234
		serverConnection = network.ConnectPeer(127, 0, 0, 1, 1234);
		NCLDebug::Log("Network: Attempting to connect to server.");
		packetID = 0;

	}


	wallmesh = new OBJMesh(MESHDIR"cube.obj");

	GLuint whitetex;
	glGenTextures(1, &whitetex);
	glBindTexture(GL_TEXTURE_2D, whitetex);
	unsigned int pixel = 0xFFFFFFFF;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
	glBindTexture(GL_TEXTURE_2D, 0);

	wallmesh->SetTexture(whitetex);


	player = CommonUtils::BuildCuboidObject(
		"Player",
		Vector3(0.0f, 1.0f, 0.0f),
		Vector3(0.1f, 0.4f, 0.1f),
		true,
		0.0f,
		false,
		false,
		Vector4(0.0f, 1.0f, .0f, 1.0f));

	//Generate Simple Scene with a box that can be updated upon recieving server packets

	
}

Net1_Client::~Net1_Client()
{
	SAFE_DELETE(wallmesh);
}

void Net1_Client::OnCleanupScene()
{
	Scene::OnCleanupScene();
	player = NULL; // Deleted in above function

	//Send one final packet telling the server we are disconnecting
	// - We are not waiting to resend this, so if it fails to arrive
	//   the server will have to wait until we time out naturally
	enet_peer_disconnect_now(serverConnection, 0);

	//Release network and all associated data/peer connections
	network.Release();
	serverConnection = NULL;
}

void  Net1_Client::GenerateNewMaze()
{
	this->DeleteAllGameObjects(); //Cleanup old mazes


	//The maze is returned in a [0,0,0] - [1,1,1] cube (with edge walls outside) regardless of grid_size,
	// so we need to scale it to whatever size we want

	Matrix4 maze_scalar = Matrix4::Scale(Vector3(15.f, 15.0f / float(grid_size), 15.f)) * Matrix4::Translation(Vector3(-0.5f, 0.f, -0.5f));
	maze = new MazeRenderer(grid_size, allEdges, wallmesh);
	maze->Render()->SetTransform(Matrix4::Translation(Vector3(1, 0, 1)) * maze_scalar);
	this->AddGameObject(maze);
	maze->StartEndPOS(Sx, Sy, Ex, Ey);


	//Create Ground (..we still have some common ground to work off)
	GameObject* ground = CommonUtils::BuildCuboidObject(
		"Ground",
		Vector3(0.0f, -1.0f, 0.0f),
		Vector3(20.0f, 1.0f, 20.0f),
		false,
		0.0f,
		false,
		false,
		Vector4(0.9f, 0.9f, 0.9f, 1.0f));
	this->AddGameObject(ground);
}


void Net1_Client::OnUpdateScene(float dt)
{
	Scene::OnUpdateScene(dt);
	

	//Update Network
	auto callback = std::bind(
		&Net1_Client::ProcessNetworkEvent,	// Function to call
		this,								// Associated class instance
		std::placeholders::_1);				// Where to place the first parameter
	network.ServiceNetwork(dt, callback);

	//Build maze
	if (!MazeBuild) {
		GenerateNewMaze();
		
		MazeBuild = true;
	}
	if (DrawAstar) {
		//maze->DrawWay(AstarPos, 0.5f);
	}
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_B)) {
		Ex = rand() % mazedim;
		Ey = rand() % mazedim;
		Sx = PlayerPos.x;
		Sy = PlayerPos.z;
		if (Sx == Ex && Sy == Ey) 
		{
			Ex = rand() % mazedim;
			Ey = rand() % mazedim;
		}
		string send = to_string(MOVEPOS) + " " + to_string(Sx) + " " + to_string(Sy) + " " + to_string(Ex) + " " + to_string(Ey);
		ENetPacket* EndPos = enet_packet_create(send.c_str(), strlen(send.c_str()), 0);
		enet_peer_send(serverConnection, 0, EndPos);
		maze->StartEndPOS(Sx, Sy, Ex, Ey);
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_M)) 
	{	
		if(StartMoving == 0) StartMoving = 1;
		else StartMoving = 0;
		string send = to_string(STARTMOVING) + " " + to_string(StartMoving);
		ENetPacket* MOVE = enet_packet_create(send.c_str(), strlen(send.c_str()), 0);
		enet_peer_send(serverConnection, 0, MOVE);
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_O) && !PlayerActive)
	{
		PlayerActive = true;
		this->AddGameObject(player);
		player->Render()->SetTransform(Matrix4::Translation(Vector3(Sx * 3, 1.0f, Sy * 3) * scalar + cellsize * 0.5f) * Matrix4::Scale(cellsize * 0.5f));
		float scale = 7.5f / (3 * mazedim - 1);
		
		player->Physics()->SetPosition(Vector3((6 * Sx + 15 - 3 * mazedim)*scale, 0.45, (6 * Sy +15 - 3 * mazedim)*scale));
	}

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_T)) {
		srand(rand());
		float mazeden;
		cout << "Please give the size of the maze.\n";
		cin >> mazedim;
		cout << "Please give the density of the maze.\n";
		cin >> mazeden;
		Sx = rand() % mazedim;
		Sy = rand() % mazedim;
		Ex = rand() % mazedim;
		Ey = rand() % mazedim;

		string send = to_string(MAZEGEN) + " " + to_string(mazedim) + " " + to_string(mazeden)
			+ " " + to_string(Sx) + " " + to_string(Sy) + " " + to_string(Ex) + " " + to_string(Ey);
		ENetPacket* Restart = enet_packet_create(send.c_str(), strlen(send.c_str()), 0);
		enet_peer_send(serverConnection, 0, Restart);
	}

	//if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_R)) {
	//	srand(rand());
	//	float mazeden;
	//	mazeden = ((rand() % 50) / 100) +0.2f;
	//	Sx = rand() % mazedim;
	//	Sy = rand() % mazedim;
	//	Ex = rand() % mazedim;
	//	Ey = rand() % mazedim;
	//	string send = to_string(MAZEGEN) + " " + to_string(mazedim) + " " + to_string(mazeden)
	//		+ " " + to_string(Sx) + " " + to_string(Sy) + " " + to_string(Ex) + " " + to_string(Ey);
	//	ENetPacket* Restart1 = enet_packet_create(send.c_str(), strlen(send.c_str()), 0);
	//	enet_peer_send(serverConnection, 0, Restart1);
	//}



	//Add Debug Information to screen
	uint8_t ip1 = serverConnection->address.host & 0xFF;
	uint8_t ip2 = (serverConnection->address.host >> 8) & 0xFF;
	uint8_t ip3 = (serverConnection->address.host >> 16) & 0xFF;
	uint8_t ip4 = (serverConnection->address.host >> 24) & 0xFF;

	if (PlayerActive)
	NCLDebug::DrawTextWs(player->Physics()->GetPosition() + Vector3(0.f, 0.6f, 0.f), STATUS_TEXT_SIZE, TEXTALIGN_CENTRE, Vector4(0.f, 0.1f, 0.f, 1.f),
		"Somnitis");

	
	NCLDebug::AddStatusEntry(status_color, "Network Traffic");
	NCLDebug::AddStatusEntry(status_color, "    Incoming: %5.2fKbps", network.m_IncomingKb);
	NCLDebug::AddStatusEntry(status_color, "    Outgoing: %5.2fKbps", network.m_OutgoingKb);
	
}

void Net1_Client::ProcessNetworkEvent(const ENetEvent& evnt)
{
	switch (evnt.type)
	{
		//New connection request or an existing peer accepted our connection request
	case ENET_EVENT_TYPE_CONNECT:
	{
		if (evnt.peer == serverConnection)
		{
			NCLDebug::Log(status_color3, "Network: Successfully connected to server!");
		}
	}
	break;


	//Server has sent us a new packet
	case ENET_EVENT_TYPE_RECEIVE:
	{
		std::ostringstream convert;
		for (int a = 0; a < evnt.packet->dataLength; a++) {
			convert << (enet_uint8)evnt.packet->data[a];
		}

		string pack = convert.str();
		vector<string> words;
		words = split(pack, ' ');
		packetID = stoi(words[0]);

		switch (packetID)
		{
		case CREATEMAZE: {
			srand(rand());
			float mazeden;
			cout << "Please give the size of the maze.\n";
			cin >> mazedim;
			cout << "Please give the density of the maze.\n";
			cin >> mazeden;
			Sx = rand() % mazedim;
			Sy = rand() % mazedim;
			Ex = rand() % mazedim;
			Ey = rand() % mazedim;

			string send = to_string(MAZEGEN) + " " + to_string(mazedim) + " " + to_string(mazeden)
				+ " " + to_string(Sx) + " " + to_string(Sy) + " " + to_string(Ex) + " " + to_string(Ey);
			ENetPacket* MazeSpcs = enet_packet_create(send.c_str(), strlen(send.c_str()), 0);
			enet_peer_send(serverConnection, 0, MazeSpcs);
			
			break;
		}
		case MAZEREND: {
			grid_size = stoi(words[1]);
			for (int i = 2; i < words.size();i++)
			{
				allEdges.push_back(stoi(words[i]));
				cout << stoi(words[i]) << "   " ;
			}
			cout << "\n";
			MazeBuild = false;
			string send = to_string(ASKASTAR) + " " + to_string(mazedim);
			ENetPacket* ReqAstar = enet_packet_create(send.c_str(), strlen(send.c_str()), 0);
			enet_peer_send(serverConnection, 0, ReqAstar);
			break;
		}
		case ASTAR: {
			AstarPos.clear();
			Vector3 pos;
			cout << words.size();
			for (int i = 1; i < words.size();)
			{
				pos.x = stof(words[i]);
				pos.y = stof(words[i + 1]);
				pos.z = stof(words[i + 2]);
				AstarPos.push_back(pos);
				i += 3;
				cout << i << "  ";
			}
			DrawAstar = true;
			cout <<"\n" << AstarPos.data() << "\n";
		}
		case POS:
		{
			Vector3 pos;
			float scale = 7.5f / (3 * mazedim -1);
			PlayerPos.x = stof(words[1]);
			PlayerPos.y = stof(words[2]);
			PlayerPos.z = stof(words[3]);
			pos.x = (6 * PlayerPos.x + 26.5 - 3 * mazedim)*scale;
			pos.y = PlayerPos.y;
			pos.z = (6 * PlayerPos.z + 26.5 - 3 * mazedim)*scale;
			player->Physics()->SetPosition(-(pos + cellsize * 0.5f) * (cellsize * 0.5f));
			break;
		}
		default:
			NCLERROR("Recieved Invalid Network Packet!");
			break;

		}
		break;
	}

	//Server has disconnected
	case ENET_EVENT_TYPE_DISCONNECT:
	{
		NCLDebug::Log(status_color3, "Network: Server has disconnected!");
	}
	break;
	}

}