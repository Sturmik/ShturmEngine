#pragma once

#include "ECS/ECS.h"
#include "Components/LifecycleComponent.h"

class LifecycleSystem : public System
{
public:
	LifecycleSystem()
	{
		RequireComponent<LifecycleComponent>();
	}

	void Update()
	{
		for (std::shared_ptr<Archetype>& archetype : AccessArchetypes())
		{
			std::vector<Entity>& entities = archetype->entities;

			// Loop all entities that the system is interested in
			for (Entity& entity : entities)
			{
				const LifecycleComponent& lifecycle = entity.GetComponent<LifecycleComponent>();

				// Kill object after it reaches it's duration limit
				if (SDL_GetTicks() - lifecycle.startTimeInMs > lifecycle.durationInMs)
				{
					entity.Kill();
				}
			}
		}
	}
};