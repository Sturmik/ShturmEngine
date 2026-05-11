#pragma once

#include "ECS/ECS.h"
#include "Components/BoxColliderComponent.h"
#include "Components/ProjectileComponent.h"
#include "Components/HealthComponent.h"
#include "EventBus/EventBus.h"
#include "Events/CollisionEvent.h"

class DamageSystem : public System
{
public:
	DamageSystem()
	{
		RequireComponent<BoxColliderComponent>();
	}

	void SubscribeToEvents(EventBus& eventBus)
	{
		eventBus.SubscribeToEvent(this, &DamageSystem::OnCollision);
	}

	void OnCollision(CollisionEvent& event)
	{
		Entity entityA = event.a;
		Entity entityB = event.b;

		LOG_INFO("Damage system received an event collision between entities %d and %d", entityA.GetId(), entityB.GetId());
		
		if (entityA.BelongsToGroup("projectiles") && entityB.HasTag("player"))
		{
			OnProjectileHitsTarget(entityA, entityB);
		}
		if (entityB.BelongsToGroup("projectiles") && entityA.HasTag("player"))
		{
			OnProjectileHitsTarget(entityB, entityA);
		}

		if (entityA.BelongsToGroup("projectiles") && entityB.BelongsToGroup("enemies"))
		{
			OnProjectileHitsTarget(entityA, entityB);
		}
		if (entityB.BelongsToGroup("projectiles") && entityA.BelongsToGroup("enemies"))
		{
			OnProjectileHitsTarget(entityB, entityA);
		}
	}

	void OnProjectileHitsTarget(Entity projectile, Entity target)
	{
		if (!projectile.HasComponent<ProjectileComponent>() || !target.HasComponent<HealthComponent>())
		{
			return;
		}

		ProjectileComponent& projectileComponent = projectile.GetComponent<ProjectileComponent>();

		if ((!projectileComponent.isFriendly && target.HasTag("player")) || (projectileComponent.isFriendly && target.BelongsToGroup("enemies")))
		{
			// Reduce the health of the target by the projectile hit percent damage
			HealthComponent& health = target.GetComponent<HealthComponent>();

			// Subtract the health of the target
			health.healthPercentage -= projectileComponent.hitPercentDamage;

			// Kills the target when health reaches zero
			if (health.healthPercentage <= 0)
			{
				target.Kill();
			}

			// Kill the projectile
			projectile.Kill();
		}
	}
};