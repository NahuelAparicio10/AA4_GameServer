#pragma once
#include<SFML/Graphics.hpp>
#include "PhysicsManager.h"

class BulletHandler
{
public:
	BulletHandler(PhysicsManager* pManager);
	void CreateBullet(int bulletID, sf::Vector2f position, sf::Vector2f direction);
	void UpdateBullets(float dt);
	void SetPhysicsManager(PhysicsManager* pManager) { _physicsManager = pManager; }
	Event<int> onWallHitted;
	Event<int> onPlayerHitted;
private:
	std::vector<GameObject*> _bullets;
	PhysicsManager* _physicsManager;

};

