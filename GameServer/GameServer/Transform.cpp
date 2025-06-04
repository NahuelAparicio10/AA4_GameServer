#include "Transform.h"

Transform::Transform()
{
    position = sf::Vector2f(0.f, 0.f);
    scale = sf::Vector2f(1.f, 1.f);
}

void Transform::Translate(const sf::Vector2f& offset)
{
    position += offset;
}
void Transform::Rotate(sf::Angle angle)
{
    rotation += angle;
}

const std::type_index Transform::GetType()
{
    return typeid(Transform);
}