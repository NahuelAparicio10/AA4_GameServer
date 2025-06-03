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
	 void AddPlayer(unsigned int playerID);
	 void RegisterPlayer(unsigned int id, GameObject* go) { _playerById[id] = go; }
	 bool startGame = false;

	#pragma region Getters
	 sf::Vector2f GetPlayerPositionByID(unsigned int playerID) { return _playerById[playerID]->transform->position; }
	 GameObject* GetPlayerByIndex(int index) { return _players[index]; }
	 GameObject* GetPlayerByID(unsigned int id) { return _playerById[id]; }
	 std::map<unsigned int, GameObject*>& GetPlayerMap() { return _playerById; }
	 BulletHandler* GetBulletHandler() { return _bulletHandler; }
	#pragma endregion

private:
	PhysicsManager _physicsManager;

	std::vector<GameObject*> _players;
	std::map<unsigned int, GameObject*> _playerById;
	MapManager* _mapManager; 
	BulletHandler* _bulletHandler;
};

