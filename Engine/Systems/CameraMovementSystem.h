#pragma once

#include "ECS/ECS.h"
#include "AssetStore/AssetStore.h"
#include "Components/CameraFollowComponent.h"
#include "Components/TransformComponent.h"

#include <SDL3/SDL.h>
#include <algorithm>

class CameraMovementSystem : public System
{
public:
	CameraMovementSystem()
	{
		RequireComponent<CameraFollowComponent>();
		RequireComponent<TransformComponent>();
	}

	void Update(SDL_FRect& camera, glm::vec2 mapSize)
	{
		// Loop all entities that the system is interested in
		for (Entity& entity : AccessSystemEntities())
		{
			const TransformComponent& transform = entity.GetComponent<TransformComponent>();

			if (transform.position.x + (camera.w / 2) < mapSize.x)
			{
				camera.x = transform.position.x - (camera.w / 2);
			}

			if (transform.position.y + (camera.h / 2) < mapSize.y)
			{
				camera.y = transform.position.y - (camera.h / 2);
			}

			// Keep camera rectangle view inside the screen limits
			camera.x = camera.x < 0 ? 0 : camera.x;
			camera.y = camera.y < 0 ? 0 : camera.y;
			float cameraBoundariesW = mapSize.x - camera.w / 2;
			float cameraBoundariesH = mapSize.y - camera.h / 2;
			camera.x = camera.x > cameraBoundariesW ? cameraBoundariesW : camera.x;
			camera.y = camera.y > cameraBoundariesH ? cameraBoundariesH : camera.y;
		}
	}
};