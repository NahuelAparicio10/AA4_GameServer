#include "GameScene.h"

GameScene::GameScene(int numPlayers)
{
	_mapManager = new MapManager(&_physicsManager);
	_bulletHandler = new BulletHandler(&_physicsManager);
}

GameScene::~GameScene()
{
	delete _mapManager;
	delete _bulletHandler;
}

// -- Updates bullets and physic system

void GameScene::Update(float dt)
{
	_bulletHandler->UpdateBullets(dt);

	for (auto* p : _players)
	{
		p->GetComponent<Rigidbody2D>()->Update(p->transform, dt);
	}	

	_physicsManager.Update(dt);
}


// -- Adds a player to the game server with all the components that needs
void GameScene::AddPlayer(unsigned int playerID)
{
   //"Añado player " 
    auto* player = new GameObject();

    // Espaciado horizontal por ID
    float xPos = 25.f + 250.f * static_cast<float>(playerID);
    player->transform->position = { xPos, 600.f };

    auto* rb = player->AddComponent<Rigidbody2D>();
    rb->applyGravity = true;

    player->AddComponent<BoxCollider2D>()->size = { 32.f, 32.f };
    player->id = playerID;
    player->tag = "Player";

    _physicsManager.Register(player);
    _players.push_back(player);
    _playerById[playerID] = player;

   
}
