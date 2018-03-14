
/******************************************************************************
Class: Net1_Client
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

:README:
- In order to run this demo, we also need to run "Tuts_Network_Client" at the same time.
- To do this:-
	1. right click on the entire solution (top of the solution exporerer) and go to properties
	2. Go to Common Properties -> Statup Project
	3. Select Multiple Statup Projects
	4. Select 'Start with Debugging' for both "Tuts_Network_Client" and "Tuts_Network_Server"

- Now when you run the program it will build and run both projects at the same time. =]
- You can also optionally spawn more instances by right clicking on the specific project
and going to Debug->Start New Instance.




FOR MORE NETWORKING INFORMATION SEE "Tuts_Network_Client -> Net1_Client.h"



		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once


#include "server.h"

using namespace std;
//void CheckPacketID(int packetID, vector<string> words, ENetEvent evnt);
//Needed to get computer adapter IPv4 addresses via windows
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")


int onExit(int exitcode)
{
	server.Release();
	system("pause");
	exit(exitcode);
}


int main(int arcg, char** argv)
{
	string pack;
	int packetID = 0;
	int MazeSize = 0;
	float MazeDensity = 0;

	player = new Player();

	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occurred while initializing ENet.\n");
		return EXIT_FAILURE;
	}

	//Initialize Server on Port 1234, with a possible 32 clients connected at any time
	if (!server.Initialize(SERVER_PORT, 32))
	{
		fprintf(stderr, "An error occurred while trying to create an ENet server host.\n");
		onExit(EXIT_FAILURE);
	}

	printf("Server Initiated\n");

	search_as = new SearchAStar();


	timer.GetTimedMS();
	while (true)
	{

		packetID = 0;
		float dt = timer.GetTimedMS() * 0.001f;
		accum_time += dt;
		rotation += 0.5f * PI * dt;

		//Handle All Incoming Packets and Send any enqued packets
		server.ServiceNetwork(dt, [&](const ENetEvent& evnt)
		{
			switch (evnt.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				printf("- New Client Connected\n");
				// if it's the 1st client that gets connected we will ask him to give maze specs,
				//  else we will just send the maze we have already created
				if (FirstConnection) {
					FirstConnection = false;
					string send = to_string(CREATEMAZE) + " ";
					ENetPacket* MazeRend = enet_packet_create(send.c_str(), strlen(send.c_str()), 0);
					enet_host_broadcast(server.m_pNetwork, 0, MazeRend);
				}
				//else {
				//	//SendMaze();
				////	MazeRen.size = NewM.size;
				////	MazeRen.allEdges = NewM.allEdges;
				//	ENetPacket* MazeRend = enet_packet_create(&MazeRen, sizeof(MazeRen), 0);
				//	enet_host_broadcast(server.m_pNetwork, 0, MazeRend);
				//}
				break;

			case ENET_EVENT_TYPE_RECEIVE: {

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
				case MAZEGEN: {
					string walls;
					MazeSize = stoi(words[1]);
					MazeDensity = stof(words[2]);
					Maze = new MazeGenerator();
					Maze->Generate(MazeSize, MazeDensity);

					Sx = stof(words[3]);
					Sy = stof(words[4]);
					Maze->GetNodeFromPOS(Vector3(Sx, Sy, 0.0f));
					Ex = stoi(words[5]);
					Ey = stoi(words[6]);
					Maze->GetNodeFromPOS(Vector3(Ex, Ey, 0.0f));
					UpdateAStarPreset();

					player->SetAllNodes(Maze->GetV3POSFromNodes(search_as->GetFinalPath()));
					player->InitializeMovement();

					vector<int> wallsVector = Maze->CreateWalls();
					for (int i = 0; i < wallsVector.size();i++)
					{
						walls += to_string(wallsVector[i]) + " ";
						
					}
					cout << walls << "\n";
					//cin >> packetID;
					string send = to_string(MAZEREND) + " " + to_string(MazeSize) + " " + walls;
					ENetPacket* MazeRend = enet_packet_create(send.c_str(), strlen(send.c_str()), 0);
					enet_host_broadcast(server.m_pNetwork, 0, MazeRend);

					break;
				}
				case ASKASTAR: {
					string StarPos;
					vector<float> AstarPOS;
					AstarPOS = Maze->GetPOSFromNodes(search_as->GetFinalPath());
					cout << search_as->GetFinalPath().size();
					for (int i =0; i < search_as->GetFinalPath().size()*3;)
					{
						StarPos += to_string(AstarPOS[i]) + " " + to_string(AstarPOS[i+1]) + " " + to_string(AstarPOS[i+2]) + " ";
						i += 3;
					}
					cout << StarPos << '\n';
					string send = to_string(ASTAR) + " " + StarPos;
					ENetPacket* Astar = enet_packet_create(send.c_str(), strlen(send.c_str()), 0);
					enet_host_broadcast(server.m_pNetwork, 0, Astar);
					break;
				}
				case MOVEPOS: {
					Sx = stoi(words[1]);
					Sy = stoi(words[2]);
					Ex = stoi(words[3]);
					Ey = stoi(words[4]);
					UpdateAStarPreset();
					string StarTravelPOS;
					vector<float> AStarGridPOS;
					AStarGridPOS = Maze->GetPOSFromNodes(search_as->GetFinalPath());
					for (int i = 0; i < search_as->GetFinalPath().size() * 3;)
					{
						StarTravelPOS += to_string(AStarGridPOS[i]) + " " + to_string(AStarGridPOS[i + 1]) + " " + to_string(AStarGridPOS[i + 2]) + " ";
						i += 3;
					}
					player->SetAllNodes(Maze->GetV3POSFromNodes(search_as->GetFinalPath()));
					player->InitializeMovement();
					string send = to_string(ASTAR) + " " + StarTravelPOS;
					ENetPacket* AstarDraw = enet_packet_create(send.c_str(), strlen(send.c_str()), 0);
					enet_host_broadcast(server.m_pNetwork, 0, AstarDraw);
					break;
				}
				case STARTMOVING:
					StartMoving = stoi(words[1]);
					break;
			}
				//CheckPacketID(packetID, words, evnt);
				printf("\t Client %d says: %s\n", evnt.peer->incomingPeerID, evnt.packet->data);
				enet_packet_destroy(evnt.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
				printf("- Client %d has disconnected.\n", evnt.peer->incomingPeerID);
				break;
			}
		


		
		});
		cout << "    Incoming: " << to_string(server.m_IncomingKb) << "f KB/s\n";
		cout << "    Outgoing: " << to_string(server.m_OutgoingKb) << "f KB/s\n";
		//Broadcast update packet to all connected clients at a rate of UPDATE_TIMESTEP updates per second
		if (accum_time >= UPDATE_TIMESTEP && StartMoving == 1)
		{
			accum_time = 0.0f;
			Vector3 pos = player->UpdatePOS();
			string send = to_string(ASTAR) + " " + to_string(pos.x) + " " + to_string(-1) + " " + to_string(pos.y);
			ENetPacket* PlayerPos = enet_packet_create(send.c_str(), strlen(send.c_str()), 0);
			enet_host_broadcast(server.m_pNetwork, 0, PlayerPos);
		}

		Sleep(0);
	}

	system("pause");
	server.Release();
}


//void CheckPacketID(int packetID, vector<string> words, ENetEvent evnt) {
//	string pack;
//	switch (packetID)
//	{
//	case MAZEGEN: {
//		string walls;
//		MazeSize = stoi(words[1]);
//		MazeDensity = stof(words[2]);
//		Maze = new MazeGenerator();
//		Maze->Generate(MazeSize, MazeDensity);
//		vector<int> WallsUp;
//		WallsUp = Maze->CreateWalls();
//		for (int i = 0; i < Maze->GetNumWalls();i++)
//		{
//			walls += to_string(WallsUp[i]) + " ";
//		}
//		string send = to_string(MAZEREND) + " " + to_string(MazeSize) + " " + walls;
//		ENetPacket* MazeRend = enet_packet_create(send.c_str(), strlen(send.c_str()), 0);
//		enet_host_broadcast(server.m_pNetwork, 0, MazeRend);
//
//		break;
//		}
//	}
//}

//vector<string> GetPacketElements(string packet) {
//	vector<string> words;
//	int i = 0;
//	int chars = 0;
//	stringstream pack;
//	vector<int> spaces;
//
//	pack << packet;
//	while (pack.good)
//	{
//		packetData[i] = pack.get();
//		spaces.push_back(packet.find(" "));
//		chars++;
//	}
//	for (int i = 0; i <= spaces.size();i++) {
//
//	}
//}


//Yay Win32 code >.>
//  - Grabs a list of all network adapters on the computer and prints out all IPv4 addresses associated with them.
void Win32_PrintAllAdapterIPAddresses()
{
	//Initially allocate 5KB of memory to store all adapter info
	ULONG outBufLen = 5000;
	

	IP_ADAPTER_INFO* pAdapters = NULL;
	DWORD status = ERROR_BUFFER_OVERFLOW;

	//Keep attempting to fit all adapter info inside our buffer, allocating more memory if needed
	// Note: Will exit after 5 failed attempts, or not enough memory. Lets pray it never comes to this!
	for (int i = 0; i < 5 && (status == ERROR_BUFFER_OVERFLOW); i++)
	{
		pAdapters = (IP_ADAPTER_INFO *)malloc(outBufLen);
		if (pAdapters != NULL) {

			//Get Network Adapter Info
			status = GetAdaptersInfo(pAdapters, &outBufLen);

			// Increase memory pool if needed
			if (status == ERROR_BUFFER_OVERFLOW) {
				free(pAdapters);
				pAdapters = NULL;
			}
			else {
				break;
			}
		}
	}

	
	if (pAdapters != NULL)
	{
		//Iterate through all Network Adapters, and print all IPv4 addresses associated with them to the console
		// - Adapters here are stored as a linked list termenated with a NULL next-pointer
		IP_ADAPTER_INFO* cAdapter = &pAdapters[0];
		while (cAdapter != NULL)
		{
			IP_ADDR_STRING* cIpAddress = &cAdapter->IpAddressList;
			while (cIpAddress != NULL)
			{
				printf("\t - Listening for connections on %s:%u\n", cIpAddress->IpAddress.String, SERVER_PORT);
				cIpAddress = cIpAddress->Next;
			}
			cAdapter = cAdapter->Next;
		}

		free(pAdapters);
	}
	
}


void UpdateAStarPreset()
{
	float weightingG, weightingH;
	//Equal weighting
	weightingG = 1.0f;
	weightingH = 1.0f;
	search_as->SetWeightings(weightingG, weightingH);

	GraphNode* start = Maze->GetNodeFromPOS(Vector3(Sx, Sy,0));
	GraphNode* end = Maze->GetNodeFromPOS(Vector3(Ex, Ey, 0));
	search_as->FindBestPath(start, end);
}