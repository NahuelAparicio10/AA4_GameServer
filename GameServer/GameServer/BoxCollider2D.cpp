#include "BoxCollider2D.h"



sf::FloatRect BoxCollider2D::GetBounds(Transform* transform) const
{
    return sf::FloatRect(transform->position + offset - size / 2.f, size);
}

const std::type_index BoxCollider2D::GetType() { return typeid(BoxCollider2D); }

