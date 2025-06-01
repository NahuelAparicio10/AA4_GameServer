#pragma once
#include "Component.h"
#include "Rigidbody2D.h"
#include "Transform.h"
#include "BulletHandler.h"
#include "NetworkDefs.h"

class PlayerComponentScript : public Component
{
public:
	PlayerComponentScript(BulletHandler* bHandler, GameObject* go);
	void Update(float dt);
	const std::type_index GetType() override;

private:
	void UpdatePlayerPhysics(float dt);
	void UpdateMovement(float dt);


	float _moveSpeed = 150.f;
	float _jumpForce = -300.f;
	GameObject* playerGo;
	Rigidbody2D* _rigidbody;
	BulletHandler* _bulletHandler;
	unsigned int _tick = 0;
};

