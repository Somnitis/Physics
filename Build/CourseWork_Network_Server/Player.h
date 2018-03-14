#pragma once
#include <nclgl\Vector3.h>
#include <vector>

class Player
{
public:
	Player();
	~Player();
	Vector3 UpdatePOS();
	void InitializeMovement();
	inline void SetPlayerSpeed(float speed) { Pspeed = speed; }
	inline void SetAllNodes(std::vector<Vector3> Nodes) { AllNodes = Nodes; }
	inline bool IsMoving() {return moving; }

private:
	std::vector<Vector3> AllNodes;
	Vector3 StartPOS;
	Vector3 EndPOS;
	Vector3 CurrPOS;
	Vector3 NextPOS;
	float Pspeed;
	int NextNode;
	bool moving;
};

Player::Player() : CurrPOS(Vector3(0, 0, 0)), Pspeed(0.05f), NextNode(1)
{
	moving = false;
}

Player::~Player()
{
}

void Player::InitializeMovement() 
{
	NextNode = 1;
	StartPOS = AllNodes[0];
	EndPOS = AllNodes[AllNodes.size() - 1];
	NextPOS = AllNodes[NextNode];
	CurrPOS = StartPOS;
}

Vector3 Player::UpdatePOS()
{
	if ((CurrPOS - EndPOS).Length() < 0.15f) return CurrPOS;
	else
	{
		if ((CurrPOS - NextPOS).Length() < 0.1f) {
			NextNode++;
			NextPOS = AllNodes[NextNode];
			CurrPOS += (NextPOS - CurrPOS).Normalise() * Pspeed;
		}
		else
		{
			CurrPOS += (NextPOS - CurrPOS).Normalise() * Pspeed;
		}
		return CurrPOS;
	}
	return Vector3(-1, -1, -1);
}


