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

void GameScene::Update(float dt)
{
	_bulletHandler->UpdateBullets(dt);

	for (auto* p : _players)
	{
		p->GetComponent<Rigidbody2D>()->Update(p->transform, dt);
	}	
	_physicsManager.Update(dt);
}



void GameScene::AddPlayer(unsigned int playerID)
{
	auto* player = new GameObject();
	player->transform->position = { 25, 25 };
	player->AddComponent<BoxCollider2D>()->size = { 32, 32 };
	player->AddComponent<Rigidbody2D>();
	player->id = playerID;

	_physicsManager.Register(player);
	_players.push_back(player);
	_playerById[playerID] = player;
}
