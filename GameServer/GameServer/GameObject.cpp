#include "GameObject.h"

GameObject::GameObject()
{
	transform = AddComponent<Transform>();
}

GameObject::~GameObject()
{
	for (Component* component : components)
	{
		if (component)
		{
			delete component;
		}
	}
	components.clear();
}