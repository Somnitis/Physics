#pragma once

#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <nclgl\NCLDebug.h>
#include <ncltech\GameObject.h>
#include <ncltech\CommonMeshes.h>
#include <ncltech\CommonUtils.h>

class TargetScene : public Scene
{
public:
	TargetScene(const std::string& friendly_name);
	virtual ~TargetScene();

	virtual void OnInitializeScene()	 override;
	virtual void OnCleanupScene()		 override;
	virtual void OnUpdateScene(float dt) override;

protected:

	static bool TargetScene::ScoreInc(PhysicsNode* self, PhysicsNode* collidingObject);
	static bool TargetScene::ScoreDec(PhysicsNode* self, PhysicsNode* collidingObject);

	GLuint tex;
	float m_AccumTime;
	GameObject *PowerIND;
	GameObject *ball;
	vector<GameObject*> G_targets;
	vector<GameObject*> B_targets;
	int targets;
	float power;
	bool shooting;
	static int score;
	float scoreTimer;
	static bool scored;
	GLuint bal;
};