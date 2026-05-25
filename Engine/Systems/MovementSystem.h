#pragma once

#include "ECS/ECS.h"
#include "Components/TransformComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/SpriteComponent.h"
#include "EventBus/EventBus.h"
#include "Events/CollisionEvent.h"

class MovementSystem : public System
{
public:
	MovementSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<RigidBodyComponent>();
	}

	void SubscribeToEvents(EventBus& eventBus)
	{
		eventBus.SubscribeToEvent(this, &MovementSystem::OnCollision);
	}

	void OnCollision(CollisionEvent& event)
	{
		Entity entityA = event.a;
		Entity entityB = event.b;

		LOG_INFO("Damage system received an event collision between entities %d and %d", entityA.GetId(), entityB.GetId());

		if (entityA.BelongsToGroup("enemies") && entityB.BelongsToGroup("obstacles"))
		{
			OnEnemyHitsObstacle(entityA, entityB);
		}
		if (entityB.BelongsToGroup("obstacles") && entityA.BelongsToGroup("enemies"))
		{
			OnEnemyHitsObstacle(entityB, entityA);
		}
	}

	void OnEnemyHitsObstacle(Entity enemy, Entity obstacle)
	{
		if (!enemy.HasComponent<RigidBodyComponent>())
		{
			return;
		}

		RigidBodyComponent& rigidBody = enemy.GetComponent<RigidBodyComponent>();

		rigidBody.velocity.x *= -1;
		rigidBody.velocity.y *= -1;

		if (!enemy.HasComponent<SpriteComponent>())
		{
			return;
		}

		SpriteComponent& sprite = enemy.GetComponent<SpriteComponent>();

		// If x velocity is larger than y velocity - flip horizontally
		if (abs(rigidBody.velocity.x) > abs(rigidBody.velocity.y))
		{
			sprite.flip = (sprite.flip == SDL_FlipMode::SDL_FLIP_NONE ? SDL_FlipMode::SDL_FLIP_HORIZONTAL : SDL_FlipMode::SDL_FLIP_NONE);
		}
		// If y velocity is larger than x velocity - flip vertically
		else
		{
			sprite.flip = (sprite.flip == SDL_FlipMode::SDL_FLIP_NONE ? SDL_FlipMode::SDL_FLIP_VERTICAL : SDL_FlipMode::SDL_FLIP_NONE);
		}
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