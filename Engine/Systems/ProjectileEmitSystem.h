#pragma once

#include "ECS/ECS.h"
#include "Components/TransformComponent.h"
#include "Components/RigidbodyComponent.h"
#include "Components/SpriteComponent.h"
#include "Components/ProjectileEmitterComponent.h"

class ProjectileEmitSystem : public System
{
public:
	ProjectileEmitSystem()
	{
		RequireComponent<ProjectileEmitterComponent>();
		RequireComponent<TransformComponent>();
	}

	void Update(AssetStore& assetStore, Registry& registry)
	{
		// Loop all entities that the system is interested in
		for (Entity& entity : AccessSystemEntities())
		{
			ProjectileEmitterComponent& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
			const TransformComponent& transform = entity.GetComponent<TransformComponent>();

			// Check if its time to re-emit a new projectileEmitter
			if (SDL_GetTicks() - projectileEmitter.lastEmissionTimeInMs > projectileEmitter.repeatFrequency)
			{
				glm::vec2 projectilePosition = transform.position;
				if (entity.HasComponent<SpriteComponent>())
				{
					const SpriteComponent& spriteComponent = entity.GetComponent<SpriteComponent>();

					projectilePosition.x += spriteComponent.width / 2;
					projectilePosition.y += spriteComponent.height / 2;
				}

				// Add a new projectileEmitter entity to the registry
				Entity projectile = registry.CreateEntity();
				projectile.AddComponent<TransformComponent>(projectilePosition, glm::vec2(1.0, 1.0), 0);
				projectile.AddComponent<RigidBodyComponent>(projectileEmitter.projectileVelocity);
				
				std::string textureKey = "bullet-image";
				projectile.AddComponent<SpriteComponent>(assetStore, textureKey, 4);
				
				float textureWidth = 0.0f;
				float textureHeight = 0.0f;
				SDL_GetTextureSize(assetStore.GetTexture(textureKey),
					&textureWidth,
					&textureHeight);
				projectile.AddComponent<BoxColliderComponent>(textureWidth, textureHeight);

				// Update the projectile emitter component last emission to the current milliseconds
				projectileEmitter.lastEmissionTimeInMs = SDL_GetTicks();
			}
		}
	}
};