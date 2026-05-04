#pragma once 

#include "ECS/ECS.h"
#include "Components/TransformComponent.h"
#include "Components/BoxColliderComponent.h"

#include <SDL3/SDL.h>

class RenderColliderSystem: public System
{
public:
	RenderColliderSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<BoxColliderComponent>();
	}

	void Update(SDL_Renderer& renderer)
	{
		// Loop all entities that the system is interested in
		for (Entity& entity : AccessSystemEntities())
		{
			// Update entity position based on its velocity every frame of the game loop
			const TransformComponent& transform = entity.GetComponent<TransformComponent>();
			const BoxColliderComponent& boxCollider = entity.GetComponent<BoxColliderComponent>();

			// Set drawing color. Red in case of collision. Green in case of it's absence
			SDL_SetRenderDrawColor(&renderer, (boxCollider.isColliding ? 255 : 0), (boxCollider.isColliding ? 0 : 255), 0, 255);

			// Set the destination rectangle with the x, y position to be rendered
			SDL_FRect rect = {
				transform.position.x + boxCollider.offset.x,
				transform.position.y + boxCollider.offset.y,
				(boxCollider.offset.x + boxCollider.width) * transform.scale.x,
				(boxCollider.offset.y + boxCollider.height) * transform.scale.y
			};

			// Draw rectangle
			SDL_RenderRect(&renderer, &rect);
		}
	}
};