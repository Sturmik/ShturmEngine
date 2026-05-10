#pragma once

#include <SDL3/SDL.h>

struct LifecycleComponent
{
	int durationInMs;
	int startTimeInMs;

	LifecycleComponent(int durationInMs = 0)
	: durationInMs(durationInMs), startTimeInMs(SDL_GetTicks()){}
};