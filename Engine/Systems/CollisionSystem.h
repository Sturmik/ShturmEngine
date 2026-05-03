#pragma once

#include "ECS/ECS.h"
#include "Components/TransformComponent.h"
#include "Components/BoxColliderComponent.h"

class CollisionSystem : public System
{
public:
	CollisionSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<BoxColliderComponent>();
	}

	void Update()
	{
		// TODO: Check all entities that have a boxcollider
		// to see if they are colliding with each other
		// AABB collision check
	}
};