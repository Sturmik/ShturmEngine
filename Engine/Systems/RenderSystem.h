#pragma once

#include "ECS/ECS.h"
#include "AssetStore/AssetStore.h"
#include "Components/TransformComponent.h"
#include "Components/SpriteComponent.h"

#include <SDL3/SDL.h>
#include <algorithm>

class RenderSystem : public System
{
public:
	RenderSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<SpriteComponent>();
	}

	void Update(SDL_Renderer& renderer, AssetStore& assetStore)
	{
		// Sort entities by their z-index before rendering them
		std::sort(AccessSystemEntities().begin(), AccessSystemEntities().end(), [](const Entity& a, const Entity& b)
		{
			const SpriteComponent& spriteA = a.GetComponent<SpriteComponent>();
			const SpriteComponent& spriteB = b.GetComponent<SpriteComponent>();

			return spriteA.zIndex < spriteB.zIndex;
		});

		// Loop all entities that the system is interested in
		for (Entity& entity : AccessSystemEntities())
		{
			// Update entity position based on its velocity every frame of the game loop
			const TransformComponent& transform = entity.GetComponent<TransformComponent>();
			const SpriteComponent& sprite = entity.GetComponent<SpriteComponent>();

			// Set the source rectangle of our original sprite texture
			SDL_FRect srcRect = sprite.srcRect;

			// Set the destination rectangle with the x, y position to be rendered
			SDL_FRect dstRect = {
				transform.position.x,
				transform.position.y,
				sprite.width * transform.scale.x,
				sprite.height * transform.scale.y
			};

			// Draw the PNG texture
			SDL_RenderTextureRotated(&renderer, 
			assetStore.GetTexture(sprite.assetId), 
			&srcRect, 
			&dstRect, 
			transform.rotation,
			NULL, 
			SDL_FLIP_NONE);
		}
	}
};