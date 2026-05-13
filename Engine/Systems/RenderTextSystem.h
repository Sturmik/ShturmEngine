#pragma once

#include "ECS/ECS.h"
#include "AssetStore/AssetStore.h"
#include "Components/TransformComponent.h"
#include "Components/TextLabelComponent.h"

#include <SDL3/SDL.h>
#include <algorithm>

class RenderTextSystem : public System
{
public:
	RenderTextSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<TextLabelComponent>();
	}

	void Update(SDL_Renderer& renderer, AssetStore& assetStore, SDL_FRect& camera)
	{
		// Loop all entities that the system is interested in
		for (Entity& entity : AccessSystemEntities())
		{
			const TransformComponent& transform = entity.GetComponent<TransformComponent>();
			const TextLabelComponent& textLabel = entity.GetComponent<TextLabelComponent>();

			SDL_Surface* surface = TTF_RenderText_Blended(assetStore.GetFont(textLabel.assetId), 
			textLabel.text.c_str(), 
			textLabel.text.size(),
			textLabel.color);

			SDL_Texture* texture = SDL_CreateTextureFromSurface(&renderer, surface);
			SDL_DestroySurface(surface);

			float labelWidth = 0;
			float labelHeight = 0;
			SDL_GetTextureSize(texture, &labelWidth, &labelHeight);

			// Set the destination rectangle with the x, y position to be rendered
			SDL_FRect dstRect = {
				transform.position.x - (transform.isFixed ? 0 : camera.x),
				transform.position.y - (transform.isFixed ? 0 : camera.y),
				labelWidth,
				labelHeight
			};

			// Draw the PNG texture
			SDL_RenderTextureRotated(&renderer,
				texture,
				NULL,
				&dstRect,
				transform.rotation,
				NULL,
				SDL_FLIP_NONE);

			// Destroy texture
			SDL_DestroyTexture(texture);
		}
	}
};