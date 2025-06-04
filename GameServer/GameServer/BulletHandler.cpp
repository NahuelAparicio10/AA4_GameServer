#include "BulletHandler.h"

BulletHandler::BulletHandler(PhysicsManager* pManager) : _physicsManager(pManager) { }

void BulletHandler::CreateBullet(int bulletID, sf::Vector2f position, sf::Vector2f direction)
{
    auto* bullet = new GameObject();
    bullet->transform->position = position;

    auto* collider = bullet->AddComponent<BoxCollider2D>();
    collider->size = static_cast<sf::Vector2f>(sf::Vector2f{16,16});
    collider->isTrigger = true;

    auto* rb = bullet->AddComponent<Rigidbody2D>();
    rb->applyGravity = false;
    rb->velocity = direction * 150.f;

    bullet->id = bulletID;

    if (_physicsManager)
        _physicsManager->Register(bullet);

    _bullets.push_back(bullet);

    collider->OnTriggerEnter.Subscribe([this](GameObject* other, GameObject* me) 
        {
            if (other->tag == "Wall")
            {
                onWallHitted.Invoke(me->id);
            }
            if (other->tag == "Player")
            {
                onPlayerHitted.Invoke(other->id, me->id);
            }
        });
}

void BulletHandler::UpdateBullets(float dt)
{
    if (_bullets.empty()) return;

	for (auto* b : _bullets)
	{
        b->GetComponent<Rigidbody2D>()->Update(b->transform, dt);
	}
}

