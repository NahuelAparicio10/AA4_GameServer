#pragma once
#include "Component.h"
#include <SFML/System/Vector2.hpp>

class Transform : public Component
{
public:
	Transform();

	sf::Vector2f position;
	sf::Angle rotation;
	sf::Vector2f scale;

	const std::type_index GetType() override;

	void Translate(const sf::Vector2f& offset);

	void Rotate(sf::Angle angle);
};

