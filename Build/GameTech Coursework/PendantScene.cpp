#include "PendantScene.h"

#include <nclgl\Vector4.h>
#include <ncltech\GraphicsPipeline.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\SceneManager.h>
#include <nclgl\NCLDebug.h>


using namespace CommonUtils;

PendantScene::PendantScene(const std::string& friendly_name)
	: Scene(friendly_name)
	, m_AccumTime(0.0f)
{
}

PendantScene::~PendantScene()
{

}

void PendantScene::OnInitializeScene()
{
	uint columns = 22;
	uint rows = 15;
	string s;
	size = 1;

	for (uint i = 0; i < columns;i++) {
		for (uint j = 0; j < rows;j++) {
			if (j== rows -1)
				balls = CommonUtils::BuildSphereObject("NET" + to_string(i) + "x" + to_string(j),
					Vector3(.1f*i*size, .1f*j*size, -8.2f), 0.03f*size, true, 0.0f*size, true, true, CommonUtils::GenColor(0.5f, 1.0f));
			else
			balls = CommonUtils::BuildSphereObject("NET" + to_string(i) + "x" + to_string(j),
				Vector3(.1f*i*size, .1f*j*size, -8.2f), 0.03f*size, true, 0.1f*size, true, true, CommonUtils::GenColor(0.5f, 1.0f));
			this->AddGameObject(balls);
		}
	}



	DistanceConstraint* constraint;

	for (uint i = 0; i < columns ;i++) {
		for (uint j = 0; j < rows ;j++)
		{
		//	GameObject* b = this->FindGameObject("NET" + to_string(i) + "x" + to_string(j));
			if (i < columns - 1 && j < rows - 1)
			{
				constraint = new DistanceConstraint(
					FindGameObject("NET" + to_string(i) + "x" + to_string(j))->Physics(),
					FindGameObject("NET" + to_string(i + 1) + "x" + to_string(j))->Physics(),					//Physics Object B
					FindGameObject("NET" + to_string(i) + "x" + to_string(j))->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
					FindGameObject("NET" + to_string(i + 1) + "x" + to_string(j))->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
				PhysicsEngine::Instance()->AddConstraint(constraint);

				constraint = new DistanceConstraint(
					FindGameObject("NET" + to_string(i) + "x" + to_string(j))->Physics(),
					FindGameObject("NET" + to_string(i) + "x" + to_string(j + 1))->Physics(),					//Physics Object B
					FindGameObject("NET" + to_string(i) + "x" + to_string(j))->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
					FindGameObject("NET" + to_string(i) + "x" + to_string(j + 1))->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
				PhysicsEngine::Instance()->AddConstraint(constraint);
			}
			if (i < columns - 1 && j == rows -1) {
				constraint = new DistanceConstraint(
					FindGameObject("NET" + to_string(i) + "x" + to_string(j))->Physics(),
					FindGameObject("NET" + to_string(i + 1) + "x" + to_string(j))->Physics(),					//Physics Object B
					FindGameObject("NET" + to_string(i) + "x" + to_string(j))->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
					FindGameObject("NET" + to_string(i + 1) + "x" + to_string(j))->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
				PhysicsEngine::Instance()->AddConstraint(constraint);
			}
			if (i == columns -1 && j < rows -1) {
				constraint = new DistanceConstraint(
					FindGameObject("NET" + to_string(i) + "x" + to_string(j))->Physics(),
					FindGameObject("NET" + to_string(i) + "x" + to_string(j + 1))->Physics(),					//Physics Object B
					FindGameObject("NET" + to_string(i) + "x" + to_string(j))->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
					FindGameObject("NET" + to_string(i) + "x" + to_string(j + 1))->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
				PhysicsEngine::Instance()->AddConstraint(constraint);
			}
			//if (j < rows-1)
			//{
			//	net.push_back(this->FindGameObject("NET" + to_string(i) + "x" + to_string(j)));
			//}
			//for (uint n = 0; n < net.size();n++) {
			//	constraint = new DistanceConstraint(
			//	b->Physics(),					//Physics Object A
			//	FindGameObject("NET" + to_string(i) + "x" + to_string(j))->Physics(),					//Physics Object B
			//	b->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
			//	FindGameObject("NET" + to_string(i) + "x" + to_string(j))->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
			//	PhysicsEngine::Instance()->AddConstraint(constraint);
			//}
		}
	}
	//Create Hanging Ball
	handle = CommonUtils::BuildSphereObject("",
		Vector3(.1f*(columns + 1)*size, .1f*(rows - 1)*size, -8.2f),				//Position
		0.03f*size,								//Radius
		true,									//Has Physics Object
		0.0f,									//Infinite Mass
		false,									//No Collision Shape Yet
		true,									//Dragable by the user
		CommonUtils::GenColor(0.45f, 0.5f));	//Color
												//Create Hanging Ball
	handle1 = CommonUtils::BuildSphereObject("",
		Vector3(-.1f*size, .1f*(rows - 1)*size, -8.2f),				//Position
		0.03f*size,								//Radius
		true,									//Has Physics Object
		0.0f,									//Infinite Mass
		false,									//No Collision Shape Yet
		true,									//Dragable by the user
		CommonUtils::GenColor(0.45f, 0.5f));	//Color

	this->AddGameObject(handle);
	this->AddGameObject(handle1);

	constraint = new DistanceConstraint(
		FindGameObject("NET" + to_string(0) + "x" + to_string(rows - 1))->Physics(),
		handle1->Physics(),					//Physics Object B
		FindGameObject("NET" + to_string(0) + "x" + to_string(rows - 1))->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
		handle1->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
	PhysicsEngine::Instance()->AddConstraint(constraint);

	constraint = new DistanceConstraint(
		FindGameObject("NET" + to_string(columns-1) + "x" + to_string(rows-1))->Physics(),
		handle->Physics(),					//Physics Object B
		FindGameObject("NET" + to_string(columns-1) + "x" + to_string(rows-1))->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
		handle->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
	PhysicsEngine::Instance()->AddConstraint(constraint);
}

void PendantScene::OnCleanupScene()
{
	//Just delete all created game objects 
	//  - this is the default command on any Scene instance so we don't really need to override this function here.
	Scene::OnCleanupScene();
}

void PendantScene::OnUpdateScene(float dt)
{
	m_AccumTime += dt;
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J)) {
		ball = CommonUtils::BuildSphereObject("",
			Vector3(GraphicsPipeline::Instance()->GetCamera()->GetPosition()-Vector3(0,0.5,0)),				//Position
			0.2f*size,								//Radius
			true,									//Has Physics Object
			1.0f,									// Inverse Mass = 1 / 1kg mass
			true,									//No Collision Shape Yet
			false,									//Dragable by the user
			CommonUtils::GenColor(0.5f, 1.0f));		//Color

		float yaw = GraphicsPipeline::Instance()->GetCamera()->GetYaw();
		float pitch = GraphicsPipeline::Instance()->GetCamera()->GetPitch();


		this->AddGameObject(ball);
		ball->Physics()->SetLinearVelocity(Matrix3::Rotation(pitch, Vector3(8, 0, 0)) * Matrix3::Rotation(yaw, Vector3(0, 8, 0)) * Vector3(0, 0, -8));
	}

	uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();
}