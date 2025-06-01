#pragma once
#include "GameObject.h"
#include "BoxCollider2D.h"
#include "Transform.h"
#include "Rigidbody2D.h"

class PhysicsManager
{
public:
    void Register(GameObject* go);
    void UnRegister(GameObject* go);
    void Update(float dt);

private:
    std::vector<GameObject*> _colliders;
};

