#pragma once

#include "ECS/ECS.h"
#include "AssetStore/AssetStore.h"
#include "Components/TransformComponent.h"
#include "Components/HealthComponent.h"
#include "Components/HealthBarComponent.h"

#include <SDL3/SDL.h>
#include <algorithm>

class RenderHealthBarSystem : public System
{
public:
	RenderHealthBarSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<HealthComponent>();
		RequireComponent<HealthBarComponent>();
	}

	void Update(SDL_Renderer& renderer, AssetStore& assetStore, SDL_FRect& camera)
	{
		ForEach<TransformComponent, HealthComponent, HealthBarComponent>([&](Entity entity,
			const TransformComponent& transform,
			const HealthComponent& health,
			const HealthBarComponent& healthBar)
		{
			///////////////////////////// Define health state by color -> Green-Yellow-Red

			// Set drawing based on current health percentage
			SDL_Color healthBarColor = { 255, 0, 0 };
			float healthPercentage = (float)health.healthPercentage / (float)health.maxHealthPercentage;
			// Set green, if health is above 75%
			if (healthPercentage > 0.75)
			{
				healthBarColor = { 0, 255, 0 };
			}
			// Set yellow, if health is above 35%
			else if (healthPercentage > 0.35)
			{
				healthBarColor = { 255, 255, 0 };
			}

			///////////////////////////// Render health bar

			// Set render color
			SDL_SetRenderDrawColor(&renderer, healthBarColor.r, healthBarColor.g, healthBarColor.b, 255);

			// Set the destination rectangle with the x, y position to be rendered
			SDL_FRect healthBarRect = {
				transform.position.x + healthBar.healthBarOffset.x - (transform.isFixed ? 0 : camera.x),
				transform.position.y + healthBar.healthBarOffset.y - (transform.isFixed ? 0 : camera.y),
				healthBar.healthBarSize.x * healthPercentage,
				healthBar.healthBarSize.y
			};

			// Draw filled rectangle
			SDL_RenderFillRect(&renderer, &healthBarRect);

			///////////////////////////// Render health bar text values

			std::string healthPercentageText = std::to_string(health.healthPercentage);

			SDL_Surface* surface = TTF_RenderText_Blended(assetStore.GetFont(healthBar.fontAssetId),
				healthPercentageText.c_str(),
				healthPercentageText.size(),
				healthBarColor);

			SDL_Texture* texture = SDL_CreateTextureFromSurface(&renderer, surface);
			SDL_DestroySurface(surface);

			float labelWidth = 0;
			float labelHeight = 0;
			SDL_GetTextureSize(texture, &labelWidth, &labelHeight);

			// Set the destination rectangle with the x, y position to be rendered
			SDL_FRect textRect = {
				transform.position.x + healthBar.textOffset.x - (transform.isFixed ? 0 : camera.x),
				transform.position.y + healthBar.textOffset.y - (transform.isFixed ? 0 : camera.y),
				labelWidth,
				labelHeight
			};

			// Draw the PNG texture
			SDL_RenderTextureRotated(&renderer,
				texture,
				NULL,
				&textRect,
				transform.rotation,
				NULL,
				SDL_FLIP_NONE);

			// Destroy texture
			SDL_DestroyTexture(texture);
		});
	}
};