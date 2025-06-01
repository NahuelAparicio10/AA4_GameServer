#include "PlayerComponentScript.h"

PlayerComponentScript::PlayerComponentScript(BulletHandler* bHandler, GameObject* go)
{
	playerGo = go;
	_bulletHandler = bHandler;
	_rigidbody = go->GetComponent<Rigidbody2D>();

}

void PlayerComponentScript::Update(float dt)
{
    UpdateMovement(dt);
    UpdatePlayerPhysics(dt);
}


void PlayerComponentScript::UpdatePlayerPhysics(float dt)
{
    _rigidbody->Update(playerGo->transform, dt);
}

void PlayerComponentScript::UpdateMovement(float dt)
{
}



const std::type_index PlayerComponentScript::GetType()
{
    return typeid(PlayerComponentScript);
}

