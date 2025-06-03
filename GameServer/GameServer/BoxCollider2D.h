#pragma once
#include <SFML/Graphics.hpp>
#include "Component.h"
#include "Event.h"
#include "Transform.h"
#include "GameObject.h"

class BoxCollider2D : public Component
{
public:
	sf::Vector2f size;
	sf::Vector2f offset;

	bool isTrigger = false;

	Event<GameObject*, GameObject*> OnCollisionEnter;
	Event<GameObject*, GameObject*> OnTriggerEnter;
	sf::FloatRect GetBounds(Transform* transform) const;

	virtual const std::type_index GetType() override;
};

