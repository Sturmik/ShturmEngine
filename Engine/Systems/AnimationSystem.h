#pragma once

#include "ECS/ECS.h"

#include "Components/SpriteComponent.h"
#include "Components/AnimationComponent.h"

class AnimationSystem : public System
{
public:
	AnimationSystem()
	{
		RequireComponent<SpriteComponent>();
		RequireComponent<AnimationComponent>();
	}

	void Update()
	{
		for (std::shared_ptr<Archetype>& archetype : AccessArchetypes())
		{
			std::vector<Entity>& entities = archetype->entities;

			for (const Entity& entity : entities)
			{
				SpriteComponent& sprite = entity.GetComponent<SpriteComponent>();
				AnimationComponent& animation = entity.GetComponent<AnimationComponent>();

				animation.currentFrame = ((SDL_GetTicks() - animation.startTimeInMs) *
				animation.frameRateSpeedPerSecond / 1000) % animation.numFrames;

				sprite.srcRect.x = animation.currentFrame * sprite.width;
			}
		}
	}
};