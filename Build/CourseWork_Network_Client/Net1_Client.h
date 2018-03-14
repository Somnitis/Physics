
#pragma once

#include <ncltech\Scene.h>
#include <ncltech\NetworkBase.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>



//Basic Network Example



class Net1_Client : public Scene
{
public:
	Net1_Client(const std::string& friendly_name);

	virtual void OnInitializeScene() override;
	virtual void OnCleanupScene() override;
	virtual void OnUpdateScene(float dt) override;

	virtual ~Net1_Client();


	void ProcessNetworkEvent(const ENetEvent& evnt);
	void GenerateNewMaze();

protected:
	GameObject* player;
	GameObject*		floor;
	NetworkBase network;
	ENetPeer*	serverConnection;


};




