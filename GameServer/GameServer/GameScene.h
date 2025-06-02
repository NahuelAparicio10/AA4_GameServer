#pragma once
#include "PhysicsManager.h"
#include "MapManager.h"
#include "Rigidbody2D.h"
#include "BulletHandler.h"
#include "PlayerComponentScript.h"

class GameScene 
{
public:
	GameScene(int numPlayers);
	~GameScene();

	 void Update(float dt);
	 sf::Vector2f AddPlayer(unsigned int playerID);
	 GameObject* GetPlayerByIndex(int index) { return _players[index]; }
	 GameObject* GetPlayerByID(unsigned int id) { return _playerById[id]; }
	 std::map<unsigned int, GameObject*>& GetPlayerMap() { return _playerById; }
	 void RegisterPlayer(unsigned int id, GameObject* go) { _playerById[id] = go; }
private:
	PhysicsManager _physicsManager;

	std::vector<GameObject*> _players;
	std::map<unsigned int, GameObject*> _playerById;
	MapManager* _mapManager; 
	BulletHandler* _bulletHandler;
};

