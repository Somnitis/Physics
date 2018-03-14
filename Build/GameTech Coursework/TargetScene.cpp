#include "TargetScene.h"

#include <nclgl\Vector4.h>


using namespace CommonUtils;

TargetScene::TargetScene(const std::string& friendly_name)
	: Scene(friendly_name)
	, m_AccumTime(0.0f)
{

}

TargetScene::~TargetScene()
{
}

int TargetScene::score;
bool TargetScene::scored;
void TargetScene::OnInitializeScene()
{
	power = 0;
	score = 0;
	scoreTimer = 0;
	scored = false;
	shooting = false;
	targets = 14;
	//Disable the physics engine (We will be starting this later!)
	PhysicsEngine::Instance()->SetPaused(true);

	//Set the camera position
	GraphicsPipeline::Instance()->GetCamera()->SetPosition(Vector3(35.0f, 10.0f, -90.0f));
	GraphicsPipeline::Instance()->GetCamera()->SetYaw(180.f);
	GraphicsPipeline::Instance()->GetCamera()->SetPitch(0.f);

	m_AccumTime = 0.0f;

	//Example usage of Log 
	//- The on screen log can be opened from the bottom left though the
	//  main point of this is the ability to output to file easily from anywhere.
	//  So if your having trouble debugging with hundreds of 'cout << vector3()' just
	//  through them into NCLLOG() and look at the 'program_output.txt' later =]
	NCLDebug::Log("This is a log entry - It will printed to the console, on screen log and <project_dir>\program_output.txt");
	//NCLERROR("THIS IS AN ERROR!"); // <- On Debug mode this will also trigger a breakpoint in your code!


	GLuint tex = SOIL_load_OGL_texture(
		TEXTUREDIR"target.tga",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	bal = SOIL_load_OGL_texture(
		TEXTUREDIR"tenis.jpg",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

	glBindTexture(GL_TEXTURE_2D, bal);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0+1);

	G_targets.clear();
	B_targets.clear();
	
	for (int i = 0; i < targets; i++) {
		GameObject *target;
		target = BuildCuboidObject("Target", Vector3(0.f, 3.0f, 5.5f), Vector3(2.5f, 2.5f, 1.0f), true, 0.0f, true, false, Vector4(0.f, 1.f, .0f, 1.f));
		if (i % 2 == 0) {
			(*target->Render()->GetChildIteratorStart())->SetColor(Vector4(0, 1, 0, 1));
		//	(*target->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(tex);
			G_targets.push_back(target);
		}
		else {
			(*target->Render()->GetChildIteratorStart())->SetColor(Vector4(1, 0, 0, 1));
		//	(*target->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(tex);
			B_targets.push_back(target);
		}
		this->AddGameObject(target);
	}
}

void TargetScene::OnCleanupScene()
{
	//Just delete all created game objects 
	//  - this is the default command on any Scene instance so we don't really need to override this function here.
	Scene::OnCleanupScene();
}

void TargetScene::OnUpdateScene(float dt)
{
	m_AccumTime += dt;
	scoreTimer += dt;
	float poss = sin(m_AccumTime / 3);
	float posc = cos(m_AccumTime / 3);
	for (int i = 0; i < G_targets.size(); i++)
	{
		G_targets[i]->Physics()->SetPosition(Vector3(i*10 + (3 * ( poss) + 5*posc), 3+(-5 * poss + 2), 2));
		G_targets[i]->Physics()->SetOnCollisionCallback(ScoreInc);

	}
	for (int i = 0; i < B_targets.size(); i++) {
		B_targets[i]->Physics()->SetPosition(Vector3(i*10+(3 *(-poss) - 5 * posc),3+( 5 * poss + 2), -2));
		B_targets[i]->Physics()->SetOnCollisionCallback(ScoreDec);
	}

	if (Window::GetKeyboard()->KeyHeld(KEYBOARD_J)) {
		power += dt;
		shooting = true;
	}
	cout << power << "\n";
	if (power > 0.99f)
	{
		power = 1.0f;
	}
	if (scored && scoreTimer > 1.5f) {
		scored = false;
		scoreTimer = 0.f;
	}


	if (!Window::GetKeyboard()->KeyDown(KEYBOARD_J) && shooting && power > 0.1f) {
		ball = CommonUtils::BuildSphereObject("",
			Vector3(GraphicsPipeline::Instance()->GetCamera()->GetPosition() - Vector3(0, 0.5, 0)),				//Position
			0.5f,									//Radius
			true,									//Has Physics Object
			1.0f,									// Inverse Mass = 1 / 1kg mass
			true,									//No Collision Shape Yet
			false,									//Dragable by the user
			Vector4(0.8f, .8f, 0.2f, 1.0f));			//Color

		float yaw = GraphicsPipeline::Instance()->GetCamera()->GetYaw();
		float pitch = GraphicsPipeline::Instance()->GetCamera()->GetPitch();


		this->AddGameObject(ball);
		ball->Physics()->SetLinearVelocity(Matrix3::Rotation(pitch, Vector3(50 * power, 0, 0)) * Matrix3::Rotation(yaw, Vector3(0, 50 * power, 0)) * Vector3(0, 0, -50*power));
		ball->Physics()->SetAngularVelocity(Vector3(- 5 *power, 10* power, 0.0f));
		(*ball->Render()->GetChildIteratorStart())->GetMesh()->SetTexture(bal);
		shooting = false;
		power = 0.0f;
	}
	NCLDebug::DrawTextCs(Vector4(0.85f, 0.9f, 0.f, 1.f), 35, "Score: " + to_string(score), TEXTALIGN_CENTRE, Vector4(0.1f, 0.1f, 0.1f, 1.f));
	NCLDebug::DrawTextCs(Vector4(0.f, -0.9f, 0.f, 1.f), 25, to_string((int)(power * 100)) + "% POWER", TEXTALIGN_CENTRE, Vector4(0.0f, 0.0f, power, 1.0f));
	uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();
}

bool TargetScene::ScoreInc(PhysicsNode* self, PhysicsNode* collidingObject)
{
	if (!scored) {
		score += 100;
		scored = true;
	}
	return true;
}
bool TargetScene::ScoreDec(PhysicsNode* self, PhysicsNode* collidingObject)
{
	if (!scored) {
		score -= 50;
		scored = true;
	}
	return true;
}
