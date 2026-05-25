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

	void Update(float deltaTime, glm::vec2 mapSize)
	{
		ForEach<TransformComponent, RigidBodyComponent>([&](Entity entity, TransformComponent& transform, const RigidBodyComponent& rigidBody)
		{
			transform.position.x += rigidBody.velocity.x * deltaTime;
			transform.position.y += rigidBody.velocity.y * deltaTime;

			bool isEntityOutsideMap = {
				transform.position.x < 0 ||
				transform.position.x > mapSize.x ||
				transform.position.y < 0 ||
				transform.position.y > mapSize.y
			};

			// Kill all entities that move outside the map boundaries
			if (isEntityOutsideMap && !entity.HasTag("player"))
			{
				entity.Kill();
			}
		});
	}
};