#pragma once

#include "ECS/ECS.h"
#include "Components/TransformComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/ProjectileEmitterComponent.h"
#include "Components/ProjectileComponent.h"
#include "Components/LifecycleComponent.h"

#include "EventBus/EventBus.h"
#include "Events/PlaySoundEvent.h"

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
			for (std::shared_ptr<Archetype>& archetype : AccessArchetypes())
			{
				std::vector<Entity>& entities = archetype->entities;

				// Loop all entities that the system is interested in
				for (Entity& entity : entities)
				{
					if (entity.HasTag("player")) 
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
						projectile.Group("projectiles");
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

						// Add projectile component to define projectile damage and if it is friendly
						projectile.AddComponent<ProjectileComponent>( projectileEmitter.isFriendly, projectileEmitter.hitPercentDamage);

						// Attach lifecycle to destroy projectile after some time
						projectile.AddComponent<LifecycleComponent>(projectileEmitter.projectileDurationInMs);

						// Update the projectile emitter component last emission to the current milliseconds
						projectileEmitter.lastEmissionTimeInMs = SDL_GetTicks();

						// Emit sound
						if (event.eventBusPtr && !projectileEmitter.projectileSoundKey.empty())
						{
							event.eventBusPtr->EmitEvent<PlaySoundEvent>(projectileEmitter.projectileSoundKey);
						}
					}
				}
			}
		}
	}

	void Update(Registry& registry)
	{
		for (std::shared_ptr<Archetype>& archetype : AccessArchetypes())
		{
			std::vector<Entity>& entities = archetype->entities;

			// Loop all entities that the system is interested in
			for (Entity& entity : entities)
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

						projectilePosition.x += spriteComponent.width * transform.scale.x / 2.0f;
						projectilePosition.y += spriteComponent.height * transform.scale.y / 2.0f;
					}

					// Add a new projectile entity to the registry
					Entity projectile = registry.CreateEntity();
					projectile.Group("projectiles");
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

					// Add projectile component to define projectile damage and if it is friendly
				projectile.AddComponent<ProjectileComponent>(projectileEmitter.isFriendly, projectileEmitter.hitPercentDamage);

					// Attach lifecycle to destroy projectile after some time
					projectile.AddComponent<LifecycleComponent>(projectileEmitter.projectileDurationInMs);

					// Update the projectile emitter component last emission to the current milliseconds
					projectileEmitter.lastEmissionTimeInMs = SDL_GetTicks();
				}
			}
		}
	}
};