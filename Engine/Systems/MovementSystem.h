#pragma once

#include "ECS/ECS.h"
#include "Components/TransformComponent.h"
#include "Components/RigidBodyComponent.h"
#include "Components/SpriteComponent.h"
#include "EventBus/EventBus.h"
#include "Events/CollisionEvent.h"

#include <algorithm>

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
		ForEach<TransformComponent, RigidBodyComponent>([&](Entity entity, TransformComponent& transform, RigidBodyComponent& rigidBody)
		{
			// Update position based on velocity
			transform.position.x += rigidBody.velocity.x * deltaTime;
			transform.position.y += rigidBody.velocity.y * deltaTime;

			// Check, if entity is out of bounds
			float bottomRightPositionX = transform.position.x;
			float bottomRightPositionY = transform.position.y;
			float possibleMarginX = 0.0f;
			float possibleMarginY = 0.0f;

			// Assume, that our position is on the top-left corner of the srite, therefore we need to account for that
			if (entity.HasComponent<SpriteComponent>())
			{
				SpriteComponent& sprite = entity.GetComponent<SpriteComponent>();

				possibleMarginX = sprite.srcRect.w * transform.scale.x;
				possibleMarginY = sprite.srcRect.h * transform.scale.y;

				bottomRightPositionX += possibleMarginX;
				bottomRightPositionY += possibleMarginY;
			}

			bool isEntityOutsideMap = {
				transform.position.x < 0.0f ||
				bottomRightPositionX > mapSize.x ||
				transform.position.y < 0.0f ||
				bottomRightPositionY > mapSize.y
			};

			// Kill all entities that move outside the map boundaries
			if (isEntityOutsideMap && !entity.HasTag("player"))
			{
				LOG_INFO("Entity %d out of bounds is destroyed", entity.GetId());
				entity.Kill();
			}

			// Stop the player and return it to the map boundaries
			if (isEntityOutsideMap && entity.HasTag("player"))
			{
				transform.position.x = std::clamp(transform.position.x, 0.0f, mapSize.x - possibleMarginX);
				transform.position.y = std::clamp(transform.position.y, 0.0f, mapSize.y - possibleMarginY);

				rigidBody.velocity.x = rigidBody.velocity.y = 0.0f;
			}
		});
	}
};