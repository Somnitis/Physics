#include "EmptyScene.h"

using namespace CommonUtils;

EmptyScene::EmptyScene(const std::string& friendly_name)
	: Scene(friendly_name)
	, m_AccumTime(0.0f)
{
}

EmptyScene::~EmptyScene()
{

}


void EmptyScene::OnInitializeScene()
{
	//Disable the physics engine (We will be starting this later!)
	PhysicsEngine::Instance()->SetPaused(true);

	//Set the camera position
	GraphicsPipeline::Instance()->GetCamera()->SetPosition(Vector3(15.0f, 10.0f, -15.0f));
	GraphicsPipeline::Instance()->GetCamera()->SetYaw(140.f);
	GraphicsPipeline::Instance()->GetCamera()->SetPitch(-20.f);

	m_AccumTime = 0.0f;

	//Example usage of Log 
	//- The on screen log can be opened from the bottom left though the
	//  main point of this is the ability to output to file easily from anywhere.
	//  So if your having trouble debugging with hundreds of 'cout << vector3()' just
	//  through them into NCLLOG() and look at the 'program_output.txt' later =]
	NCLDebug::Log("This is a log entry - It will printed to the console, on screen log and <project_dir>\program_output.txt");
	//NCLERROR("THIS IS AN ERROR!"); // <- On Debug mode this will also trigger a breakpoint in your code!



	//<--- SCENE CREATION --->
	//Create Ground
	this->AddGameObject(BuildCuboidObject("Ground", Vector3(0.0f, -1.0f, 0.0f), Vector3(20.0f, 1.0f, 20.0f), true, 0.0f, true, false, Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
	this->AddGameObject(BuildCuboidObject("NorthWall", Vector3(0.f, 3.0f, 5.5f), Vector3(5.5f, 3.0f, 1.0f), true, 0.0f, true, false, Vector4(0.2f, 0.5f, 1.0f, .3f)));
	this->AddGameObject(BuildCuboidObject("SouthWall", Vector3(0.f, 3.0f, -5.5f), Vector3(5.5f, 3.0f, 1.0f), true, 0.0f, true, false, Vector4(0.2f, 0.5f, 1.0f, .3f)));
	this->AddGameObject(BuildCuboidObject("EastWall", Vector3(5.5f, 3.f, 0.f), Vector3(1.0f, 3.0f, 5.5f), true, 0.0f, true, false, Vector4(0.2f, 0.5f, 1.0f, .3f)));
	this->AddGameObject(BuildCuboidObject("WestWall", Vector3(-5.5f, 3.f, 0.5f), Vector3(1.0f, 3.0f, 5.5f), true, 0.0f, true, false, Vector4(0.2f, 0.5f, 1.0f, .3f)));
	
	int balls = 7;

	for (int i = 0; i < balls;i++)
	{
		for (int j = 0; j < balls;j++)
		{
			for (int k = 0; k < balls;k++)
			{
				ball = CommonUtils::BuildSphereObject("",
					Vector3(-2.5f+i, 5.f+j, -2.5f+k),				//Position
					0.3f,									//Radius
					true,									//Has Physics Object
					1.0f,									// Inverse Mass = 1 / 1kg mass
					true,									//No Collision Shape Yet
					true,									//Dragable by the user
					CommonUtils::GenColor(0.5f, 1.0f));		//Color

				this->AddGameObject(ball);
			}
		}
	}

	ball = CommonUtils::BuildSphereObject("",
		Vector3(0.f,15.f , 0.f),				//Position
		0.5f,									//Radius
		true,									//Has Physics Object
		1.0f,									// Inverse Mass = 1 / 1kg mass
		true,									//No Collision Shape Yet
		true,									//Dragable by the user
		CommonUtils::GenColor(0.5f, 1.0f));		//Color

	this->AddGameObject(ball);
}

void EmptyScene::OnCleanupScene()
{
	//Just delete all created game objects 
	//  - this is the default command on any Scene instance so we don't really need to override this function here.
	Scene::OnCleanupScene();
}

void EmptyScene::OnUpdateScene(float dt)
{
	m_AccumTime += dt;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J)) {
		ball = CommonUtils::BuildSphereObject("",
			Vector3(GraphicsPipeline::Instance()->GetCamera()->GetPosition() - Vector3(0, 0.5, 0)),				//Position
			0.5f,									//Radius
			true,									//Has Physics Object
			1.0f,									// Inverse Mass = 1 / 1kg mass
			true,									//No Collision Shape Yet
			false,									//Dragable by the user
			CommonUtils::GenColor(0.5f, 1.0f));		//Color

		float yaw = DegToRad(GraphicsPipeline::Instance()->GetCamera()->GetYaw());
		float pitch = GraphicsPipeline::Instance()->GetCamera()->GetPitch();


		this->AddGameObject(ball);
		ball->Physics()->SetLinearVelocity(Vector3(-30 * sin(yaw), 2.0f + pitch *0.8f, -30 * cos(yaw)));
		ball->Physics()->SetAngularVelocity(Vector3(-5.0f, 10.0f, 0.0f));

	}

}