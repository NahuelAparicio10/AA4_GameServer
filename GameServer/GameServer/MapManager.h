#pragma once
#include "GameObject.h"
#include <vector>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <string>
#include <iostream>
#include "Constants.h"
#include "PhysicsManager.h"

class MapManager
{
public:
	MapManager(PhysicsManager* physicisManager);
	std::vector<std::string> LoadMapFromFile();
	void GenerateGameObjects();
private:
	std::vector<GameObject*> _mapObjects;
	PhysicsManager* _physicsManager;

};

