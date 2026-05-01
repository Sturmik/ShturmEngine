#pragma once

#include "ECS/ECS.h"
#include "AssetStore/AssetStore.h"
#include "Components/TransformComponent.h"
#include "Components/SpriteComponent.h"

#include <SDL3/SDL.h>

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
		// Loop all entities that the system is interested in
		for (Entity& entity : GetSystemEntities())
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