#pragma once
#include <enet\enet.h>
#include <nclgl\GameTimer.h>
#include <nclgl\Vector3.h>
#include <nclgl\common.h>
#include <ncltech\NetworkBase.h>
#include "MazeGenerator.h"
#include "SearchAStar.h"
#include "lexer.h"  // by Evan Teran
#include "Player.h"

#define PLAYERS (20)
#define SERVER_PORT 1234
#define UPDATE_TIMESTEP (1.0f / 30.0f) //send 30 position updates per second

enum PacketID {
	CREATEMAZE = 10,
	MAZEGEN, MAZEREND,
	ASKASTAR, ASTAR,
	MOVEPOS, STARTMOVING,
	PRINTASTAR,
	PLAYERPOS
};

char* packetData;
int packetID;
NetworkBase server;
GameTimer timer;
float accum_time = 0.0f;
float rotation = 0.0f;
bool FirstConnection = true;

int MazeSize = 0;
int MazeDensity = 0;
MazeGenerator* Maze;
SearchAStar* search_as;
Player* player;
bool StartMoving;

float Sx;
float Sy;
float Ex;
float Ey;


void UpdateAStarPreset();
void Win32_PrintAllAdapterIPAddresses();