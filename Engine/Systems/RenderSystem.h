#pragma once

#include "ECS/ECS.h"
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

	void Update(SDL_Renderer& renderer)
	{
		// Loop all entities that the system is interested in
		for (Entity& entity : GetSystemEntities())
		{
			// Update entity position based on its velocity every frame of the game loop
			const TransformComponent& transform = entity.GetComponent<TransformComponent>();
			const SpriteComponent& sprite = entity.GetComponent<SpriteComponent>();

			SDL_FRect objRect = {
				transform.position.x,
				transform.position.y,
				sprite.width,
				sprite.height
			};

			SDL_SetRenderDrawColor(&renderer, 255, 255, 255, 255);
			SDL_RenderFillRect(&renderer, &objRect);
		}
	}
};