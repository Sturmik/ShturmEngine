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
		// Loop all entities that the system is interested in
		for (int i = 0; i < AccessSystemEntities().size(); ++i)
		{
			// Get current entity
			const Entity& currentEntity = AccessSystemEntities()[i];

			// Get transform and box collider component
			BoxColliderComponent& currentBoxCollider = currentEntity.GetComponent<BoxColliderComponent>();

			// Reset collision state
			currentBoxCollider.isColliding = false;
		}

		// Loop all entities that the system is interested in
		for (int i = 0; i < AccessSystemEntities().size(); ++i)
		{
			// Get current entity
			const Entity& currentEntity = AccessSystemEntities()[i];

			// Get transform and box collider component
			const TransformComponent& currentTransform = currentEntity.GetComponent<TransformComponent>();
			BoxColliderComponent& currentBoxCollider = currentEntity.GetComponent<BoxColliderComponent>();

			// Test collision against other entities
			for (int j = i + 1; j < AccessSystemEntities().size(); ++j)
			{
				// Get other entity
				const Entity& otherEntity = AccessSystemEntities()[j];

				if (currentEntity == otherEntity)
				{
					continue;
				}

				// Get transform and box collider component
				const TransformComponent& otherTransform = otherEntity.GetComponent<TransformComponent>();
				BoxColliderComponent& otherBoxCollider = otherEntity.GetComponent<BoxColliderComponent>();

				bool collisionHappened = CheckAABBCollision(
				currentTransform.position.x + currentBoxCollider.offset.x,
				currentTransform.position.y + currentBoxCollider.offset.y,
				(currentBoxCollider.offset.x + currentBoxCollider.width) * currentTransform.scale.x,
				(currentBoxCollider.offset.y + currentBoxCollider.height) * currentTransform.scale.y,
				otherTransform.position.x + otherBoxCollider.offset.x,
				otherTransform.position.y + otherBoxCollider.offset.y,
				(otherBoxCollider.offset.x + otherBoxCollider.width) * otherTransform.scale.x,
				(otherBoxCollider.offset.y + otherBoxCollider.height) * otherTransform.scale.y);

				if (collisionHappened)
				{
					currentBoxCollider.isColliding = true;
					otherBoxCollider.isColliding = true;

					LOG_INFO("Entity %d collided with entity %d", AccessSystemEntities()[i].GetId(), AccessSystemEntities()[j].GetId());
				}
			}
		}
	}

	bool CheckAABBCollision(double aX, double aY, double aW, double aH, double bX, double bY, double bW, double bH) {
		return (
			aX < bX + bW &&
			aX + aW > bX &&
			aY < bY + bH &&
			aY + aH > bY
			);
	}
};