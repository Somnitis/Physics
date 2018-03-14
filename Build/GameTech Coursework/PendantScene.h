#pragma once

#include <ncltech\Scene.h>

class PendantScene : public Scene
{
public:
	PendantScene(const std::string& friendly_name);
	virtual ~PendantScene();

	virtual void OnInitializeScene()	 override;
	virtual void OnCleanupScene()		 override;
	virtual void OnUpdateScene(float dt) override;

protected:
	float m_AccumTime;
	GameObject *m_pPlayer;
	GameObject *handle, *handle1, *ball;
	GameObject* balls;
	vector<GameObject*> net;
	float size;
};