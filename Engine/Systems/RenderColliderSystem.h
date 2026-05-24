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

	void Update(SDL_Renderer& renderer, SDL_FRect& camera)
	{
		ForEach<TransformComponent, BoxColliderComponent>([&](Entity entity, const TransformComponent& transform, const BoxColliderComponent& boxCollider)
		{
			// Set drawing color. Red in case of collision. Green in case of it's absence
			SDL_SetRenderDrawColor(&renderer, (boxCollider.isColliding ? 255 : 0), (boxCollider.isColliding ? 0 : 255), 0, 255);

			// Set the destination rectangle with the x, y position to be rendered
			SDL_FRect rect = {
				transform.position.x + boxCollider.offset.x - (transform.isFixed ? 0 : camera.x),
				transform.position.y + boxCollider.offset.y - (transform.isFixed ? 0 : camera.y),
				(boxCollider.offset.x + boxCollider.width) * transform.scale.x,
				(boxCollider.offset.y + boxCollider.height) * transform.scale.y
			};

			// Draw rectangle
			SDL_RenderRect(&renderer, &rect);
		});
	}
};