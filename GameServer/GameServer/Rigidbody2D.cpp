#include "Rigidbody2D.h"

void Rigidbody2D::Update(Transform* transform, float dt)
{
	if (applyGravity)
	{
		velocity.y += gravityScale * dt;
	}

	transform->position += velocity * dt;
	if (velocity.y <= -300.f)
	{
		velocity.y = -300.f;
	}
}

const std::type_index Rigidbody2D::GetType() { return typeid(Rigidbody2D); }

