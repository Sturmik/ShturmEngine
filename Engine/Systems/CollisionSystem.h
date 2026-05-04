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
			// Get transform and box collider component
			const TransformComponent& currentTransform = AccessSystemEntities()[i].GetComponent<TransformComponent>();
			const BoxColliderComponent& currentBoxCollider = AccessSystemEntities()[i].GetComponent<BoxColliderComponent>();

			// Test collision against other entities
			for (int j = i + 1; j < AccessSystemEntities().size(); ++j)
			{
				// Get transform and box collider component
				const TransformComponent& otherTransform = AccessSystemEntities()[j].GetComponent<TransformComponent>();
				const BoxColliderComponent& otherBoxCollider = AccessSystemEntities()[j].GetComponent<BoxColliderComponent>();

				bool collisionHappened = CheckAABBCollision(
				currentTransform.position.x + currentBoxCollider.offset.x,
				currentTransform.position.y + currentBoxCollider.offset.y,
				currentBoxCollider.width * currentTransform.scale.x,
				currentBoxCollider.height * currentTransform.scale.y,
				otherTransform.position.x + otherBoxCollider.offset.x,
				otherTransform.position.y + otherBoxCollider.offset.y,
				otherBoxCollider.width * otherTransform.scale.x,
				otherBoxCollider.height * otherTransform.scale.y);

				if (collisionHappened)
				{
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