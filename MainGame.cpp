#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH{ 1280 };
int DISPLAY_HEIGHT{ 720 };
int DISPLAY_SCALE{ 1 };
constexpr int AGENT_RADIUS{ 48 };

////defining the default velocity for the agent, meteor and asteroid 
const Vector2D AGENT_VELOCITY_DEFAULT{ 0.f, 0.f };
const Vector2D ASTEROID_VELOCITY_DEFAULT{ 0.f, 0.0f }; 
const Vector2D METEOR_VELOCITY_DEFAULT{ 5.f, -0.5f };

float AGENT_SPEED{ 2.0f };
float METEOR_SPEED{ 1.0f };
float ASTEROID_SPEED{ 1.0f };

enum AgentState
{
	STATE_APPEAR, 
	STATE_PLAY, 
	STATE_DEAD,
	STATE_WIN, 
	STATE_LAND,
};

struct GameState
{
	int objectcount = 4;
	int score = 0;
	int level = 1; 
	int collision = 0;
	int deaths = 0; 
	bool agent_dead = false; 
	bool agent_won = false; 
	float asteroid_rotation = 0;
	float lift_off = 0;
	int gemcreated = 0;
	int gemsleft = objectcount - 1 ;  
	Point2f GemPos{ -100, -100 };
	AgentState agentstate = STATE_APPEAR; 
};

GameState gamestate;

enum GameObjectTyoe
{
	TYPE_AGENT = 0,
	TYPE_GEM,
	TYPE_METEOR,
	TYPE_ASTEROID,
	TYPE_RING, 
	TYPE_PIECES,
	TYPE_PARTICLES,
};

//function declarations
void Draw();

void UpdateAgent(); 
void UpdateGem(); 
void UpdateMeteor();
void UpdateAsteroid();

void UpdateControls();
void Collision();
void gemcollision(); 
void meteorcollision();  

void FloatDirectionObject(GameObject& obj, float speed);
void WrapObject(GameObject& obj);
float Randomize(int range, float multiplier);

void UpdateAgent(); 


void MainGameEntry(PLAY_IGNORE_COMMAND_LINE) //no need to use command line args for more playbuffer programs
{
	//setting up playbuffer
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background.png"); 
	Play::StartAudioLoop("music");

	//creating agent and gem object
	Play::CreateGameObject(TYPE_AGENT, { DISPLAY_WIDTH / 2,DISPLAY_HEIGHT / 2 }, 10, "agent8_fly");
/*	GameObject& obj_agent = Play::GetGameObjectByType(TYPE_AGENT); 
	obj_agent.rotation += Play::DegToRad(180);  */  

	Play::MoveSpriteOrigin("agent8_fly", 64, 108);
	Play::MoveSpriteOrigin("agent8_left_7", 64, 108);
	Play::MoveSpriteOrigin("agent8_right_7", 64, 108);

	//METEOR
	for (int m = 1; m < gamestate.objectcount - 1; m++)
	{
		int met_id = Play::CreateGameObject(TYPE_METEOR, { Play::RandomRollRange(0, 1280), Play::RandomRollRange(0, 720) }, AGENT_RADIUS, "meteor_2");  //random pos  
		//Play::MoveSpriteOrigin("meteor_2", 20, 15);
		Play::CentreSpriteOrigin("meteor_2"); 
		GameObject& meteor_obj = Play::GetGameObject(met_id);     
		meteor_obj.rotation = Randomize(628, 0.01); 
	}

	//GEMS
	for (int j = 1; j < gamestate.objectcount; j++)   
	{
		Play::CreateGameObject(TYPE_GEM, gamestate.GemPos, 10, "gem");
		Play::CentreSpriteOrigin("gem");
		GameObject& obj_gem = Play::GetGameObject(j);
		Play::DrawCircle(obj_gem.pos, 40, Play::cWhite);

	}

	//ASTEROID
	for (int i = 1; i < gamestate.objectcount; i++)
	{
		int ast_id = Play::CreateGameObject(TYPE_ASTEROID, { Play::RandomRollRange(0, 1280), Play::RandomRollRange(0, 720) }, AGENT_RADIUS, "asteroid_2"); //random pos  v 
		Play::CentreSpriteOrigin("asteroid_2");
		GameObject& asteroid_obj = Play::GetGameObject(ast_id);
		//WrapObject(asteroid_obj); 
		asteroid_obj.rotation = Randomize(628, 0.01);


	}

}

bool MainGameUpdate(float elapsedTime)
{
	UpdateMeteor();

	UpdateAsteroid();

	//UpdateControls(); --replaced by UpdateAgent() 

	UpdateAgent();

	gemcollision();

	Collision();

	meteorcollision(); 

	Draw();

	
	return Play::KeyDown(VK_ESCAPE);
}

int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}

void Draw()
{
	Play::ClearDrawingBuffer(Play::cWhite);

	Play::DrawBackground();

	for (int j : Play::CollectGameObjectIDsByType(TYPE_GEM))
	{
		Play::DrawObjectRotated(Play::GetGameObject(j));
		GameObject& obj_gem = Play::GetGameObject(j);
		//Play::DrawCircle({ obj_gem.pos }, 10, Play::cGreen);
	}

	for (int m : Play::CollectGameObjectIDsByType(TYPE_METEOR))
	{
		Play::DrawObjectRotated(Play::GetGameObject(m)); 
		GameObject& obj_meteor = Play::GetGameObject(m); 
		WrapObject(obj_meteor);   
		//Play::DrawCircle({ obj_meteor.pos }, 30, Play::cGreen); 
	}

	for (int i : Play::CollectGameObjectIDsByType(TYPE_ASTEROID))
	{
		Play::DrawObjectRotated(Play::GetGameObject(i));
		GameObject& obj_asteroid = Play::GetGameObject(i);
		WrapObject(obj_asteroid);
		//Play::DrawCircle({ obj_asteroid.pos }, 30, Play::cGreen);
	}

	Play::DrawObjectRotated(Play::GetGameObjectByType(TYPE_AGENT));
	GameObject& obj_agent{ Play::GetGameObjectByType(TYPE_AGENT) };
	WrapObject(obj_agent); 
	//Play::DrawCircle({ obj_agent.pos }, 10, Play::cGreen);

	Play::DrawFontText("64px", "Level: " + std::to_string(gamestate.level), Point2D(DISPLAY_WIDTH - 200, 50), Play::CENTRE);
	Play::DrawFontText("64px", "Score: " + std::to_string(gamestate.score), Point2D(DISPLAY_WIDTH / 2, 50), Play::CENTRE);
	
	Play::DrawFontText("64px", "LEFT AND RIGHT ARROW KEYS TO ROTATE AND SPACE TO LAUNCH", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 80 }, Play::CENTRE); 

	Play::DrawFontText("64px", "Gems Remaining: " + std::to_string(gamestate.gemsleft), Point2D(200 , 50), Play::CENTRE);   
 
	/*Play::DrawFontText("64px", "Deaths: " + std::to_string(gamestate.deaths), Point2D(50, 400), Play::LEFT);
	Play::DrawFontText("64px", "Asteroid Rotation: " + std::to_string(gamestate.asteroid_rotation), Point2D(50, 350), Play::LEFT);
	Play::DrawFontText("64px", "Lift Off: " + std::to_string(gamestate.lift_off), Point2D(50, 300), Play::LEFT);*/
	//Play::DrawFontText("64px", "Gems Created: " + std::to_string(gamestate.gemcreated), Point2D( 200 , 150), Play::CENTRE);

	Play::PresentDrawingBuffer();

}


void UpdateControls()
{
	GameObject& obj_agent = Play::GetGameObjectByType(TYPE_AGENT);
	Play::SetSprite(obj_agent, "agent8_fly", 0.05f);  

	FloatDirectionObject(obj_agent, AGENT_SPEED); // The function that makes it go in the direction its pointing 

	if (Play::KeyDown(VK_LEFT))
	{
		obj_agent.rotation -= 0.1f;
		Play::SetSprite(obj_agent, "agent8_left", 0.1f);
	}
	if (Play::KeyDown(VK_RIGHT))
	{  
		obj_agent.rotation += 0.1f;     
		Play::SetSprite(obj_agent, "agent8_right", 0.1f);      
	}
	if (Play::KeyDown(VK_UP))
	{
		AGENT_SPEED += 0.3f;
	}
	if (Play::KeyDown(VK_DOWN))
	{
		AGENT_SPEED *= 0.9f;
	}
	if (Play::KeyDown(VK_F1))
	{
		obj_agent.pos = Vector2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2);
		obj_agent.velocity = AGENT_VELOCITY_DEFAULT;
	}

	std::vector<int> vAsteroids = Play::CollectGameObjectIDsByType(TYPE_ASTEROID);
	for (int id_asteroid : vAsteroids)
	{
		GameObject& obj_asteroid = Play::GetGameObject(id_asteroid);
		if (Play::IsColliding(obj_agent, obj_asteroid))
		{
			obj_agent.rotation += Play::DegToRad(180); 
			Play::SetSprite(obj_agent, "agent8_left", 0.05f);
			
		}
	}
	//Play::DrawSpriteCircle(obj_agent.pos, 10, "agent8_fly", Play::cBlue);  
	Play::UpdateGameObject(obj_agent);
}

void UpdateAgent( )
{
	GameObject& obj_agent = Play::GetGameObjectByType(TYPE_AGENT);

	//obj_agent.rotation += Play::DegToRad(180);   
	

	switch (gamestate.agentstate)
	{
	case STATE_APPEAR:
		Play::DrawFontText("64px", "PRESS ENTER TO START", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 300 }, Play::CENTRE);
		obj_agent.velocity = { 0, 0 }; 
		Play::SetSprite(obj_agent, "agent8_fly", 0.05f); 
		if (Play::KeyDown(VK_RETURN))
		{
			gamestate.agentstate = STATE_PLAY;  
		}
		Play::PresentDrawingBuffer();
		break; 

	case STATE_PLAY:
		UpdateControls();
		if (gamestate.agent_won)
		{
			gamestate.agentstate = STATE_WIN;
		}
		if (gamestate.agent_dead)
		{
			gamestate.agentstate = STATE_DEAD; 
		} 
		break; 

	case STATE_DEAD:
		Play::SetSprite(obj_agent, "agent8_dead_2", 0.05f);
		Play::DrawFontText("64px", "GAME OVER", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 300 }, Play::CENTRE);
		Play::PresentDrawingBuffer();
		break; 
	
	case STATE_WIN:
		Play::SetSprite(obj_agent, "saucer", 0.f);
		obj_agent.rotation = 0.3f; 
		obj_agent.velocity = { 5, -1 }; 
		Play::DrawFontText("64px", "JACKPOT !!", { DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 300 }, Play::CENTRE);
		Play::PresentDrawingBuffer();
		break; 
	}

	if (gamestate.agent_dead)   
	{
		Play::SetSprite(obj_agent, "agent8_dead_2", 0.05f); 
	} 


	Play::UpdateGameObject(obj_agent);
	Play::DrawObjectRotated(obj_agent);

}

void UpdateGem()
{
	for (int i : Play::CollectGameObjectIDsByType(TYPE_GEM))
	{
		GameObject& obj_gem = Play::GetGameObject(TYPE_GEM);

		FloatDirectionObject(obj_gem, METEOR_SPEED);

		obj_gem.rotation = 0.5f;

		Play::UpdateGameObject(obj_gem);
	}
}


void UpdateAsteroid()
{
	GameObject& obj_agent = Play::GetGameObjectByType(TYPE_AGENT);   

	for (int i : Play::CollectGameObjectIDsByType(TYPE_ASTEROID))
	{

		GameObject& obj_asteroid = Play::GetGameObject(i); 

		gamestate.asteroid_rotation = obj_asteroid.rotation; 

		switch (gamestate.agentstate)
		{
		case STATE_APPEAR:
			obj_asteroid.velocity = { 0, 0 },
			Play::SetSprite(obj_asteroid, "asteroid_2", 0.05f);
			if (Play::KeyDown(VK_RETURN)) 
			{
				gamestate.agentstate = STATE_PLAY; 
			} 
			break; 

		case STATE_PLAY:
			
			FloatDirectionObject(obj_asteroid, METEOR_SPEED);  
			obj_asteroid.pos += obj_asteroid.velocity; 
			if (gamestate.agent_won)
			{
				gamestate.agentstate = STATE_WIN; 
			}

			if (gamestate.agent_dead)
			{
				gamestate.agentstate = STATE_DEAD; 
			}
			break; 

		case STATE_DEAD:
			Play::DestroyGameObject(i); 
			break; 

		case STATE_WIN:
			Play::DestroyGameObject(i);
			break;

		}


		Play::SetSprite(obj_asteroid, "asteroid_2", 0.05f); //sprite, .png, anim speed      

		Play::UpdateGameObject(obj_asteroid);

	}
}

void UpdateMeteor()
{
	for (int m : Play::CollectGameObjectIDsByType(TYPE_METEOR)) 
	{
		GameObject& obj_meteor = Play::GetGameObject(m); 

		Play::SetSprite(obj_meteor, "meteor_2", 0.05f); //sprite, .png, anim speed   

		switch (gamestate.agentstate)
		{
		case STATE_APPEAR:
			obj_meteor.velocity = { 0,0 };
			Play::SetSprite(obj_meteor, "meteor_2", 0.05f); 
			if (Play::KeyDown(VK_RETURN))
			{
				gamestate.agentstate = STATE_PLAY;
			}
			break; 

		case STATE_PLAY:
			FloatDirectionObject(obj_meteor, METEOR_SPEED);  
			obj_meteor.pos += obj_meteor.velocity;  
			if (gamestate.agent_won) 
			{
				gamestate.agentstate = STATE_WIN;
			}
			if (gamestate.agent_dead)
			{
				gamestate.agentstate = STATE_DEAD;
			}
			break;
		
		case STATE_DEAD:
			Play::DestroyGameObject(m);  
			break;

		case STATE_WIN:  
			Play::DestroyGameObject(m);   
			break;
		}

		Play::UpdateGameObject(obj_meteor);
	}
}

void Collision()
{
	bool liftoff = false;

	//agent - asteroid collision
	GameObject& obj_agent = Play::GetGameObjectByType(TYPE_AGENT);
	   
	for (int i : Play::CollectGameObjectIDsByType(TYPE_ASTEROID))
	{

		GameObject& obj_asteroid = Play::GetGameObject(i);
		if (Play::IsColliding(obj_agent, obj_asteroid))
		{
			
			obj_agent.pos = obj_asteroid.pos; //update agent's position
			//obj_agent.rotation = obj_agent.rotation + Play::DegToRad(180);  

			gamestate.collision += 1;

			Play::SetSprite(obj_agent, "agent8_left_7", 0.01);

			UpdateControls(); 


			if (Play::KeyDown(VK_SPACE))
			{
				Play::PlayAudio("laser");
				gamestate.lift_off += 1;
				liftoff = true;

				int gem_id = Play::CreateGameObject(TYPE_GEM, obj_asteroid.pos, AGENT_RADIUS, "gem");
				GameObject& obj_gem = Play::GetGameObject(gem_id);
				obj_gem.pos = obj_asteroid.pos;  
				gamestate.gemcreated += 1;

				obj_agent.pos -= obj_agent.velocity * 40; 

				Play::DestroyGameObject(i);
			
			}

			Play::UpdateGameObject(obj_agent);

		}
 
	}
}

void gemcollision()
{
	GameObject& obj_agent = Play::GetGameObjectByType(TYPE_AGENT);
	for (int j : Play::CollectGameObjectIDsByType(TYPE_GEM))
	{
		//gamestate.gemsleft = gamestate.objectcount - 1; 
		GameObject& obj_gem = Play::GetGameObject(j);
		bool gemcollision = false; 
		if (Play::IsColliding(obj_gem, obj_agent))
		{
			gamestate.score += 1000;

			gamestate.gemsleft -= 1; 

			gemcollision = true; 
			
		}

		if (gemcollision)
		{
			//Play::SetSprite(obj_agent, "blue_ring", 0.01); 
			Play::PlayAudio("reward"); 
			Play::DestroyGameObject(j);  
			//gamestate.gemsleft -= 1; 

			if (gamestate.gemsleft < 1)
			{
				gamestate.agent_won = true; 
			}
		}

		Play::UpdateGameObject(obj_gem);  
	}
}

void meteorcollision()
{
	GameObject& obj_agent = Play::GetGameObjectByType(TYPE_AGENT);
	for (int m : Play::CollectGameObjectIDsByType(TYPE_METEOR))
	{
		GameObject& obj_meteor = Play::GetGameObject(m); 
		//bool agent_meteor_collision = false;   
		 
		if (Play::IsColliding(obj_agent, obj_meteor))
		{	
			Play::PlayAudio("combust"); 

			obj_agent.pos = obj_meteor.pos; 

			Play::SetSprite(obj_meteor, "agent8_dead_2", 0.05f); 
			Play::DrawSpriteCircle(obj_agent.pos, 1, "agent8_dead_2", Play::cOrange);   //cMagenta , cOrange,  cGrey

			obj_agent.rotation = obj_meteor.rotation; 
			obj_agent.velocity = obj_meteor.velocity; 

			gamestate.deaths += 1; 
			gamestate.agent_dead = true;  
			
			Play::StopAudioLoop("music");     
			   
			Play::DestroyGameObject(m);     
		}
	}
}

float Randomize(int range, float multiplier = 1.f)
{
	return (float)(rand() % range) * multiplier;
}

void WrapObject(GameObject& obj)
{

	if (obj.pos.y < 0)
	{
		obj.pos.x = obj.oldPos.x;
		obj.pos.y = DISPLAY_HEIGHT;
	}

	if (obj.pos.y > DISPLAY_HEIGHT)
	{
		obj.pos.x = obj.oldPos.x;
		obj.pos.y = 0;
	} 

	if (obj.pos.x < 0)
	{
		obj.pos.y = obj.oldPos.y;
		obj.pos.x = DISPLAY_WIDTH;
	}

	if (obj.pos.x > DISPLAY_WIDTH)
	{
		obj.pos.y = obj.oldPos.y;
		obj.pos.x = 0;
	}

	obj.pos += obj.velocity;
}

// Takes a game object and what speed you want and makes it move in the direction it is facing
void FloatDirectionObject(GameObject& obj, float speed)
{
	float x = sin(obj.rotation);
	float y = cos(obj.rotation);

	obj.velocity.x = x * speed;
	obj.velocity.y = -y * speed;
}

void test()
{
	//commit to see if the file has been renamed
}