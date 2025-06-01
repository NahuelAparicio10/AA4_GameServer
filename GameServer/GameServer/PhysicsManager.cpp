#include "PhysicsManager.h"

// -- Registers a new Collider to colliders physic manager

void PhysicsManager::Register(GameObject* go)
{
	if (go->GetComponent<BoxCollider2D>())
	{
		_colliders.push_back(go);
	}
}

void PhysicsManager::UnRegister(GameObject* go)
{
    auto it = std::find(_colliders.begin(), _colliders.end(), go);

    if (it != _colliders.end())
    {
        _colliders.erase(it);
    }
}

void PhysicsManager::Update(float dt)
{
    //- Checks if all colliders in Physics Managers are colliding with something
    for (size_t i = 0; i < _colliders.size(); ++i)
    {
        auto objectA = _colliders[i];
        auto colliderA = objectA->GetComponent<BoxCollider2D>();
        auto transformA = objectA->GetComponent<Transform>();
        auto rigidbodyA = objectA->GetComponent<Rigidbody2D>();

        for (size_t j = i + 1; j < _colliders.size(); ++j)
        {
            auto objectB = _colliders[j];
            auto colliderB = objectB->GetComponent<BoxCollider2D>();
            auto transformB = objectB->GetComponent<Transform>();
            auto rigidbodyB = objectB->GetComponent<Rigidbody2D>();

            if (colliderA && colliderB && transformA && transformB)
            {
                auto intersection = colliderA->GetBounds(transformA).findIntersection(colliderB->GetBounds(transformB));
                if (intersection.has_value())
                {
                    bool isTriggerA = colliderA->isTrigger;
                    bool isTriggerB = colliderB->isTrigger;

                    if (isTriggerA || isTriggerB)
                    {
                        colliderA->OnTriggerEnter.Invoke(objectB);
                        colliderB->OnTriggerEnter.Invoke(objectA);
                        continue;
                    }

                    sf::FloatRect inter = intersection.value();
                    sf::Vector2f mtv;

                    // - Checks the size of the intersection
                    if (inter.size.x < inter.size.y)
                        mtv.x = (transformA->position.x < transformB->position.x) ? -inter.size.x : inter.size.x;
                    else
                        mtv.y = (transformA->position.y < transformB->position.y) ? -inter.size.y : inter.size.y;

                    // - Applies position correction
                    if (rigidbodyA && !rigidbodyB) transformA->position += mtv;
                    else if (rigidbodyB && !rigidbodyA) transformB->position -= mtv;
                    else {
                        transformA->position += mtv * 0.5f;

                        transformB->position -= mtv * 0.5f;
                    }

                    // - Removes the velocity from de rigidbody
                    if (rigidbodyA)
                    {
                        if (mtv.x != 0) rigidbodyA->velocity.x = 0;
                        if (mtv.y != 0) rigidbodyA->velocity.y = 0;
                    }
                    if (rigidbodyB)
                    {
                        if (mtv.x != 0) rigidbodyB->velocity.x = 0;
                        if (mtv.y != 0) rigidbodyB->velocity.y = 0;
                    }

                    colliderA->OnCollisionEnter.Invoke(objectB);
                    colliderB->OnCollisionEnter.Invoke(objectA);
                }
            }
        }
    }
}
