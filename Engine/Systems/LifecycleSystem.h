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
		ForEach<LifecycleComponent>([&](Entity entity, const LifecycleComponent& lifecycle)
		{
			// Kill object after it reaches it's duration limit
			if (SDL_GetTicks() - lifecycle.startTimeInMs > lifecycle.durationInMs)
			{
				entity.Kill();
			}
		});
	}
};