#include <ncltech\Scene.h>
#include <nclgl\Vector4.h>
#include <ncltech\GraphicsPipeline.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>

#include "RenderNodeParticles.h"
#include "CudaCollidingParticles.cuh"

using namespace CommonUtils;

class GPUPoolScene : public Scene
{
public:
	GPUPoolScene(const std::string& friendly_name)
		: Scene(friendly_name)
	{
	}

	virtual ~GPUPoolScene()
	{

	}


	virtual void OnInitializeScene() override
	{
		//<--- SCENE CREATION --->
		//Create Ground
		//this->AddGameObject(BuildCuboidObject("Ground", Vector3(0.0f, -1.0f, 0.0f), Vector3(20.0f, 1.0f, 20.0f), true, 0.0f, true, false, Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

		cudaParticleProg = new CudaCollidingParticles();

		//The dam size (<value> * PARTICLE_RADIUS * 2) must be smaller than the simulation world size!
		cudaParticleProg->InitializeParticleDam(60, 10, 60);

		uint num_particles = cudaParticleProg->GetNumParticles();

		Vector4 colour;
		int col = rand() % 4;
		switch (col) {
		case 0:
			colour = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
			break;
		case 1:
			colour = Vector4(0.0f, 1.0f, 0.0f, 1.0f);
			break;
		case 2:
			colour = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
			break;
		case 3:
			colour = Vector4(1.0f, 1.0f, 0.0f, 1.0f);
			break;
		}


		RenderNodeParticles* rnode = new RenderNodeParticles();
		rnode->SetParticleRadius(PARTICLE_RADIUS);

		rnode->GeneratePositionBuffer(num_particles, NULL);

		rnode->SetColor(colour);

		const float half_grid_world_size = PARTICLE_GRID_SIZE * PARTICLE_GRID_CELL_SIZE * 0.5f;
		rnode->SetTransform(Matrix4::Translation(Vector3(-half_grid_world_size, -half_grid_world_size, -half_grid_world_size)));

		//We don't need any game logic, or model matrices, just a means to render our
		// particles to the screen.. so this is just a wrapper  to our actual
		// vertex buffer that holds each particles world position.
		this->AddGameObject(new GameObject("", rnode, NULL));

		// Floor
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, -1.0f, 0.0f),
			Vector3(9.6f, 1.0f, 9.6f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

		// Wall 1
		GameObject * wall1 = CommonUtils::BuildCuboidObject(
			"Wall1",
			Vector3(10.1f, 2.0f, 0.0f),
			Vector3(0.5f, 4.0f, 9.6f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f));
		this->AddGameObject(wall1);

		// Wall 2
		GameObject * wall2 = CommonUtils::BuildCuboidObject(
			"Wall1",
			Vector3(-10.1f, 2.0f, 0.0f),
			Vector3(0.5f, 4.0f, 9.6f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f));
		this->AddGameObject(wall2);

		// Wall 3
		GameObject * wall3 = CommonUtils::BuildCuboidObject(
			"Wall1",
			Vector3(0.0f, 2.0f, 10.1f),
			Vector3(9.6f, 4.0f, 0.5f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f));
		this->AddGameObject(wall3);

		// Wall 4
		GameObject * wall4 = CommonUtils::BuildCuboidObject(
			"Wall1",
			Vector3(0.0f, 2.0f, -10.1f),
			Vector3(9.6f, 4.0f, 0.5f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f));
		this->AddGameObject(wall4);

		this->AddGameObject(CommonUtils::BuildSphereObject("Interactive_sphere",
			Vector3(1.0f, 5.0f, 2.0f),
			0.5f,									//Radius
			true,									//Has Physics Object
			1.0f / 4.0f,							//Inverse Mass
			true,									//Has Collision Shape
			true,									//Dragable by the user
			CommonUtils::GenColor(0.2f, 0.8f)));	//Color

		this->AddGameObject(CommonUtils::BuildSphereObject("Interactive_sphere2",
			Vector3(-1.0f, 5.0f, -2.0f),
			0.5f,									//Radius
			true,									//Has Physics Object
			1.0f / 4.0f,							//Inverse Mass
			true,									//Has Collision Shape
			true,									//Dragable by the user
			CommonUtils::GenColor(0.2f, 0.8f)));	//Color

		cudaParticleProg->InitializeOpenGLVertexBuffer(rnode->GetGLVertexBuffer());

		Scene::OnInitializeScene();

		
	}

	virtual void OnCleanupScene() override
	{
		Scene::OnCleanupScene();
		delete cudaParticleProg;
	}

	virtual void OnUpdateScene(float dt) override
	{
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "--- Info ---");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "  Example broadphase using cuda thrust library. The thrust library");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "  is the GPU equivalent of the C++ STL and makes things easier ");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "  with vector's, sorting, iterators and array manipulation.");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "  No. Particles: %d", cudaParticleProg->GetNumParticles());


		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J))
		{
				GameObject* ball = CommonUtils::BuildSphereObject("",
					Vector3(GraphicsPipeline::Instance()->GetCamera()->GetPosition() - Vector3(0, 0.5, 0)),				//Position
					0.5f,									//Radius
					true,									//Has Physics Object
					1.0f,									// Inverse Mass = 1 / 1kg mass
					true,									//No Collision Shape Yet
					false,									//Dragable by the user
					Vector4(0.0f,0.2f,0.2f,1.0f));			//Color

				float yaw = GraphicsPipeline::Instance()->GetCamera()->GetYaw();
				float pitch = GraphicsPipeline::Instance()->GetCamera()->GetPitch();


				this->AddGameObject(ball);
				ball->Physics()->SetLinearVelocity(Matrix3::Rotation(pitch, Vector3(50, 0, 0)) * Matrix3::Rotation(yaw, Vector3(0, 50 , 0)) * Vector3(0, 0, -50 ));
				ball->Physics()->SetAngularVelocity(Vector3(-5 , 10 , 0.0f));
		}

		vector<Vector3> positions;
		vector<Vector3> velocities;

		for (std::vector<GameObject*>::iterator it = m_vpObjects.begin(); it != m_vpObjects.end(); ++it) {
			if ((*it)->HasPhysics()) {
				positions.push_back((*it)->Physics()->GetPosition());
				velocities.push_back((*it)->Physics()->GetLinearVelocity());
			}

		}

		cudaParticleProg->UpdateParticles(dt, positions, velocities);

	}


protected:
	CudaCollidingParticles* cudaParticleProg;
};