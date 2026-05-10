#pragma once

#include <SDL3/SDL.h>

#include "glm/glm.hpp"

struct ProjectileEmitterComponent
{
	glm::vec2 projectileVelocity;
	int repeatFrequency;
	int projectileDurationInMs;
	int hitPercentDamage;
	bool isFriendly;
	int lastEmissionTimeInMs;

	ProjectileEmitterComponent(glm::vec2 projectileVelocity = glm::vec2(0),
		int repeatFrequency = 0,
		int projectileDurationInMs = 10000,
		int hitPercentDamage = 10,
		bool isFriendly = false)
		: projectileVelocity(projectileVelocity),
		repeatFrequency(repeatFrequency),
		projectileDurationInMs(projectileDurationInMs),
		hitPercentDamage(hitPercentDamage),
		isFriendly(isFriendly),
		lastEmissionTimeInMs(SDL_GetTicks())
	{}
};