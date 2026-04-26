#pragma once

#include "ECS/ECS.h"
#include "Components/TransformComponent.h"
#include "Components/RigidBodyComponent.h"

class MovementSystem : public System
{
public:
	MovementSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<RigidBodyComponent>();
	}

	void Update(float deltaTime)
	{
		// Loop all entities that the system is interested in
		for (Entity& entity : GetSystemEntities())
		{
			// Update entity position based on its velocity every frame of the game loop
			TransformComponent& transform = entity.GetComponent<TransformComponent>();
			const RigidBodyComponent& rigidBody = entity.GetComponent<RigidBodyComponent>();
 
			transform.position.x += rigidBody.velocity.x * deltaTime;
			transform.position.y += rigidBody.velocity.y * deltaTime;
		}
	}
};