#include "MapManager.h"

MapManager::MapManager(PhysicsManager* physicisManager)
{
    _physicsManager = physicisManager;
    GenerateGameObjects();
}

std::vector<std::string> MapManager::LoadMapFromFile()
{
    std::ifstream file(MapFile);
    std::vector<std::string> map;
    std::string line;

    if (!file.is_open()) 
    {
        std::cerr << "[MAP_MANAGER] Couln't open map file" << std::endl;
        return map;
    }

    while (std::getline(file, line)) 
    {
        map.push_back(line);
    }

    file.close();
    return map;
}

void MapManager::GenerateGameObjects()
{
    std::vector<std::string> map = LoadMapFromFile();
    if (map.empty()) return;

    int rows = map.size();
    int cols = map[0].size();
    float tileWidth = WIDTH / cols;
    float tileHeight = HEIGHT / rows;

    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));

    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < cols; ++x)
        {
            char tile = map[y][x];
            if ((tile != '#' && tile != '-') || visited[y][x]) continue;

            // - Selects the color depending on the tile
            sf::Color color = (tile == '#') ? sf::Color::Blue : sf::Color::White;

            // - Extens X horizontally
            int maxX = x;
            while (maxX + 1 < cols && map[y][maxX + 1] == tile && !visited[y][maxX + 1]) maxX++;

            // - Extens Y vertically only if the columns from the blow horizontally are valid too
            int maxY = y;
            bool canExtendY = true;
            while (canExtendY && maxY + 1 < rows)
            {
                for (int i = x; i <= maxX; ++i)
                {
                    if (map[maxY + 1][i] != tile || visited[maxY + 1][i])
                    {
                        canExtendY = false;
                        break;
                    }
                }
                if (canExtendY) maxY++;
            }

            // - Marks the visited spots
            for (int iy = y; iy <= maxY; ++iy)
                for (int ix = x; ix <= maxX; ++ix)
                    visited[iy][ix] = true;

            // - Creates unified gameobjetcs to optimze colliders
            auto* go = new GameObject();
            auto* transform = go->GetComponent<Transform>();
            transform->position = {
                (x + (maxX - x + 1) / 2.0f) * tileWidth,
                (y + (maxY - y + 1) / 2.0f) * tileHeight
            };

            auto* collider = go->AddComponent<BoxCollider2D>();
            collider->size = {
                (maxX - x + 1) * tileWidth,
                (maxY - y + 1) * tileHeight
            };

            go->tag = "Wall";
            _mapObjects.push_back(go);
            _physicsManager->Register(go);
        }
    }
}
