#pragma once
#include "Component.h"
#include "Transform.h"

class Rigidbody2D : public Component
{
public: 
    sf::Vector2f velocity;
    bool grounded = false;
    bool applyGravity = true;
    float gravityScale = 300.f;
    void Update(Transform* transform, float dt);
    const std::type_index GetType() override;

};

