#pragma once

#include "ECS/ECS.h"
#include "Components/TransformComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/ProjectileEmitterComponent.h"
#include "Components/LifecycleComponent.h"

class ProjectileEmitSystem : public System
{
public:
	ProjectileEmitSystem()
	{
		RequireComponent<ProjectileEmitterComponent>();
		RequireComponent<TransformComponent>();
	}

	void SubscribeToEvents(EventBus& eventBus)
	{
		eventBus.SubscribeToEvent(this, &ProjectileEmitSystem::OnKeyPressed);
	}

	void OnKeyPressed(KeyPressedEvent& event)
	{
		if (event.keyScancode == SDL_SCANCODE_SPACE)
		{
			// Loop all entities that the system is interested in
			for (Entity& entity : AccessSystemEntities())
			{
				// Get entity components
				ProjectileEmitterComponent& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
				const TransformComponent& transform = entity.GetComponent<TransformComponent>();

				// Define projectile emit direction by rigidbody of parent entity
				glm::vec2 projectileEmitVector = projectileEmitter.projectileVelocity;
				if (entity.HasComponent<RigidBodyComponent>())
				{
					projectileEmitVector = glm::normalize(entity.GetComponent<RigidBodyComponent>().velocity) * glm::length(projectileEmitter.projectileVelocity);
				}

				// Define projectile start position
				glm::vec2 projectilePosition = transform.position;
				if (entity.HasComponent<SpriteComponent>())
				{
					const SpriteComponent& spriteComponent = entity.GetComponent<SpriteComponent>();

					projectilePosition.x += spriteComponent.width * transform.scale.x / 2.0;
					projectilePosition.y += spriteComponent.height * transform.scale.y / 2.0;
				}

	// Add a new projectile entity to the registry
				Entity projectile = entity.AccessRegistry().CreateEntity();
				projectile.AddComponent<TransformComponent>(projectilePosition, glm::vec2(1.0, 1.0), 0);
				projectile.AddComponent<RigidBodyComponent>(projectileEmitVector);

				// Add sprite with bullet image
				projectile.AddComponent<SpriteComponent>(AssetStore::Get(), projectileEmitter.projectileTextureKey, 4);

				// Add colldier with correct size according to sprite
				float textureWidth = 0.0f;
				float textureHeight = 0.0f;
				SDL_GetTextureSize(AssetStore::Get().GetTexture(projectileEmitter.projectileTextureKey),
					&textureWidth,
					&textureHeight);
				projectile.AddComponent<BoxColliderComponent>(textureWidth, textureHeight);

				// Attach lifecycle to destroy projectile after some time
				projectile.AddComponent<LifecycleComponent>(projectileEmitter.projectileDurationInMs);

				// Update the projectile emitter component last emission to the current milliseconds
				projectileEmitter.lastEmissionTimeInMs = SDL_GetTicks();
			}
		}
	}

	void Update(Registry& registry)
	{
		// Loop all entities that the system is interested in
		for (Entity& entity : AccessSystemEntities())
		{
			ProjectileEmitterComponent& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
			const TransformComponent& transform = entity.GetComponent<TransformComponent>();

			if (projectileEmitter.repeatFrequency == 0)
			{
				continue;
			}

			// Check if its time to re-emit a new projectileEmitter
			if (SDL_GetTicks() - projectileEmitter.lastEmissionTimeInMs > projectileEmitter.repeatFrequency)
			{
				glm::vec2 projectilePosition = transform.position;
				if (entity.HasComponent<SpriteComponent>())
				{
					const SpriteComponent& spriteComponent = entity.GetComponent<SpriteComponent>();

					projectilePosition.x += spriteComponent.width * transform.scale.x / 2.0;
					projectilePosition.y += spriteComponent.height * transform.scale.y / 2.0;
				}

				// Add a new projectile entity to the registry
				Entity projectile = registry.CreateEntity();
				projectile.AddComponent<TransformComponent>(projectilePosition, glm::vec2(1.0, 1.0), 0);
				projectile.AddComponent<RigidBodyComponent>(projectileEmitter.projectileVelocity);
				
				// Add sprite with bullet image
				projectile.AddComponent<SpriteComponent>(AssetStore::Get(), projectileEmitter.projectileTextureKey, 4);
				
				// Add colldier with correct size according to sprite
				float textureWidth = 0.0f;
				float textureHeight = 0.0f;
				SDL_GetTextureSize(AssetStore::Get().GetTexture(projectileEmitter.projectileTextureKey),
					&textureWidth,
					&textureHeight);
				projectile.AddComponent<BoxColliderComponent>(textureWidth, textureHeight);

				// Attach lifecycle to destroy projectile after some time
				projectile.AddComponent<LifecycleComponent>(projectileEmitter.projectileDurationInMs);

				// Update the projectile emitter component last emission to the current milliseconds
				projectileEmitter.lastEmissionTimeInMs = SDL_GetTicks();
			}
		}
	}
};